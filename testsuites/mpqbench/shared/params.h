/* This is a generated file. DO NOT EDIT. */

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

/* PQ parameters */
#define PQ_MAX_SIZE   (1000)
#define PQ_WARMUP_OPS (5120)
#define PQ_WORK_OPS   (1000)

#endif
