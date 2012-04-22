/*
 * Splay tree implementation for priority queue
 *
 */

#ifndef __MSPLAY_H_
#define __MSPLAY_H_

#include "../shared/pqbench.h"

#define NUM_NODES (PQ_MAX_SIZE)

#include "rtems/chain.h"
#include "rtems/rtems/types.h"

typedef struct {
  int key;
  int val;
} pq_node;

typedef struct _splay_tree_node splay_tree_node;
struct _splay_tree_node
{
    splay_tree_node  * leftlink;
    splay_tree_node  * rightlink;
    splay_tree_node  * uplink;
    int    cnt;

    int key;
};

typedef struct
{
    splay_tree_node  * root;    /* root node */

    /* Statistics, not strictly necessary, but handy for tuning  */

    int    lookups;  /* number of splookup()s */
    int    lkpcmps;  /* number of lookup comparisons */
    
    int    enqs;    /* number of spenq()s */
    int    enqcmps;  /* compares in spenq */
    
    int    splays;
    int    splayloops;

} splay_tree;

typedef struct {
  rtems_chain_node    link;
  splay_tree_node     st_node;
  pq_node             data;
  rtems_id            part_id;
} node;

// container-of magic
#define PQ_NODE_TO_NODE(hn) \
  ((node*)((uintptr_t)hn - ((uintptr_t)(&((node *)0)->data))))
#define PQ_NODE_TO_KV(n) ((((uint64_t)n->key) << 32UL) | (uint64_t)n->val)

#define ST_NODE_TO_NODE(sn) \
  ((node*)((uintptr_t)sn - ((uintptr_t)(&((node *)0)->st_node))))

//extern node the_nodes[NUM_NODES];
//extern splay_tree the_tree;

//extern node *alloc_node(void);
//extern void free_node(node *h);

extern void splay_initialize( rtems_task_argument tid, int size );
extern void splay_insert( rtems_task_argument tid, uint64_t kv );
extern uint64_t splay_min( rtems_task_argument tid );
extern uint64_t splay_pop_min( rtems_task_argument tid );
extern uint64_t splay_search( rtems_task_argument tid, int k );
extern uint64_t splay_extract( rtems_task_argument tid, int k );

#endif
