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

extern rtems_chain_control freelist;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((size_t)hn - ((size_t)(&((node *)0)->data))))
#define PQ_NODE_TO_KV(n) ((((uint64_t)n->key) << 32UL) | (uint64_t)n->val)

extern node the_nodes[NUM_NODES];
extern rtems_rbtree_control the_rbtree;

node *alloc_node(void);
void free_node(node *h);

void rbtree_initialize( int size );
void rbtree_insert( uint64_t kv );
uint64_t rbtree_min( void );
uint64_t rbtree_pop_min( void );

#endif
