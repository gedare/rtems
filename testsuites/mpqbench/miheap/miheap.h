/*
 * Heap implementation for priority queue
 *
 */

#ifndef __MIMPLICITHEAP_H_
#define __MIMPLICITHEAP_H_

#include "../shared/pqbench.h"
#define NUM_NODES (PQ_MAX_SIZE)

#include <rtems.h>
#include "rtems/chain.h"

typedef struct {
  rtems_chain_node link;
  int key;
  int val;
  int hIndex;
} node;

#define HEAP_PARENT(i) (i>>1)
#define HEAP_FIRST (1)
#define HEAP_LEFT(i) (i<<1)
#define HEAP_RIGHT(i) (HEAP_LEFT(i)+1)

#define HEAP_NODE_TO_KV(n) ((((uint64_t)n->key) << 32UL) | (uint64_t)n->val)

// container-of magic
#define HEAP_NODE_TO_NODE(hn) \
  ((node*)((size_t)hn - ((size_t)(&((node *)0)->data))))

void heap_initialize( rtems_task_argument tid, int size );
void heap_insert( rtems_task_argument tid, uint64_t kv );
void heap_remove( rtems_task_argument tid, int index );
void heap_change_key( rtems_task_argument tid, int index, int new_key );
void heap_increase_key( rtems_task_argument tid, int index, int new_key );
void heap_decrease_key( rtems_task_argument tid, int index, int new_key );
uint64_t heap_min( rtems_task_argument tid );
uint64_t heap_pop_min( rtems_task_argument tid );

#endif
