/*
 * skiplist implementation for priority queue
 */

#ifndef __MSKIPLIST_H_
#define __MSKIPLIST_H_

#include "../shared/pqbench.h"

// FIXME: Derive from parameters? Possible to support dynamic?
#define MAX_HEIGHT (5)

#include <rtems.h>
#include "rtems/chain.h"

typedef struct {
  int key;
  int val;
} pq_node;

typedef struct {
  rtems_chain_node link[MAX_HEIGHT];
  pq_node data;
} node;

typedef struct {
  rtems_chain_control lists[MAX_HEIGHT];

  int height;
} skiplist;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((uintptr_t)hn - ((uintptr_t)(&((node *)0)->data))))

#define PQ_NODE_TO_KV(n) ((((uint64_t)(n)->key) << 32UL) | (uint64_t)(n)->val)

void skiplist_initialize( rtems_task_argument tid, int size );
void skiplist_insert( rtems_task_argument tid, uint64_t kv );
uint64_t skiplist_min( rtems_task_argument tid );
uint64_t skiplist_pop_min( rtems_task_argument tid );
uint64_t skiplist_search( rtems_task_argument tid, int key );
uint64_t skiplist_extract( rtems_task_argument tid, int key );

#endif
