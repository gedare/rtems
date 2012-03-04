
#ifndef __SPARC64_SPILLPQ_H
#define __SPARC64_SPILLPQ_H


#include <rtems/score/chain.h>
#include <rtems/rtems/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kv_value(kv) ((uint32_t)kv)
#define kv_key(kv)   (kv>>32)
// following works if the node defines key and val as field names...
#define pq_node_to_kv(n) ((((uint64_t)n->key) << 32UL) | (uint64_t)n->val)

//#define GAB_DEBUG

#include <rtems/bspIo.h>

typedef uint64_t (*SpillPQ_Function)(int qid, uint64_t arg);

/* TODO: make all spill pq operations available */
typedef struct {
  SpillPQ_Function  initialize;
  SpillPQ_Function  insert;
  SpillPQ_Function  first;
  SpillPQ_Function  pop;
  SpillPQ_Function  extract;
  SpillPQ_Function  spill;
  SpillPQ_Function  fill;
  SpillPQ_Function  drain;
  SpillPQ_Function  context_switch;
} sparc64_spillpq_operations;

extern sparc64_spillpq_operations *spillpq_ops;
#define NUM_QUEUES (10)
extern size_t spillpq_queue_max_size[NUM_QUEUES];

extern int sparc64_spillpq_initialize( int queue_idx, size_t max_pq_size );
extern int sparc64_spillpq_insert(int queue_idx, uint64_t kv);
extern uint64_t sparc64_spillpq_first(int queue_idx);
extern uint64_t sparc64_spillpq_pop(int queue_idx);
extern int sparc64_spillpq_handle_extract(int queue_idx, uint64_t kv);
extern int sparc64_spillpq_handle_spill(int queue_idx, int count);
extern int sparc64_spillpq_handle_fill(int queue_idx, int count);
extern int sparc64_spillpq_drain( int queue_id );
extern int sparc64_spillpq_context_switch( int queue_id );

extern uint64_t sparc64_spillpq_null_handler(int qid, uint64_t arg);

#ifdef __cplusplus
}
#endif

#endif
