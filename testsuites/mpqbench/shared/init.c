/*  Init
 *
 *  Input parameters:  NONE
 *
 *  Output parameters:  NONE
 */

#define GAB_TIMESLICE
//#define CACHE_TASK

#include "params.h"
#define CONFIGURE_MAXIMUM_SEMAPHORES        2
#if defined(CACHE_TASK)
  #define CONFIGURE_MAXIMUM_TASKS               (2+NUM_TASKS)
#else
  #define CONFIGURE_MAXIMUM_TASKS               (1+NUM_TASKS)
#endif
#define CONFIGURE_MAXIMUM_PERIODS             (1+NUM_PERIODIC_TASKS)
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#if defined(GAB_TIMESLICE)
#define CONFIGURE_TICKS_PER_TIMESLICE       1
#define CONFIGURE_MICROSECONDS_PER_TICK   RTEMS_MILLISECONDS_TO_MICROSECONDS(5)
#endif
#define CONFIGURE_INIT
#include "system.h"

#include <rtems.h>

#include <tmacros.h>
#include <stdio.h>
#include <stdlib.h>

#include "workload.h"
#include "tasks.i"

#include <bsp.h> /* bad */
#include <boot/ofw.h> /* bad */

/* an approximation */
#define busy_wait( microseconds, inst_per_us ) \
  do { \
    register uint32_t         _delay=(microseconds); \
    register uint32_t         _inst=(inst_per_us); \
    register int              _tmp=0; \
    asm volatile( "sub     %0, 6, %0; \
        brz     %2, 2f; \
      0: \
        mov     %0, %1; \
      1: \
        nop; \
        sub     %1, 6, %1; \
        nop; \
        nop; \
        brgz  %1, 1b; \
        nop; \
        dec     %2; \
        nop; \
        nop; \
        brnz    %2, 0b; \
      2: nop " \
      : "=r" (_inst), "=r" (_tmp), "=r" (_delay) \
      : "0" (_inst), "1" (_tmp), "2" (_delay) ); \
  } while (0) 


static uint32_t  tasks_completed;
static rtems_id  tasks_complete_sem;
static rtems_id  final_barrier;
static uint32_t  Instructions_per_us;

rtems_id   Task_id[ 1+NUM_TASKS ];
rtems_name Task_name[ 1+NUM_TASKS ];

static unsigned int get_Frequency(void)
{
	phandle root = ofw_find_device("/");
	unsigned int freq;
	if (ofw_get_property(root, "clock-frequency", &freq, sizeof(freq)) <= 0) {
		return 150000000;
	}

	return freq;
}

rtems_task PQ_Cache_Task(rtems_task_argument argument)
{
  int *p, x;
  rtems_status_code status;
  printf("CT01\n");

  x = 0;
  p = 0;
  /* Barrier: tasks will be released by the init function */
  status = rtems_semaphore_obtain(tasks_complete_sem,RTEMS_DEFAULT_OPTIONS,0);

  while (FOREVER) {
    x += *p;
    p += 4;
    if ( x == -1 ) {
      printf("Killing cache task!\n");
      status = rtems_task_delete(RTEMS_SELF); /* probably unreached... */
      directive_failed(status, "rtems_task_delete of RTEMS_SELF");
      return (rtems_task)x; /* unreached */
    }
  }
}

rtems_task PQ_Periodic_Task(rtems_task_argument argument)
{
  rtems_id          rmid;
  rtems_status_code status;

  put_name( Task_name[ argument ], 1 );

  status = rtems_rate_monotonic_create( argument, &rmid );
  directive_failed( status, "rtems_rate_monotonic_create" );

  /* Barrier: tasks will be released by the init function */
  status = rtems_semaphore_obtain(tasks_complete_sem,RTEMS_DEFAULT_OPTIONS,0);

  /* Periodic Loop */
  while (FOREVER) {
    if (rtems_rate_monotonic_period(rmid, Periods[argument])==RTEMS_TIMEOUT) {
      puts("Error: deadline missed!\n");
    }
    busy_wait( Execution_us[argument], Instructions_per_us );
  }
  /* Shouldn't reach here. */

  /* And should block forever if it does. */
  status = rtems_semaphore_obtain( final_barrier, RTEMS_DEFAULT_OPTIONS, 0 );
  directive_failed( status, "rtems_semaphore_obtain" );

  printf( "Killing task %d\n", argument);
  status = rtems_task_delete(RTEMS_SELF);
  directive_failed(status, "rtems_task_delete of RTEMS_SELF");
}

rtems_task PQ_Workload_Task(rtems_task_argument argument)
{
  int               index;
  rtems_status_code status;

  put_name( Task_name[ argument ], 1 );

  /* Barrier: tasks will be released by the init function */
  status = rtems_semaphore_obtain(tasks_complete_sem, RTEMS_DEFAULT_OPTIONS, 0);
  
  /* initialize PQ structures */
  initialize(argument - 1);
  
  /* reach PQ steady state */
  warmup(argument - 1);

#ifdef DOMEASURE
  rtems_task_wake_after( 5 );
#endif
  // FIXME: how to reset stats for warmup phase?

  /* workload */
  work(argument - 1);

  status = rtems_semaphore_obtain(tasks_complete_sem, RTEMS_DEFAULT_OPTIONS, 0);
  directive_failed( status, "rtems_semaphore_obtain" );
    tasks_completed++;
    if (NUM_APERIODIC_TASKS == tasks_completed) {
      MAGIC_BREAKPOINT;
      puts( "*** END OF TEST ***" );
      rtems_test_exit(0);
    }
  status = rtems_semaphore_release( tasks_complete_sem );
  directive_failed( status, "rtems_semaphore_release" );

  /* should block forever */
  status = rtems_semaphore_obtain( final_barrier, RTEMS_DEFAULT_OPTIONS, 0 );
  directive_failed( status, "rtems_semaphore_obtain" );

  /* shouldn't reach this */
  printf( "Killing task %d", argument);
  status = rtems_task_delete(RTEMS_SELF);
  directive_failed(status, "rtems_task_delete of RTEMS_SELF");
}

rtems_task Init(
  rtems_task_argument ignored
)
{
  uint32_t          index;
  uint32_t          freq; 
  rtems_status_code status;

#if defined(CACHE_TASK)
  rtems_id   cacheTask_id;
  rtems_name cacheTask_name;

  cacheTask_name = rtems_build_name( 'C', 'T', '0', '1' );
#endif


  freq = get_Frequency( );
  Instructions_per_us = freq / 1000000;

  if ( Instructions_per_us == 0 ) {
    puts( "error: CPU frequency is too low" );
    rtems_test_exit(0);
  }

  if ( NUM_APERIODIC_TASKS == 0 ) {
    puts("error: Need at least 1 aperiodic task to ensure termination");
    rtems_test_exit(0);
  }

  tasks_completed = 0;

  build_task_name();

  status = rtems_semaphore_create(
      rtems_build_name( 'S', 'E', 'M', '0' ),  /* name = SEM0 */
      0,                                       /* locked */
      RTEMS_BINARY_SEMAPHORE | RTEMS_FIFO,     /* mutex w/desired discipline */
      0,                                       /* IGNORED */
      &tasks_complete_sem
    );
  directive_failed( status, "rtems_semaphore_create" );

  status = rtems_semaphore_create(
      rtems_build_name( 'S', 'E', 'M', '1' ),  /* name = SEM1 */
      0,                                       /* locked */
      RTEMS_SIMPLE_BINARY_SEMAPHORE,           /* mutex w/desired discipline */
      0,                                       /* IGNORED */
      &final_barrier
    );
  directive_failed( status, "rtems_semaphore_create" );

  for ( index = 1 ; index <= NUM_TASKS ; index++ ) {
    status = rtems_task_create(
      Task_name[ index ],
      Priorities[ index ],
      RTEMS_MINIMUM_STACK_SIZE,
#if defined(GAB_TIMESLICE)
      RTEMS_PREEMPT|RTEMS_TIMESLICE,
#else
      RTEMS_DEFAULT_MODES,
#endif
      RTEMS_DEFAULT_ATTRIBUTES,
      &Task_id[ index ]
    );
    directive_failed( status, "rtems_task_create loop" );
  }

  for ( index = 1 ; index <= NUM_APERIODIC_TASKS ; index++ ) {
    status = rtems_task_start( Task_id[ index ], PQ_Workload_Task, index );
    directive_failed( status, "rtems_task_start loop" );
  }

  for ( index = NUM_APERIODIC_TASKS+1 ; index <= NUM_TASKS ; index++ ) {
    status = rtems_task_start( Task_id[ index ], PQ_Periodic_Task, index );
    directive_failed( status, "rtems_task_start loop" );
  }

#if defined(CACHE_TASK)

  status = rtems_task_create(
      cacheTask_name,
      200, /* FIXME */
      RTEMS_MINIMUM_STACK_SIZE,
  #if defined(GAB_TIMESLICE)
      RTEMS_PREEMPT|RTEMS_TIMESLICE,
  #else
      RTEMS_DEFAULT_MODES,
  #endif
      RTEMS_DEFAULT_ATTRIBUTES,
      &cacheTask_id
  );
  status = rtems_task_start( cacheTask_id, PQ_Cache_Task, NUM_TASKS+1 );
#endif

 
  /* start measurement */
#ifdef WARMUP
  asm volatile("break_start_opal:");
#endif

  rtems_task_wake_after( 1 );
  /* release all of the waiting tasks */
  status = rtems_semaphore_flush( tasks_complete_sem );
  directive_failed( status, "rtems_semaphore_flush" );

  status = rtems_semaphore_release( tasks_complete_sem );
  directive_failed( status, "rtems_semaphore_release" );

  /* Should block forever */
  status = rtems_semaphore_obtain( final_barrier, RTEMS_DEFAULT_OPTIONS, 0 );
  directive_failed( status, "rtems_semaphore_obtain" );

  /* unreached */
  puts("Init killing self\n");

  status = rtems_task_delete( RTEMS_SELF );
  directive_failed( status, "rtems_task_delete of RTEMS_SELF" );
}

