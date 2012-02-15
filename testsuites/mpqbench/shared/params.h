/* This is a generated file. DO NOT EDIT. */

/* command line: -s 1000 -p 10 -i 10000 -d 4 -o 1000  */
/*
-T 4,0.003,0 -T 10,0.094,0 -T 13,0.235,0 -T 29,0.175,0 -T 50,0.092,0 -A 200,1,0 -A 200,1,0 -A 200,1,0 -A 200,1,0 -A 200,1,0
*/
/* 4916 duplicate enqueues during queue creation */

/* 996 hold operations are duplicate enqueues */

/* The maximum priority value during queue creation is 390452 */

/* The maximum priority value during hold operations is 399983 */


#ifndef __PARAMS_H_
#define __PARAMS_H_

/* task parameters */
#define NUM_PERIODIC_TASKS  (0)
#define NUM_APERIODIC_TASKS (5)
#define NUM_TASKS           ( NUM_PERIODIC_TASKS + NUM_APERIODIC_TASKS )

#define build_task_name() do { \
Task_name[ 1 ] =  rtems_build_name( 'A', 'T', '0', '1' );\
Task_name[ 2 ] =  rtems_build_name( 'A', 'T', '0', '2' );\
Task_name[ 3 ] =  rtems_build_name( 'A', 'T', '0', '3' );\
Task_name[ 4 ] =  rtems_build_name( 'A', 'T', '0', '4' );\
Task_name[ 5 ] =  rtems_build_name( 'A', 'T', '0', '5' );\
} while(0)

/* PQ parameters */
#define PQ_MAX_SIZE   (1000)
#define PQ_WARMUP_OPS (8984)
#define PQ_WORK_OPS   (1000)

#endif
