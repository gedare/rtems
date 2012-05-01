/*
 * linked list implementation for priority queue
 */

#ifndef __MLIST_H_
#define __MLIST_H_

#include "../shared/pqbench.h"

#include <rtems.h>
#include "rtems/chain.h"

typedef struct {
  int key;
  int val;
} pq_node;

typedef struct {
  rtems_chain_node link;
  pq_node data;
} node;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((uintptr_t)hn - ((uintptr_t)(&((node *)0)->data))))

#define PQ_NODE_TO_KV(n) ((((long)(n)->key) << (sizeof(long)*4L)) | (long)(n)->val)

void list_initialize( rtems_task_argument tid, int size );
void list_insert( rtems_task_argument tid, long kv );
long list_min( rtems_task_argument tid );
long list_pop_min( rtems_task_argument tid );
long list_search( rtems_task_argument tid, int key );
long list_extract( rtems_task_argument tid, int key );

#endif
