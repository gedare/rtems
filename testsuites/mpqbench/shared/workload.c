/*
 * PQ workload
 */

#include "pqbench.h"
#include "workload.h"
#include "params.h"
#include "params.i"

#include <stdlib.h>

#define ARG_TO_UINT64(n) ((((uint64_t)n->key) << 32UL) | (uint64_t)n->val)

inline uint64_t pq_add_to_key(uint64_t p, uint32_t i) {
  return p+((uint64_t)i<<32UL);
}

inline uint64_t pq_node_initialize( PQ_arg *a ) {
  return ARG_TO_UINT64(a);
}

#if defined(GAB_PRINT) || defined(GAB_DEBUG)
inline void pq_print_node( uint64_t p ) {
  printf("%d\t%d\n", kv_key(p), kv_value(p));
}
#endif

static int execute( rtems_task_argument tid, int current_op ) {
  uint64_t n;

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
      n = pq_pop(tid); // skip first otherwise take ctxt switch..

      // measure dequeue
      /*
      asm volatile("break_start_opal:"); // comment to warmup
      MAGIC(1);
      n = pq_pop(tid);
      MAGIC_BREAKPOINT;
      */

      // measure enqueue
      /*
      asm volatile("break_start_opal:"); // comment to warmup
      MAGIC(1);
      pq_insert(tid, n)
      MAGIC_BREAKPOINT;
      */
 
      // measure spill exception
      /*
      //FIXME: not working since context switch change.
      {
        int i;
        int s;
        HWDS_GET_SIZE_LIMIT(tid, s);
        for ( i = 0; i < s - (s/2); i++ ) {
          n = pq_add_to_key(n, args[current_op++].key);
          pq_insert(tid,n);
        }
      }
      //asm volatile("break_start_opal:"); // comment to warmup
      n = pq_add_to_key(n, args[current_op++].key);
      pq_insert(tid,n);
      MAGIC_BREAKPOINT;
      */

      // measure fill exception
      /*
      //FIXME: not working since context switch change.
      {
        int i;
        int s;
        HWDS_GET_SIZE_LIMIT(tid, s);
        for ( i = 0; i < s/2-1; i++ ) {
          n = pq_pop(tid);
        }
      }
//      asm volatile("break_start_opal:"); // comment to warmup
      n = pq_pop(tid);
      MAGIC_BREAKPOINT;
      */

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
    default:
#if defined(GAB_PRINT)
      printf("%d\tInvalid Op: %d\n",tid,ops[current_op]);
#endif
      break;
  }
  current_op++;
}

void initialize(rtems_task_argument tid ) {
  /* initialize structures */
  pq_initialize(tid, PQ_MAX_SIZE);
}

static void drain_and_check(rtems_task_argument tid) {
  uint64_t s = 0;

  uint64_t n;
  while ((n = pq_pop(tid)) != (uint64_t)-1) { // FIXME: casting -1 :(
    s = s + kv_key(n);
  }
  printf("%d\tChecksum: 0x%llX\n", tid, s);

}

void warmup( rtems_task_argument tid ) {
  int i = 0;

#if defined GAB_PRINT
  printf("%d\tWarmup: %d\n",tid, PQ_WARMUP_OPS);
#endif
  for ( i = 0; i < PQ_WARMUP_OPS; i++ ) {
    execute(tid, i);
  }
  // TODO: fill up the hwpq to full capacity for testing the context switch
}

void work( rtems_task_argument tid  ) {
  int i = 0;
#if defined(GAB_PRINT)
  printf("%d\tWork: %d\n",tid, PQ_WORK_OPS);
#endif
  MAGIC(1);
  for ( i = 0; i < PQ_WORK_OPS; i++ ) {
    execute(tid, PQ_WARMUP_OPS + i);
    MAGIC(1);
  }
#if defined(GAB_CHECK)
  drain_and_check(tid);
#endif
}

