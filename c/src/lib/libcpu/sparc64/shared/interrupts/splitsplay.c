#include "spillpq.h"
#include "freelist.h"
#include "gabdebug.h"

#include "splitsplay.h"
#include <rtems/splay.h>

#define _Container_of(_node, _container_type, _node_field_name) \
( \
  (_container_type*) \
  ( (uintptr_t)(_node) - offsetof(_container_type, _node_field_name) ) \
)

static Freelist_Control free_nodes[NUM_QUEUES];

/* split pq: the splay */
static rtems_splay_control trees[NUM_QUEUES];

typedef struct {
  union {
    Chain_Node Node;
    rtems_splay_node st_node;
  };
  uint32_t key;
  uint32_t val;
} pq_node;

static int sparc64_splitsplay_compare(
  rtems_splay_node* n1,
  rtems_splay_node* n2
) {
  int key1 = _Container_of( n1, pq_node, st_node )->key;
  int key2 = _Container_of( n2, pq_node, st_node )->key;

  return key1 - key2;
}

int sparc64_splitsplay_initialize( int qid, size_t max_pq_size )
{
  rtems_splay_compare_function cf = sparc64_splitsplay_compare;
  freelist_initialize(&free_nodes[qid], sizeof(pq_node), max_pq_size, NULL);


  rtems_splay_initialize_empty(&trees[qid], cf);
  spillpq[qid].max_size = max_pq_size;

  return 0;
}

uint64_t sparc64_splitsplay_insert(int qid, uint64_t kv)
{
  pq_node *new_node;
  new_node = freelist_get_node(&free_nodes[qid]);
  if (!new_node) {
    printk("%d\tUnable to allocate new node during insert\n", qid);
    while (1);
  }
  new_node->key = kv_key(kv);
  new_node->val = kv_value(kv); // FIXME: not full 64-bits

  rtems_splay_insert( &trees[qid], &new_node->st_node );
  return 0;
}

uint64_t sparc64_splitsplay_first(int qid, uint64_t kv)
{
  pq_node *p;
  rtems_splay_node *first;
 
  first = rtems_splay_min(&trees[qid]);
  if ( first ) {
    p = _Container_of(first, pq_node, st_node);
    kv = pq_node_to_kv(p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t sparc64_splitsplay_pop(int qid, uint64_t kv)
{
  rtems_splay_node *first;
  pq_node *p;
  first = rtems_splay_get_min(&trees[qid]);
  if ( first ) {
    p = _Container_of(first, pq_node, st_node);
    kv = pq_node_to_kv(p);
    freelist_put_node(&free_nodes[qid], p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

static rtems_splay_node* search_helper(int qid, uint64_t kv)
{
  rtems_splay_node *n;
  rtems_splay_control *tree;
  pq_node search_node;
  search_node.key = kv_key(kv);
  tree = &trees[qid];
  n = rtems_splay_find(tree, &search_node.st_node);
  return n;
}

uint64_t sparc64_splitsplay_extract(int qid, uint64_t kv )
{
  rtems_splay_node *n;
  rtems_splay_control *tree;
  pq_node *p;

  tree = &trees[qid];
  n = rtems_splay_extract(tree, search_helper(qid, kv));

  if ( n ) {
    p = _Container_of(n, pq_node, st_node);
    kv = pq_node_to_kv(p);
    freelist_put_node(&free_nodes[qid], p);
  } else {
    DPRINTK("%d: Failed extract: %d\t%X\n", qid, kv_key(kv), kv_value(kv));
    kv = (uint64_t)-1;
  }

  return kv;
}

uint64_t sparc64_splitsplay_search(int qid, uint64_t kv )
{
  rtems_splay_node *n;
  pq_node *p;
  n = search_helper(qid, kv);

  if ( n ) {
    p = _Container_of(n, pq_node, st_node);
    kv = pq_node_to_kv(p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

static inline uint64_t 
sparc64_splitsplay_spill_node(int qid)
{
  uint64_t kv;

  HWDS_SPILL(qid, kv);
  if (!kv) {
    DPRINTK("%d\tNothing to spill!\n", qid);
  } else {
    sparc64_splitsplay_insert(qid, kv);
  }

  return kv;
}

uint64_t sparc64_splitsplay_handle_spill( int qid, uint64_t count )
{
  int i = 0;

  // pop elements off tail of hwpq, merge into software pq
  while ( i < count ) {
    if (!sparc64_splitsplay_spill_node(qid))
      break;
    i++;
  }

  return i;
}

static inline uint64_t
sparc64_splitsplay_fill_node(int qid, int count)
{
  uint32_t exception;
  uint64_t kv;

  kv = sparc64_splitsplay_pop(qid, 0);

  // add node to hwpq
  HWDS_FILL(qid, kv_key(kv), kv_value(kv), exception); 

  if (exception) {
    DPRINTK("Spilling (%d,%X) while filling\n");
    return sparc64_splitsplay_handle_spill(qid, count);
  }

  return 0;
}

/*
 * Current algorithm pulls nodes from the head of the sorted sw pq
 * and fills them into the hw pq.
 */
uint64_t sparc64_splitsplay_handle_fill(int qid, uint64_t count )
{
  int i = 0;

  while (!_Splay_Is_empty( &trees[qid] ) && i < count) {
    i++;
    sparc64_splitsplay_fill_node(qid, count);
  }

  return 0;
}

uint64_t sparc64_splitsplay_drain( int qid, uint64_t ignored )
{
  return 0;
}

sparc64_spillpq_operations sparc64_splitsplay_ops = {
  sparc64_splitsplay_initialize,
  sparc64_splitsplay_insert,
  sparc64_splitsplay_first,
  sparc64_splitsplay_pop,
  sparc64_splitsplay_extract,
  sparc64_splitsplay_search,
  sparc64_splitsplay_handle_spill,
  sparc64_splitsplay_handle_fill,
  sparc64_splitsplay_drain
};

