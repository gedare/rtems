/*
 * ST workload
 */

#include "pqbench.h"
#include "workload.h"
#include "params.h"
#include "params.i"

#include <libcpu/spillpq.h> // bad

#include <stdlib.h>

#define ARG_TO_UINT64(n) ((((uint64_t)n->key) << 32UL) | (uint64_t)n->val)

inline uint64_t st_add_to_key(uint64_t p, uint32_t i) {
  return p+((uint64_t)i<<32UL);
}

inline uint64_t st_node_initialize( ST_arg *a ) {
  return ARG_TO_UINT64(a);
}

#if defined(GAB_PRINT) || defined(GAB_DEBUG)
inline void st_print_node( uint64_t p ) {
  printf("%d\t%d\n", kv_key(p), kv_value(p));
}
#endif

static int execute( rtems_task_argument tid, int current_op ) {
  uint64_t n;

  switch (ops[current_op]) {
    case f:
      n = st_first(tid);
#if defined(GAB_PRINT)
      printf("%d\tST first:\t",tid);
      st_print_node(p);
#endif
      break;
    case i:
      n = st_node_initialize( &args[current_op] );
      st_insert( tid, n );
#if defined(GAB_PRINT)
      printf("%d\tST insert (args=%d,%d):\t",
          tid, args[current_op].key, args[current_op].val);
      st_print_node(n);
#endif
      break;
    case p:
      n = st_pop(tid);
#if defined(GAB_DEBUG)
      if ( kv_key(n) != args[current_op].key ) {
        printf("%d\tInvalid node popped (args=%d,%d):\t",
            tid, args[current_op].key, args[current_op].val);
        st_print_node(n);
      }
#endif
#if defined(GAB_PRINT) && defined(GAB_DEBUG)
      if ( kv_value(n) != args[current_op].val ) {
        printf("%d\tUnexpected node (non-FIFO/stable) popped (args=%d,%d):\t",
            tid,args[current_op].key, args[current_op].val);
        st_print_node(n);
      }
#endif
#if defined(GAB_PRINT)
      printf("%d\tST pop (args=%d,%d):\t",tid, args[current_op].key, args[current_op].val);
      st_print_node(n);
#endif
      break;
    case h:
      #ifdef MEASURE_DEQUEUE
        MAGIC(1);
        n = st_pop(tid);
        MAGIC(2);
      #else
        n = st_pop(tid);
      #endif
#if defined(GAB_DEBUG)
      if ( kv_key(n) != args[current_op].val ) {
        printf("%d\tInvalid node popped (args=%d,%d):\t",tid, args[current_op].key, args[current_op].val);
        st_print_node(n);
      }
#endif
      n = st_add_to_key(n, args[current_op].key);/* add to prio */
#if defined(GAB_PRINT)
      printf("%d\tST hold (args=%d,%d):\t",tid, args[current_op].key, args[current_op].val);
      st_print_node(n);
#endif
      #ifdef MEASURE_ENQUEUE
        MAGIC(1);
        st_insert(tid,n);
        MAGIC(2);
      #else
        st_insert(tid,n);
      #endif
      break;
    default:
#if defined(GAB_PRINT)
      printf("%d\tInvalid Op: %d\n",tid,ops[current_op]);
#endif
      break;
  }
  current_op++;
}

static int measure( rtems_task_argument tid, int current_op )
{
  uint64_t n;
  ST_arg a;

  a.key = 1;
  a.val = 1;

  // measure context switch
#ifdef MEASURE_CS
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  n = st_pop(tid);
  MAGIC_BREAKPOINT;
#endif

  n = st_pop(tid); // take ctxt switch

  // measure dequeue
#ifdef MEASURE_DEQUEUE
  n = st_add_to_key(n, args[current_op++].key);
  st_insert(tid, n);
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  n = st_pop(tid);
  MAGIC_BREAKPOINT;
#endif

  // measure enqueue
#ifdef MEASURE_ENQUEUE
  n = st_add_to_key(n, args[current_op++].key);
  st_insert(tid, n);
  n = st_node_initialize( &a );
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  st_insert(tid, n);
  MAGIC_BREAKPOINT;
#endif

  // measure spill exception
#ifdef MEASURE_SPILL 
  n = st_add_to_key(n, args[current_op++].key);
  st_insert(tid, n);
  n = st_add_to_key(n, args[current_op++].key);
  st_insert(tid, n);
  n = st_node_initialize( &a );
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  st_insert(tid, n);
  MAGIC_BREAKPOINT;
#endif

  // measure fill exception
#ifdef MEASURE_FILL
  {
#ifndef WARMUP
    asm volatile("break_start_opal:");
#endif
    MAGIC(1);
    HWDS_INVALIDATE(tid);
    n = st_pop(tid);
    MAGIC_BREAKPOINT;
  }
#endif

  return 0;
}

void initialize(rtems_task_argument tid ) {
  /* initialize structures */
  st_initialize(tid, ST_MAX_SIZE);
}

static void drain_and_check(rtems_task_argument tid) {
  uint64_t s = 0;

  uint64_t n;
  while ((n = st_pop(tid)) != (uint64_t)-1) { // FIXME: casting -1 :(
    s = s + kv_key(n);
  }
  printf("%d\tChecksum: 0x%llX\n", tid, s);

}

void warmup( rtems_task_argument tid ) {
  int i = 0;

#if defined GAB_PRINT
  printf("%d\tWarmup: %d\n",tid, ST_WARMUP_OPS);
#endif
  for ( i = 0; i < ST_WARMUP_OPS; i++ ) {
    execute(tid, i);
  }

#ifdef DOMEASURE

  // insert some minimum priority elements (to prime the hwpq) and
  // fill up the hwpq to full capacity for measuring exceptions
  if (spillpq_ops[tid]) {
    uint64_t n;
    ST_arg a;
    int s, c;
    HWDS_GET_SIZE_LIMIT(tid, s);
    if ( s > ST_MAX_SIZE )
      s = ST_MAX_SIZE;
    for ( i = 0; i < s; i++ ) {
      n = st_pop(tid); // keep st size the same
    }
    for ( i = 0; i < s; i++ ) {
      a.key = i+2;
      a.val = i+2;
      n = st_node_initialize( &a );
      st_insert(tid,n);
    }
    HWDS_GET_CURRENT_SIZE(tid, c);
    if ( s > c )
      spillpq_ops[tid]->fill(tid, s-c-1);
    HWDS_GET_CURRENT_SIZE(tid, c);
    printk("s: %d\tc: %d\n",s,c);
  }
#endif
}

void work( rtems_task_argument tid  ) {
  int i = 0;
#if defined(GAB_PRINT)
  printf("%d\tWork: %d\n",tid, ST_WORK_OPS);
#endif

#ifdef DOMEASURE
#ifdef WARMUP
  if (spillpq_ops[tid]) {
    measure(tid, ST_WARMUP_OPS);
  }
#else
  measure(tid, ST_WARMUP_OPS);
#endif
#endif

  MAGIC(1);
  for ( i = 0; i < ST_WORK_OPS; i++ ) {
    execute(tid, ST_WARMUP_OPS + i);
    MAGIC(1);
  }
#if defined(GAB_CHECK)
  drain_and_check(tid);
#endif
}

