/*
 * PQ workload
 */

#include "pqbench.h"
#include "workload.h"
#include "params.h"
#include "params.i"

#include <libcpu/spillpq.h> // bad

#include <stdlib.h>

#define ARG_TO_LONG(n) ((((long)n->key) << (sizeof(long)*4L)) | (long)n->val)

inline long pq_add_to_key(long p, uint32_t i) {
  return p+((long)i<<(sizeof(long)*4L));
}

inline long pq_node_initialize( PQ_arg *a ) {
  return ARG_TO_LONG(a);
}

#if defined(GAB_PRINT) || defined(GAB_DEBUG)
inline void pq_print_node( long p ) {
  printf("%d\t%d\n", kv_key(p), kv_value(p));
}
#endif

static int execute( rtems_task_argument tid, int current_op );
static void drain_and_check(rtems_task_argument tid);
static int measure( rtems_task_argument tid, int current_op );

void initialize(rtems_task_argument tid ) {
  /* initialize structures */
  pq_initialize(tid, PQ_MAX_SIZE);
}

void warmup( rtems_task_argument tid ) {
  int i = 0;

#if defined GAB_PRINT
  printf("%d\tWarmup: %d\n",tid, PQ_WARMUP_OPS);
#endif
  for ( i = 0; i < PQ_WARMUP_OPS; i++ ) {
    execute(tid, i);
  }

#ifdef DOMEASURE

  // insert some minimum priority elements (to prime the hwpq) and
  // fill up the hwpq to full capacity for measuring exceptions
  if (spillpq[tid].ops) {
    long n;
    PQ_arg a;
    int s, c;
    HWDS_GET_SIZE_LIMIT(tid, s);
    if ( s > PQ_MAX_SIZE )
      s = PQ_MAX_SIZE;
    for ( i = 0; i < s; i++ ) {
      n = pq_pop(tid); // keep pq size the same
    }
    for ( i = 0; i < s; i++ ) {
      a.key = i+2;
      a.val = i+2;
      n = pq_node_initialize( &a );
      pq_insert(tid,n);
    }
    HWDS_GET_CURRENT_SIZE(tid, c);
    if ( s > c )
      spillpq[tid].ops->fill(tid, s-c-1);
    HWDS_GET_CURRENT_SIZE(tid, c);
    printk("s: %d\tc: %d\n",s,c);
  }
#endif
}

void work( rtems_task_argument tid  ) {
  int i = 0;
#if defined(GAB_PRINT)
  printf("%d\tWork: %d\n",tid, PQ_WORK_OPS);
#endif

#ifdef DOMEASURE
#ifdef WARMUP
  if (spillpq[tid].ops) {
    measure(tid, PQ_WARMUP_OPS);
  }
#else
  measure(tid, PQ_WARMUP_OPS);
#endif
#endif
#ifdef WARMUP
  MAGIC(1);
#endif
  for ( i = 0; i < PQ_WORK_OPS; i++ ) {
    execute(tid, PQ_WARMUP_OPS + i);
#ifdef RESET_EACH_WORK_OP
    MAGIC(1);
#endif
  }
#if defined(GAB_CHECK)
  drain_and_check(tid);
#endif
}


static int execute( rtems_task_argument tid, int current_op ) {
  long n;

  switch (ops[current_op]) {
    case f:
      n = pq_first(tid);
#if defined(GAB_PRINT)
      printf("%d\tPQ first:\t",tid);
      pq_print_node(p);
#endif
      break;
    case i:
      n = pq_node_initialize( &args[current_op] );
      pq_insert( tid, n );
#if defined(GAB_PRINT)
      printf("%d\tPQ insert (args=%d,%d):\t",
          tid, args[current_op].key, args[current_op].val);
      pq_print_node(n);
#endif
      break;
    case p:
      n = pq_pop(tid);
#if defined(GAB_DEBUG)
      if ( kv_key(n) != args[current_op].key ) {
        printf("%d\tInvalid node popped (args=%d,%d):\t",
            tid, args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
#endif
#if defined(GAB_PRINT) && defined(GAB_DEBUG)
      if ( kv_value(n) != args[current_op].val ) {
        printf("%d\tUnexpected node (non-FIFO/stable) popped (args=%d,%d):\t",
            tid,args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
#endif
#if defined(GAB_PRINT)
      printf("%d\tPQ pop (args=%d,%d):\t",tid, args[current_op].key, args[current_op].val);
      pq_print_node(n);
#endif
      break;
    case h:
        n = pq_pop(tid);
#if defined(GAB_DEBUG)
      if ( kv_key(n) != args[current_op].val ) {
        printf("%d\tInvalid node popped (args=%d,%d):\t",tid, args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
#endif
      n = pq_add_to_key(n, args[current_op].key);/* add to prio */
#if defined(GAB_PRINT)
      printf("%d\tPQ hold (args=%d,%d):\t",tid, args[current_op].key, args[current_op].val);
      pq_print_node(n);
#endif
      pq_insert(tid,n);
      break;
    case s:
      n = pq_search(tid, args[current_op].key); // UNIQUE!
#if defined(GAB_DEBUG)
      if (kv_value(n) != args[current_op].val) {
        printf("%d\tInvalid node found (args=%d,%d):\t",
            tid, args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
#endif
#if defined(GAB_PRINT)
      printf("%d\tPQ search (args=%d,%d):\t",
          tid, args[current_op].key, args[current_op].val);
      pq_print_node(n);
#endif
      break;
    case x:
      n = pq_extract(tid, args[current_op].key);
#if defined(GAB_DEBUG)
      if (kv_value(n) != args[current_op].val) {
        printf("%d\tInvalid node found (args=%d,%d):\t",
            tid, args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
#endif
#if defined(GAB_PRINT)
      printf("%d\tPQ extract (args=%d,%d):\t",
          tid, args[current_op].key, args[current_op].val);
      pq_print_node(n);
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




static void drain_and_check(rtems_task_argument tid) {
  long s = 0;

  long n;
  while ((n = pq_pop(tid)) != (long)-1) { // FIXME: casting -1 :(
    s = s + kv_key(n);
  }
  printf("%d\tChecksum: 0x%llX\n", tid, s);

}

static int measure( rtems_task_argument tid, int current_op )
{
  long n;
  PQ_arg a;

  a.key = 1;
  a.val = 1;

  // measure context switch
#ifdef MEASURE_CS
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  n = pq_pop(tid);
  MAGIC_BREAKPOINT;
#endif

  n = pq_pop(tid); // take ctxt switch

  // measure dequeue
#ifdef MEASURE_DEQUEUE
  n = pq_add_to_key(n, args[current_op++].key);
  pq_insert(tid, n);
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  n = pq_pop(tid);
  MAGIC_BREAKPOINT;
#endif

  // measure enqueue
#ifdef MEASURE_ENQUEUE
  n = pq_add_to_key(n, args[current_op++].key);
  pq_insert(tid, n);
  n = pq_node_initialize( &a );
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  pq_insert(tid, n);
  MAGIC_BREAKPOINT;
#endif

  // measure spill exception
#ifdef MEASURE_SPILL 
  n = pq_add_to_key(n, args[current_op++].key);
  pq_insert(tid, n);
  n = pq_add_to_key(n, args[current_op++].key);
  pq_insert(tid, n);
  n = pq_node_initialize( &a );
#ifndef WARMUP
  asm volatile("break_start_opal:");
#endif
  MAGIC(1);
  pq_insert(tid, n);
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
    n = pq_pop(tid);
    MAGIC_BREAKPOINT;
  }
#endif

  return 0;
}


