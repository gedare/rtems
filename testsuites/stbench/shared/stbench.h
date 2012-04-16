#ifndef _STBENCH_H_
#define _STBENCH_H_

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
} ST_op;

typedef struct {
  int key;
  int val;
} ST_arg;

/* stbench interface */
extern void st_initialize( rtems_task_argument tid, int size );
extern void st_insert( rtems_task_argument tid, uint64_t p );
extern uint64_t st_first( rtems_task_argument tid );
extern uint64_t st_pop( rtems_task_argument tid );

#ifdef __cplusplus
}
#endif

#endif
