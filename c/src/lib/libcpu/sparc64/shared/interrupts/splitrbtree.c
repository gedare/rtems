#include "spillpq.h"
#include "freelist.h"
#include "gabdebug.h"

#include "splitrbtree.h"
#include <rtems/rbtree.h>

static Freelist_Control free_nodes[NUM_QUEUES];

/* split pq: the rbtree */
static rtems_rbtree_control trees[NUM_QUEUES];

typedef struct {
  union {
    Chain_Node Node;
    rtems_rbtree_node rbt_node;
  };
  uint32_t key;
  uint32_t val;
} pq_node;

int rbtree_compare(
  rtems_rbtree_node* n1,
  rtems_rbtree_node* n2
) {
  int key1 = rtems_rbtree_container_of( n1, pq_node, rbt_node )->key;
  int key2 = rtems_rbtree_container_of( n2, pq_node, rbt_node )->key;

  return key1 - key2;
}


int sparc64_splitrbtree_initialize( int tid, size_t max_pq_size )
{
  int i;
  uint64_t reg = 0;
  freelist_initialize(&free_nodes[tid], sizeof(pq_node), max_pq_size);

  rtems_rbtree_initialize_empty(&trees[tid], &rbtree_compare, false);
  spillpq_queue_max_size[tid] = max_pq_size;

  return 0;
}

uint64_t sparc64_splitrbtree_insert(int tid, uint64_t kv)
{
  pq_node *new_node;
  new_node = freelist_get_node(&free_nodes[tid]);
  if (!new_node) {
    printk("%d\tUnable to allocate new node during insert\n", tid);
    while (1);
  }
  new_node->key = kv_key(kv);
  new_node->val = kv_value(kv); // FIXME: not full 64-bits

  rtems_rbtree_insert_unprotected( &trees[tid], &new_node->rbt_node );
  return 0;
}

uint64_t sparc64_splitrbtree_first(int tid, uint64_t kv)
{
  pq_node *p;
  rtems_rbtree_node *first;
  
  first = rtems_rbtree_min(&trees[tid]);
  if ( first ) {
    p = rtems_rbtree_container_of(first, pq_node, rbt_node);
    kv = pq_node_to_kv(p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t sparc64_splitrbtree_pop(int tid, uint64_t kv)
{
  rtems_rbtree_node *first;
  pq_node *p;
  first = rtems_rbtree_get_min_unprotected(&trees[tid]);
  if ( first ) {
    p = rtems_rbtree_container_of(first, pq_node, rbt_node);
    kv = pq_node_to_kv(p);
    freelist_put_node(&free_nodes[tid], p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

static rtems_rbtree_node* search_helper(int tid, uint64_t kv)
{
  rtems_rbtree_node *n;
  pq_node search_node;

  search_node.key = kv_key(kv);
  return rtems_rbtree_find_unprotected(&trees[tid], &search_node.rbt_node);
}

uint64_t sparc64_splitrbtree_extract(int tid, uint64_t kv )
{
  rtems_rbtree_node *n;
  pq_node *p;

  n = search_helper(tid, kv);

  if ( n ) {
    p = rtems_rbtree_container_of(n, pq_node, rbt_node);
    rtems_rbtree_extract_unprotected(&trees[tid], n);
    kv = pq_node_to_kv(p);
    freelist_put_node(&free_nodes[tid], p);
  } else {
    DPRINTK("%d: Failed extract: %d\t%X\n", tid, kv_key(kv), kv_value(kv));
    kv = (uint64_t)-1;
  }

  return kv;
}

uint64_t sparc64_splitrbtree_search(int tid, uint64_t kv )
{
  rtems_rbtree_node *n;
  pq_node *p;
  n = search_helper(tid, kv);

  if ( n ) {
    p = rtems_rbtree_container_of(n, pq_node, rbt_node);
    kv = pq_node_to_kv(p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

static inline uint64_t 
sparc64_splitrbtree_spill_node(int tid)
{
  uint64_t kv;

  HWDS_SPILL(tid, kv);
  if (!kv) {
    DPRINTK("%d\tNothing to spill!\n", tid);
  } else {
    sparc64_splitrbtree_insert(tid, kv);
  }

  return kv;
}

static inline uint64_t
sparc64_splitrbtree_fill_node(int tid)
{
  uint32_t exception;
  uint64_t kv;

  kv = sparc64_splitrbtree_pop(tid, 0);

  // add node to hwpq
  HWDS_FILL(tid, kv_key(kv), kv_value(kv), exception); 

  if (exception) {
    DPRINTK("Spilling (%d,%X) while filling\n");
    return sparc64_splitrbtree_spill_node(tid);
  }

  return 0;
}

uint64_t sparc64_splitrbtree_handle_spill( int tid, uint64_t count )
{
  int i = 0;

  // pop elements off tail of hwpq, merge into software pq
  while ( i < count ) {
    if (!sparc64_splitrbtree_spill_node(tid))
      break;
    i++;
  }

  return i;
}

/*
 * Current algorithm pulls nodes from the head of the sorted sw pq
 * and fills them into the hw pq.
 */
uint64_t sparc64_splitrbtree_handle_fill(int tid, uint64_t count )
{
  int i = 0;

  while (!rtems_rbtree_is_empty( &trees[tid] ) && i < count) {
    i++;
    sparc64_splitrbtree_fill_node(tid);
  }

  return 0;
}

uint64_t sparc64_splitrbtree_drain( int tid, uint64_t ignored )
{
  return 0;
}

sparc64_spillpq_operations sparc64_splitrbtree_ops = {
  sparc64_splitrbtree_initialize,
  sparc64_splitrbtree_insert,
  sparc64_splitrbtree_first,
  sparc64_splitrbtree_pop,
  sparc64_splitrbtree_extract,
  sparc64_splitrbtree_search,
  sparc64_splitrbtree_handle_spill,
  sparc64_splitrbtree_handle_fill,
  sparc64_splitrbtree_drain
};

