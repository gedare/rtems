/*
 * Splay tree implementation for priority queue
 *
 */

#ifndef __MNEWSPLAY_H_
#define __MNEWSPLAY_H_

#include "../shared/pqbench.h"

#define NUM_NODES (PQ_MAX_SIZE)

#include "rtems/chain.h"
#include "rtems/rtems/types.h"

#include <rtems/splay.h>

typedef struct {
  int key;
  int val;
} pq_node;

typedef struct {
  rtems_chain_node    link;
  rtems_splay_node    st_node;
  pq_node             data;
} node;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((uintptr_t)hn - ((uintptr_t)(&((node *)0)->data))))
#define PQ_NODE_TO_KV(n) ((((long)n->key) << (sizeof(long)*4L)) | (long)n->val)

#define ST_NODE_TO_NODE(sn) \
  ((node*)((uintptr_t)sn - ((uintptr_t)(&((node *)0)->st_node))))

//extern node the_nodes[NUM_NODES];
//extern splay_tree the_tree;

//extern node *alloc_node(void);
//extern void free_node(node *h);

extern void newsplay_initialize( rtems_task_argument tid, int size );
extern void newsplay_insert( rtems_task_argument tid, long kv );
extern long newsplay_min( rtems_task_argument tid );
extern long newsplay_pop_min( rtems_task_argument tid );
extern long newsplay_search( rtems_task_argument tid, int k );
extern long newsplay_extract( rtems_task_argument tid, int k );

#endif
