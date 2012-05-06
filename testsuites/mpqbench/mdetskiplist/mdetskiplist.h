/*
 * skiplist implementation for priority queue
 * using deterministic skiplist (Munro et al.)
 */

#ifndef __MDETSKIPLIST_H_
#define __MDETSKIPLIST_H_

#include "../shared/pqbench.h"

// FIXME: Derive from parameters? Possible to support dynamic?
#define MAXLEVEL (12)

#include <rtems.h>
#include "rtems/chain.h"

typedef struct {
  int key;
  int val;
} pq_node;

typedef struct {
  rtems_chain_node link[MAXLEVEL]; // FIXME: wasteful :(
  pq_node data;
  int height;
} node;

typedef struct {
  rtems_chain_control lists[MAXLEVEL+1];

  int level;
} skiplist;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((uintptr_t)hn - ((uintptr_t)(&((node *)0)->data))))

#define LINK_TO_NODE(cn, index) \
  ((node*)((uintptr_t)cn - ((uintptr_t)(&(((node *)0)->link[index])))))

#define PQ_NODE_TO_KV(n) ((((long)(n)->key) << (sizeof(long)*4L)) | (long)(n)->val)

void skiplist_initialize( rtems_task_argument tid, int size );
void skiplist_insert( rtems_task_argument tid, long kv );
long skiplist_min( rtems_task_argument tid );
long skiplist_pop_min( rtems_task_argument tid );
long skiplist_search( rtems_task_argument tid, int key );
long skiplist_extract( rtems_task_argument tid, int key );

#endif
