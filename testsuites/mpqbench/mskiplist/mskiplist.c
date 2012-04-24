
#include "mskiplist.h"

#include <stdlib.h>
#include <tmacros.h>
#include <rtems/chain.h>
#include "../shared/params.h"

/* data structure */
static rtems_chain_control the_skiplist[NUM_APERIODIC_TASKS][MAX_HEIGHT];
/* data */
static node* the_nodes[NUM_APERIODIC_TASKS];
/* free storage */
rtems_chain_control freenodes[NUM_APERIODIC_TASKS];

/* helpers */
static node *alloc_node(rtems_task_argument tid) {
  node *n = rtems_chain_get_unprotected( &freenodes[tid] );
  return n;
}
static void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freenodes[tid], n );
}

static void initialize_helper(rtems_task_argument tid, int size) {
  int i;
  for ( i = 0; i < MAX_HEIGHT; i++ ) {
    rtems_chain_initialize_empty ( &the_skiplist[tid][i] );
  }
}

static inline void insert_helper(rtems_task_argument tid, node *before, node *n)
{
  rtems_chain_insert_unprotected(before, n);
}

/* Returns node with same key, first key greater, or tail of list */
static node* search_helper(rtems_task_argument tid, int key)
{
  rtems_chain_node *iter;
  rtems_chain_control *list;
 
  // FIXME: implement skiplist search
  list = &the_skiplist[tid][0];
  iter = rtems_chain_first(list); // unprotected
  while ( !rtems_chain_is_tail(list, iter) ) {
    node *n = (node*)iter;
    if (n->data.key >= key) {
      return n;
    }
    iter = rtems_chain_next(iter);
  }
  return (node*)iter;
}

static inline void extract_helper(rtems_task_argument tid, node *n)
{
  // FIXME: update all lists pointing to the node... or mark node for deletion
  // and deal with it later.
  if (rtems_chain_is_first(n)) {
    rtems_chain_get_unprotected(&the_skiplist[tid][0]);
  } else {
    rtems_chain_extract_unprotected(n);
  }
  free_node(tid, n);
}

/**
 * benchmark interface
 */
void skiplist_initialize( rtems_task_argument tid, int size ) {
  int i;

  the_nodes[tid] = (node*)malloc(sizeof(node)*size);
  if ( ! the_nodes[tid] ) {
    printk("failed to alloc nodes\n");
    while(1);
  }

  rtems_chain_initialize_empty ( &freenodes[tid] );
  for ( i = 0; i < size; i++ ) {
    rtems_chain_append_unprotected(&freenodes[tid], &the_nodes[tid][i].link);
  }

  initialize_helper(tid, size);
}

void skiplist_insert(rtems_task_argument tid, uint64_t kv ) {
  node *new_node = alloc_node(tid);
  node *target;
  int key = kv_key(kv);

  target = search_helper(tid, key);

  if ( !rtems_chain_is_first(target) )
    target = rtems_chain_previous(target);

  new_node->data.key = kv_key(kv);
  new_node->data.val = kv_value(kv);
  insert_helper(tid, target, new_node);
}

uint64_t skiplist_min( rtems_task_argument tid ) {
  node *n;

  n = rtems_chain_first(&the_skiplist[tid][0]); // unprotected
  if (n) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (uint64_t)-1;
}

uint64_t skiplist_pop_min( rtems_task_argument tid ) {
  uint64_t kv;
  node *n;
  n = rtems_chain_get_unprotected(&the_skiplist[tid][0]);
  if (n) {
    kv = PQ_NODE_TO_KV(&n->data);
    free_node(tid, n);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t skiplist_search( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
  if (!rtems_chain_is_tail(&the_skiplist[tid][0], n)) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (uint64_t)-1;
}

uint64_t skiplist_extract( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
  uint64_t kv;
  if (!rtems_chain_is_tail(&the_skiplist[tid][0], n) && n->data.key == k) {
    kv = PQ_NODE_TO_KV(&n->data);
    extract_helper(tid, n);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

