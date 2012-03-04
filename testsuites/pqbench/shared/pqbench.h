#ifndef _PQBENCH_H_
#define _PQBENCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#define GAB_PRINT
//#define GAB_DEBUG
//#define GAB_CHECK

//#define USE_NEW_FREELIST

#if defined(GAB_PRINT) || defined(GAB_DEBUG) || defined(GAB_CHECK)
#include <stdio.h>
#endif

#include "rtems/rtems/types.h"
#include "params.h"

#define kv_value(kv) ((uint32_t)kv)
#define kv_key(kv)   (kv>>32)

typedef enum {
  f,      /* first */
  i,      /* insert */
  p,      /* pop */
  h,      /* hold */
} PQ_op;

typedef struct {
  int key;
  int val;
} PQ_arg;

/* pqbench interface */
extern void pq_initialize( int size );
extern void pq_insert( uint64_t p );
extern uint64_t pq_first( void );
extern uint64_t pq_pop( void );

#ifdef __cplusplus
}
#endif

#endif
