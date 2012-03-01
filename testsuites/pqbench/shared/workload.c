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

static void execute( void ) {
  uint64_t n;
  static int current_op = 0;
  switch (ops[current_op]) {
    case f:
      n = pq_first( );
#if defined(GAB_PRINT)
      printf("PQ first:\t");
      pq_print_node(p);
#endif
      break;
    case i:
      n = pq_node_initialize( &args[current_op] );
      pq_insert( n );
#if defined(GAB_PRINT)
      printf("PQ insert (args=%d,%d):\t",
          args[current_op].key, args[current_op].val);
      pq_print_node(n);
#endif
      break;
    case p:
      n = pq_pop( );
#if defined(GAB_DEBUG)
      if ( kv_key(n) != args[current_op].key ) {
        printf("Invalid node popped (args=%d,%d):\t",
            args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
      if ( kv_value(n) != args[current_op].val ) {
        printf("Unexpected node (non-FIFO/stable) popped (args=%d,%d):\t",
            args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
#endif
#if defined(GAB_PRINT)
      printf("PQ pop (args=%d,%d):\t",
          args[current_op].key, args[current_op].val);
      pq_print_node(n);
#endif
      break;
    case h:
      n = pq_pop( );
#if defined(GAB_DEBUG)
      if ( kv_key(n) != args[current_op].val ) {
        printf("Invalid node popped (args=%d,%d):\t",
            args[current_op].key, args[current_op].val);
        pq_print_node(n);
      }
#endif
      n = pq_add_to_key(n, args[current_op].key);/* add to prio */
#if defined(GAB_PRINT)
      printf("PQ hold (args=%d,%d):\t",
          args[current_op].key, args[current_op].val);
      pq_print_node(n);
#endif
      pq_insert(n);
      break;
    default:
#if defined(GAB_PRINT)
      printf("Invalid Op: %d\n",ops[current_op]);
#endif
      break;
  }
  current_op++;
}

void initialize( void ) {
  /* initialize structures */
  pq_initialize(PQ_MAX_SIZE);
}

static void drain_and_check() {
  uint64_t s = 0;

  uint64_t n;
  while ((n = pq_pop()) != (uint64_t)-1) { // FIXME: casting -1 :(
    s = s + kv_key(n); // only use keys (unstable sort makes values invalid)
  }
  printf("\tChecksum: 0x%llX\n", s);

}

void warmup( void ) {
  int i = 0;

#if defined GAB_PRINT
  printf("Warmup: %d\n", PQ_WARMUP_OPS);
#endif
  for ( i = 0; i < PQ_WARMUP_OPS; i++ ) {
    execute( );
  }
}

void work( void ) {
  int i = 0;
#if defined(GAB_PRINT)
  printf("Work: %d\n", PQ_WORK_OPS);
#endif
  MAGIC(1);
  for ( i = 0; i < PQ_WORK_OPS; i++ ) {
    execute( );
    MAGIC(1);
  }
#if defined(GAB_CHECK)
  drain_and_check();
#endif
}

