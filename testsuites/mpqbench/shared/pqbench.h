#ifndef _PQBENCH_H_
#define _PQBENCH_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define GAB_PRINT
#define GAB_DEBUG
#define GAB_CHECK

#if defined(GAB_PRINT) || defined(GAB_DEBUG) || defined(GAB_CHECK)
#include <stdio.h>
#endif
#include <rtems.h>
#include "rtems/rtems/types.h"
#include "workload.h"
#include "params.h"

#define kv_value(kv) (kv & ~((~0UL)<<(sizeof(long)*4L)))
#define kv_key(kv)   (kv>>(sizeof(long)*4L))

typedef enum {
  f,      /* first */
  i,      /* insert */
  p,      /* pop */
  h,      /* hold */
  s,      /* search */
  x,      /* extract */
} PQ_op;

typedef struct {
  int key;
  int val;
} PQ_arg;

/* pqbench interface */
extern void pq_initialize( rtems_task_argument tid, int size );
extern void pq_insert( rtems_task_argument tid, long p );
extern long pq_first( rtems_task_argument tid );
extern long pq_pop( rtems_task_argument tid );
extern long pq_search( rtems_task_argument tid, int key );
extern long pq_extract( rtems_task_argument tid, int key );

#ifdef __cplusplus
}
#endif

#endif
