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

typedef struct _splay_tree_node splay_tree_node;
struct _splay_tree_node
{
    splay_tree_node  * leftlink;
    splay_tree_node  * rightlink;
    splay_tree_node  * uplink;

    int key;
    int val;
};

typedef struct
{
    splay_tree_node  * root;    /* root node */

    /* Statistics, not strictly necessary, but handy for tuning  */
#if defined(SPLAY_STATS)
    int    lookups;  /* number of splookup()s */
    int    lkpcmps;  /* number of lookup comparisons */
    
    int    enqs;    /* number of spenq()s */
    int    enqcmps;  /* compares in spenq */
    
    int    splays;
    int    splayloops;
#endif
} splay_tree;

typedef struct {
  rtems_chain_node    link;
  splay_tree_node     st_node;
  rtems_id            part_id;
} node;

// container-of magic
#define ST_NODE_TO_KV(n) ((((long)n->key) << (sizeof(long)*4L)) | (long)n->val)

#define ST_NODE_TO_NODE(sn) \
  ((node*)((uintptr_t)sn - ((uintptr_t)(&((node *)0)->st_node))))

//extern node the_nodes[NUM_NODES];
//extern splay_tree the_tree;

//extern node *alloc_node(void);
//extern void free_node(node *h);

extern void splay_initialize( rtems_task_argument tid, int size );
extern void splay_insert( rtems_task_argument tid, long kv );
extern long splay_min( rtems_task_argument tid );
extern long splay_pop_min( rtems_task_argument tid );
extern long splay_search( rtems_task_argument tid, int k );
extern long splay_extract( rtems_task_argument tid, int k );

#endif
