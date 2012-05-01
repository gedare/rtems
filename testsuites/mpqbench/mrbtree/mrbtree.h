/*
 * Heap implementation for priority queue
 *
 */

#ifndef __MRBTREE_H_
#define __MRBTREE_H_

#include "../shared/pqbench.h"

#define NUM_NODES (PQ_MAX_SIZE)

#include "rtems/chain.h"
#include "rtems/rbtree.h"
#include "rtems/rtems/types.h"

typedef struct {
  int key;
  int val;
} pq_node;

typedef struct {
  rtems_chain_node    link;
  rtems_rbtree_node   rbt_node;
  pq_node             data;
} node;

//extern rtems_chain_control freelist;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((size_t)hn - ((size_t)(&((node *)0)->data))))
#define PQ_NODE_TO_KV(n) ((((long)n->key) << (sizeof(long)*4L)) | (long)n->val)

//extern node the_nodes[NUM_NODES];
//extern rtems_rbtree_control the_rbtree;

//node *alloc_node(void);
//void free_node(node *h);

extern void rbtree_initialize( rtems_task_argument tid, int size );
extern void rbtree_insert( rtems_task_argument tid, long kv );
extern long rbtree_min( rtems_task_argument tid );
extern long rbtree_pop_min( rtems_task_argument tid );
extern long rbtree_search( rtems_task_argument tid, int k );
extern long rbtree_extract( rtems_task_argument tid, int k );

#endif
