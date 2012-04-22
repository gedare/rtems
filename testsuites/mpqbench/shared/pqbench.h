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
#include "params.h"

#define kv_value(kv) ((uint32_t)kv)
#define kv_key(kv)   (kv>>32)

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
extern void pq_insert( rtems_task_argument tid, uint64_t p );
extern uint64_t pq_first( rtems_task_argument tid );
extern uint64_t pq_pop( rtems_task_argument tid );
extern uint64_t pq_search( rtems_task_argument tid, int key );
extern uint64_t pq_extract( rtems_task_argument tid, int key );

#ifdef __cplusplus
}
#endif

#endif
