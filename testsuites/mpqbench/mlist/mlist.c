
#include "mlist.h"

#include <stdlib.h>
#include <tmacros.h>
#include <rtems/chain.h>
#include "../shared/params.h"

/* data structure */
static rtems_chain_control the_list[NUM_APERIODIC_TASKS];
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

/*
 * Returns either the pq_node with key = kv_key(kv), the first node with
 * a greater key, or the tail of the list in case no node has a greater key
 */
static node* search_helper(rtems_task_argument tid, int key)
{
  rtems_chain_node *iter;
  rtems_chain_control *list;
  
  list = &the_list[tid];
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

/**
 * benchmark interface
 */
void list_initialize( rtems_task_argument tid, int size ) {
  int i;

  the_nodes[tid] = (node*)malloc(sizeof(node)*size);
  if ( ! the_nodes[tid] ) {
    printk("failed to alloc nodes\n");
    while(1);
  }

  rtems_chain_initialize_empty ( &the_list[tid] );

  rtems_chain_initialize_empty ( &freenodes[tid] );
  for ( i = 0; i < size; i++ ) {
    rtems_chain_append_unprotected(&freenodes[tid], &the_nodes[tid][i].link);
  }
}

void list_insert(rtems_task_argument tid, uint64_t kv ) {
  node *new_node = alloc_node(tid);
  rtems_chain_node *target;
  int key = kv_key(kv);

  target = (rtems_chain_node*)search_helper(tid, key);

  if ( !rtems_chain_is_first(target) )
    target = rtems_chain_previous(target);

  new_node->data.key = kv_key(kv);
  new_node->data.val = kv_value(kv);
  rtems_chain_insert_unprotected(target, new_node);
}

uint64_t list_min( rtems_task_argument tid ) {
  node *n;

  n = rtems_chain_first(&the_list[tid]); // unprotected
  if (n) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (uint64_t)-1;
}

uint64_t list_pop_min( rtems_task_argument tid ) {
  uint64_t kv;
  node *n;
  n = rtems_chain_get_unprotected(&the_list[tid]);
  if (n) {
    kv = PQ_NODE_TO_KV(&n->data);
    free_node(tid, n);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t list_search( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
  if (!rtems_chain_is_tail(&the_list[tid], n)) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (uint64_t)-1;
}

uint64_t list_extract( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
  uint64_t kv;
  if (!rtems_chain_is_tail(&the_list[tid], n) && n->data.key == k) {
    kv = PQ_NODE_TO_KV(&n->data);
    if (rtems_chain_is_first(n)) {
      rtems_chain_get_unprotected(&the_list[tid]);
    } else {
      rtems_chain_extract_unprotected(n);
    }
    free_node(tid, n);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

