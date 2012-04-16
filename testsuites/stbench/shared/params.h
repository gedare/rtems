/* This is a generated file. DO NOT EDIT. */

/* command line: -s 65536 -p 10 -i 1000 -d 2 -o 1024 -m 3  */
/* i: 1000
   alpha: 0.5
   beta: 0.5
 */

/* 37 duplicate enqueues during warmup */

/* 0 duplicate enqueues during work */

/* The maximum priority value during warmup is 65554012 */

/* The maximum priority value during work is 0 */


#ifndef __PARAMS_H_
#define __PARAMS_H_

/* task parameters */
#define NUM_PERIODIC_TASKS  (0)
#define NUM_APERIODIC_TASKS (2)
#define NUM_TASKS           ( NUM_PERIODIC_TASKS + NUM_APERIODIC_TASKS )

#define build_task_name() do { \
Task_name[ 1 ] =  rtems_build_name( 'A', 'T', '0', '1' );\
Task_name[ 2 ] =  rtems_build_name( 'A', 'T', '0', '2' );\
} while(0)

/* ST parameters */
#define ST_MAX_SIZE   (65536)
#define ST_WARMUP_OPS (65536)
#define ST_WORK_OPS   (65536)

#endif