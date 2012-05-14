
#include "mnewsplay.h"

#include <stdlib.h>
#include <tmacros.h>

#if defined(GAB_DEBUG)
#include <assert.h>
#else
#define assert(x) { ; }
#endif

static rtems_splay_control the_tree[NUM_APERIODIC_TASKS];

node the_nodes[NUM_NODES][NUM_APERIODIC_TASKS];
rtems_chain_control freelist[NUM_APERIODIC_TASKS];

static node *alloc_node(rtems_task_argument tid) {
  node *n = rtems_chain_get_unprotected( &freelist[tid] );
  return n;
}
static void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freelist[tid], n );
}

static int splay_compare(rtems_splay_node *n1, rtems_splay_node *n2)
{
  int key1 = ST_NODE_TO_NODE(n1)->data.key;
  int key2 = ST_NODE_TO_NODE(n2)->data.key;
  return key1 - key2;
}

void newsplay_initialize(rtems_task_argument tid, int size ) {
  int i;

  rtems_chain_initialize_empty ( &freelist[tid] );
  for ( i = 0; i < size; i++ ) {
    rtems_chain_append(&freelist[tid], &the_nodes[i][tid].link);
  }

  rtems_splay_initialize_empty(&the_tree[tid], splay_compare);
}

void newsplay_insert( rtems_task_argument tid, long kv ) {
  node *n = alloc_node(tid);
  pq_node *pn = &n->data;
  pn->key = kv_key(kv);
  pn->val = kv_value(kv);
  rtems_splay_insert( &the_tree[tid], &n->st_node );
}

long newsplay_min(rtems_task_argument tid ) {
  long kv;
  rtems_splay_node *stn;
  node *n;
  pq_node *p;

  stn = rtems_splay_dequeue( &the_tree[tid] ); // TODO: peek without remove
  if ( stn ) {
    rtems_splay_insert(&the_tree[tid], stn); // FIXME ^
    assert ( stn == the_tree[tid].first[TREE_LEFT] ); 
    n = ST_NODE_TO_NODE(stn);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
    return kv;
  } 
  return (long)-1; // FIXME: error handling
}

long newsplay_pop_min( rtems_task_argument tid) {
  long kv;
  node *n;
  pq_node *p;
  rtems_splay_node *stn;

  stn = rtems_splay_dequeue( &the_tree[tid] ); // TODO: use O(1) dequeue

  if ( stn ) {
    n = ST_NODE_TO_NODE(stn);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
    free_node(tid,n);
  } else {
    kv = (long)-1;
  }
  return kv;
}

long newsplay_search( rtems_task_argument tid, int k) {
  long kv;
  rtems_splay_node *stn;
  rtems_splay_control *tree;
  node *n;
  pq_node *p;
  node search_node;
  tree = &the_tree[tid];
  search_node.data.key = k;
  
  stn = rtems_splay_find(tree, &search_node.st_node);
  if ( stn ) {
    n = ST_NODE_TO_NODE(stn);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
  } else {
    kv = (long)-1;
  }
  return kv;
}

long newsplay_extract( rtems_task_argument tid, int k) {
  long kv;
  rtems_splay_node *stn;
  rtems_splay_control *tree;
  node *n;
  pq_node *p;
  node search_node;
  search_node.data.key = k;

  tree = &the_tree[tid];
  stn = rtems_splay_extract(tree, &search_node.st_node);
  if ( stn ) {
    n = ST_NODE_TO_NODE(stn);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
    free_node(tid, n);
  } else {
    kv = (long)-1;
  }
  return kv;
}
