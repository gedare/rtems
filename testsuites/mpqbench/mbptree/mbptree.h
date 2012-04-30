/*
 * linked list implementation for priority queue
 */

#ifndef __MBTREE_H_
#define __MBTREE_H_

#include "../shared/pqbench.h"

#include <rtems.h>
#include "rtems/chain.h"

#define NODES_PER_BLOCK (4)
#define MIN_NODES ( (NODES_PER_BLOCK + 1) / 2 )
#define CHILDREN_PER_BLOCK (NODES_PER_BLOCK + 1)
#define MIN_CHILDREN ( (CHILDREN_PER_BLOCK+1)/2 )
#define MAX_CHILDREN CHILDREN_PER_BLOCK

typedef struct {
  int key;
  int val;
} pq_node;

typedef struct {
  rtems_chain_node link;
  pq_node data;
} node;

typedef struct _bptree_block bptree_block;

struct _bptree_block {
  rtems_chain_node link;
  node *nodes[NODES_PER_BLOCK+1];  /* FIXME: unionize nodes&children add keys */
  bptree_block *children[CHILDREN_PER_BLOCK+1];
  bptree_block *parent;
  int num_nodes;
  bool is_leaf;
};

typedef struct {
  bptree_block *root;
  int id;
  int t;  /* sharing threshold: 0 <= t <= b+1-2a */
  int s;  /* sharing amount: 1 <= s <= t+1 */
} bptree;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((uintptr_t)hn - ((uintptr_t)(&((node *)0)->data))))

#define PQ_NODE_TO_KV(n) ((((uint64_t)(n)->key) << 32UL) | (uint64_t)(n)->val)

void bptree_initialize( rtems_task_argument tid, int size );
void bptree_insert( rtems_task_argument tid, uint64_t kv );
uint64_t bptree_min( rtems_task_argument tid );
uint64_t bptree_pop_min( rtems_task_argument tid );
uint64_t bptree_search( rtems_task_argument tid, int key );
uint64_t bptree_extract( rtems_task_argument tid, int key );

#endif
