
#ifndef __SPARC64_SPILLPQ_H
#define __SPARC64_SPILLPQ_H


#include <rtems/score/chain.h>
#include <rtems/rtems/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kv_value(kv) ((uint32_t)(kv))
#define kv_key(kv)   ((kv)>>32)
// following works if the node defines key and val as field names...
#define pq_node_to_kv(n) ((((uint64_t)(n)->key) << 32UL) | (uint64_t)(n)->val)

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
  SpillPQ_Function  search;
  SpillPQ_Function  spill;
  SpillPQ_Function  fill;
  SpillPQ_Function  drain;
} sparc64_spillpq_operations;


#define NUM_QUEUES (10) // FIXME:number of queues...

// HWPQ context
typedef struct {
  int max_size;
  int current_qid;
} hwpq_context_t;

typedef struct {

  // shorthand for a realtime policy.
  bool realtime;
  
  bool pinned; // this hwds does not get switched involuntarily

  int spill_from;

  // upper limit for filling, spilling
  int fill_max;
  int spill_max;
} spillpq_policy_t;

typedef struct {
  int spills;
  int fills;
  int switches;
  int firsts;
  int inserts;
  int extracts;
  int searches;
  int pops;
} spillpq_stats_t;

typedef struct {
  sparc64_spillpq_operations *ops;
  int max_size;
  int cs_count;
  spillpq_policy_t policy;
  spillpq_stats_t stats;
} spillpq_context_t;

extern spillpq_context_t spillpq[NUM_QUEUES];

#define SPILLPQ_POLICY_DEFAULT  (0)
#define SPILLPQ_POLICY_RT       (1)

#define SPILLPQ_POLICY_PIN      (2)

extern hwpq_context_t *hwpq_context;
extern int sparc64_spillpq_hwpq_context_initialize( int, hwpq_context_t* );
extern int sparc64_spillpq_hwpq_set_max_size(int queue_idx, int size);

extern int sparc64_spillpq_initialize(
  int queue_idx,
  spillpq_policy_t *policy,
  sparc64_spillpq_operations *ops,
  size_t max_pq_size
);
extern uint64_t sparc64_spillpq_first(int queue_idx, uint64_t kv);
extern uint64_t sparc64_spillpq_insert(int queue_idx, uint64_t kv);
extern uint64_t sparc64_spillpq_extract(int queue_idx, uint64_t kv);
extern uint64_t sparc64_spillpq_search(int queue_idx, uint64_t kv);
extern uint64_t sparc64_spillpq_pop(int queue_idx, uint64_t kv);
extern int sparc64_spillpq_handle_spill(int queue_idx); /* FIXME: count */
extern int sparc64_spillpq_handle_fill(int queue_idx);
extern int sparc64_spillpq_handle_failover(int queue_idx, uint32_t trap_ctx);
extern int sparc64_spillpq_drain( int queue_id );
extern int sparc64_spillpq_context_save( int queue_idx );
extern int sparc64_spillpq_context_restore( int queue_idx );
extern int sparc64_spillpq_context_switch( int from_idx, int trap_idx );

extern uint64_t sparc64_spillpq_null_handler(int qid, uint64_t arg);

#ifdef __cplusplus
}
#endif

#endif
