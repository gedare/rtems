#include "spillpq.h"
#include "freelist.h"
#include "gabdebug.h"


/* B+ tree */
typedef struct {
  Chain_Node Node;
  uint32_t key;
  uint32_t val;
} pq_node;

#define NODES_PER_BLOCK (4)
#define CHILDREN_PER_BLOCK (NODES_PER_BLOCK + 1)
#define MIN_CHILDREN ( (CHILDREN_PER_BLOCK+1)/2 )
#define MAX_CHILDREN CHILDREN_PER_BLOCK

typedef struct {
  pq_node *nodes_list;  /* FIXME: array or linked list? */
  struct btree_block *children[CHILDREN_PER_BLOCK];
  struct btree_block *parent;
  int node_count;
  bool is_leaf;
} btree_block;

static inline bool btree_block_is_full(btree_node *block)
{
  return (block->node_count == NODES_PER_BLOCK);
}

static inline int btree_index_in_block(btree_node *block, int key)
{
  int i;
  pq_node *n = nodes_list;

  /* FIXME: binary search? */
  for ( i = 0; i < block->node_count && n->key < key; i++ ) {
    n = _Chain_Next((Chain_Node*)n);
  }
  return i;
}

static inline pq_node* btree_node_in_block(btree_node *block, int key)
{
  int i;
  pq_node *n = nodes_list;

  /* FIXME: binary search? */
  for ( i = 0; i < block->node_count && n->key < key; i++ ) {
    n = _Chain_Next((Chain_Node*)n);
  }
  return n;
}

static btree_block* btree_find_block(btree_block *root, int key)
{
  btree_block *iter = root;
  while (!iter->is_leaf) {
    iter = iter->children[btree_index_in_block(iter, key)];
  }
  return iter;
}

static pq_node* btree_search(btree_block *root, int key)
{
  btree_block *the_block = btree_find_block(root, key);
  return btree_node_in_block(the_block, key);
}

static inline void insert_helper(pq_node *after, pq_node *new_node)
{
  prev = (_Chain_Node*)after;
  if ( !_Chain_Is_first((_Chain_Node*)after) ) {
    prev = _Chain_Previous(prev);
  }
  _Chain_Insert_unprotected(prev, (Chain_Node*)new_node);
}


static inline pq_node* split(pq_node *n, pq_node *new_node, int count)
{
  int i;
  Chain_Node *prev;

  for ( i = 0; i < count; i++ ) {
    if ( new_node && new_node->key < n->key ) {
      insert_helper(n, new_node);
      new_node = NULL;
      continue;
    }
    n = (pq_node*)_Chain_Next((_Chain_Node*)n);
  }
  if ( new_node && new_node->key < n->key ) {
    insert_helper(n, new_node);
    return new_node;
  }
  return n;
}


static void btree_split(btree_block *b, pq_node *new_node)
{
  if ( !b->parent ) { /* b is the root */

  }

  btree *left_block = freelist_get_node(&free_blocks[queue_idx]);
  btree *right_block = freelist_get_node(&free_blocks[queue_idx]);
  pq_node *n = b->nodes_list;
  pq_node *splitter = NULL;

  if ( new_node->key < n->key ) {
    left_block->nodes_list = new_node;
  } else {
    left_block->nodes_list = n;
  }

  splitter = split(n, new_node, (b->node_count+1)/2);
  if ( splitter->key < new_node->key ) { /* still have to place new_node */
    pq_node *t = split(splitter, new_node, b->node_count - (b->node_count+1)/2);
    if ( t->key < new_node->key ) {
      insert_helper(t, new_node); /* still! have to place new_node */
    }
  }

  right_block->nodes_list = _Chain_Next(splitter);

}

static Chain_Control queues[NUM_QUEUES];
static Freelist_Control free_nodes[NUM_QUEUES];
static Freelist_Control free_blocks[NUM_QUEUES];
static btree_block *btree_root[NUM_QUEUES];

static inline
void sparc64_print_all_queues()
{
  Chain_Node *iter;
  Chain_Control *spill_pq;
  int i,k;
  for ( k = 0; k < NUM_QUEUES; k++ ) {
    spill_pq = &queues[k];
    iter = _Chain_First(spill_pq);
    i = 0;
    while(iter && !_Chain_Is_tail(spill_pq,iter)) {
      pq_node *p = (pq_node*)iter;
      printk("%d\tChain: %d\t%X\t%d\t%x\n", k, i, p,p->key,p->val);
      i++;
      iter = _Chain_Next(iter);
    }
  }
}

uint64_t sparc64_unitedlistbtree_initialize( int qid, size_t max_pq_size )
{
  int i;
  uint64_t reg = 0;
  freelist_initialize(&free_nodes[qid], sizeof(pq_node), max_pq_size);

  // the maximum number of blocks is actually the log of max_pq_size...
  freelist_initialize(&free_blocks[qid], sizeof(btree_block), max_pq_size);

  _Chain_Initialize_empty(&queues[qid]);
  spillpq_queue_max_size[qid] = max_pq_size;
  btree_root[qid] = freelist_get_node(&free_blocks[qid]);
  btree_root[qid].node_count = 0;
  btree_root[qid].nodes_list = _Chain_Head(&queues[qid]);
  btree_root[qid].leaf = true;
  DPRINTK("%d\tSize: %ld\n", qid, reg);
  return 0;
}

uint64_t sparc64_unitedlistbtree_insert(int queue_idx, uint64_t kv)
{
  Chain_Node *iter;
  pq_node *new_node;
  int i;

  btree_block *b;
  btree_block *root = btree_root[queue_idx];

  new_node = freelist_get_node(&free_nodes[queue_idx]);
  if (!new_node) {
    // debug output
    sparc64_print_all_queues();
    printk("%d\tUnable to allocate new node during insert\n", queue_idx);
    while (1);
  }
  new_node->key = kv_key(kv);
  new_node->val = kv_value(kv); // FIXME: not full 64-bits

  b = btree_find_block(root, new_node->key);

  if (btree_block_is_full(b)) {
    btree_split(b, new_node);
  } else {
    pq_node *next = btree_node_in_block(b, node->key);
    Chain_Node *prev = next;
    if ( !_Chain_Is_empty(spill_pq) ) {
      if ( !_Chain_Is_first((_Chain_Node*)next) ) {
        prev = _Chain_Previous(prev);
      }
    }

    if ( leaf->nodes_list == next ) {
      leaf->nodes_list = node;
    }

    _Chain_Insert_unprotected(prev, (Chain_Node*)node);
  }

  return 0;
}

uint64_t sparc64_unitedlistbtree_first(int queue_idx, uint64_t kv)
{
  Chain_Node *first;
  Chain_Control *spill_pq;
  pq_node *p;
  spill_pq = &queues[queue_idx];
  first = _Chain_First(spill_pq);
  if ( _Chain_Is_tail(spill_pq, first) ) {
    return (uint64_t)-1;
  }
  p = (pq_node*)first;

  return pq_node_to_kv(p);
}

uint64_t sparc64_unitedlistbtree_pop(int queue_idx, uint64_t kv)
{
  Chain_Node *first;
  Chain_Control *spill_pq;
  uint64_t rv;
  spill_pq = &queues[queue_idx];


  first = _Chain_Get_unprotected(spill_pq);
  if (first) {
    rv = pq_node_to_kv((pq_node*)first);
  } else {
    rv = (uint64_t)-1;
  }
  return rv;
}


uint64_t sparc64_unitedlistbtree_extract(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  uint32_t key = kv_key(kv);
  Chain_Control *spill_pq = &queues[queue_idx];
  pq_node *node = search_helper(queue_idx, kv);

  if ( !_Chain_Is_tail(spill_pq, (Chain_Node*)node) && node->key == key ) {
    if ( _Chain_Is_first((Chain_Node*)node) ) {
      _Chain_Get_first_unprotected(spill_pq);
    } else {
      _Chain_Extract_unprotected((Chain_Node*)node);
    }
    rv = pq_node_to_kv(node);
  } else {
    DPRINTK("%d\tFailed software extract: %d\t%X\n",queue_idx, key,val);
    rv = (uint64_t)-1;
  }
  return rv;
}

uint64_t sparc64_unitedlistbtree_search(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  uint32_t key = kv_key(kv);
  Chain_Control *spill_pq = &queues[queue_idx];
  pq_node *node = search_helper(queue_idx, kv);

  if ( !_Chain_Is_tail(spill_pq, (Chain_Node*)node) && node->key == key ) {
    rv = pq_node_to_kv(node);
  } else {
    DPRINTK("%d\tFailed search: %d\t%X\n",queue_idx, kv_key(kv), kv_value(kv));
    rv = (uint64_t)-1;
  }

  return rv;
}

// Pass iter node as either the tail of spill_pq or as a node that is known
// to have lower priority than the lowest priority node in the hwpq; this is
// simple when there is not concurrent access to a pq in the hwpq.
static inline 
Chain_Node* sparc64_unitedlistbtree_spill_node(
    int queue_idx,
    Chain_Control *spill_pq,
    Chain_Node *iter
)
{
  uint64_t kv;
  uint32_t key, val;
  pq_node *new_node;
  int i;

  //Chain_Node *iter;
  //iter = _Chain_Last(spill_pq);

  HWDS_SPILL(queue_idx, kv);
  if (!kv) {
    DPRINTK("%d\tNothing to spill!\n",queue_idx);
    return 0;
  }
  key = kv_key(kv);
  val = kv_value(kv);

  DPRINTK("%d\tspill: node: %x\tprio: %d\n",queue_idx,val,key);

  // sort by ascending priority
  // Note that this is not a stable sort (globally) because it has
  // FIFO behavior for spilled nodes with equal keys. To get global stability
  // we would need to ensure that (1) the key comparison is <= and (2) the
  // last node to be spilled has no ties left in the hwpq.
  while (!_Chain_Is_head(spill_pq, iter) && key < ((pq_node*)iter)->key) 
    iter = _Chain_Previous(iter);

  new_node = freelist_get_node(&free_nodes[queue_idx]);
  if (!new_node) {
    // debug output
    sparc64_print_all_queues();
    printk("%d\tUnable to allocate new node while spilling\n", queue_idx);
    while (1);
  }

  // key > iter->key, insert new node after iter
  new_node->key = key;
  new_node->val = val; // FIXME: not full 64-bits
  _Chain_Insert_unprotected(iter, (Chain_Node*)new_node);
  return new_node;
}


uint64_t sparc64_unitedlistbtree_handle_spill( int queue_idx, uint64_t count )
{
  int i = 0;
  Chain_Control *spill_pq;
  Chain_Node *iter;

  spill_pq = &queues[queue_idx];
  DPRINTK("spill: queue: %d\n", queue_idx);

  iter = _Chain_Last(spill_pq);
  // pop elements off tail of hwpq, merge into software pq
  while ( i < count ) {
    if (!(iter = sparc64_unitedlistbtree_spill_node(queue_idx, spill_pq, iter)))
      break;
    i++;
  }
  return i;
}

static inline uint64_t
sparc64_unitedlistbtree_fill_node(
    int queue_idx,
    Chain_Control *spill_pq,
    uint64_t count
) {
  uint32_t exception;
  pq_node *p;

  p = (pq_node*)_Chain_Get_first_unprotected(spill_pq);

  DPRINTK("%d\tfill node: %x\tprio: %d\n", queue_idx, p->val, p->key);

  // add node to hw pq 
  HWDS_FILL(queue_idx, p->key, p->val, exception); 

  freelist_put_node(&free_nodes[queue_idx], p);

  if (exception) {
    DPRINTK("%d\tSpilling while filling\n", queue_idx);
    return sparc64_unitedlistbtree_handle_spill(queue_idx, count);
  }
  return 0;
}

/*
 * Current algorithm pulls nodes from the head of the sorted sw pq
 * and fills them into the hw pq.
 */
uint64_t sparc64_unitedlistbtree_handle_fill(int queue_idx, uint64_t count)
{
 Chain_Control *spill_pq;
 int            i = 0;

 spill_pq = &queues[queue_idx];
 DPRINTK("fill: queue: %d\n", queue_idx);

  while (!_Chain_Is_empty(spill_pq) && i < count) {
    i++;
    sparc64_unitedlistbtree_fill_node(queue_idx, spill_pq, count);
  }
  return 0;
}

uint64_t sparc64_unitedlistbtree_drain( int qid, uint64_t ignored )
{

  DPRINTK("%d\tdrain queue unimplemented\n", qid);

  return 0;
}

sparc64_spillpq_operations sparc64_unitedlistbtree_ops = {
  sparc64_unitedlistbtree_initialize,
  sparc64_unitedlistbtree_insert,
  sparc64_unitedlistbtree_first,
  sparc64_unitedlistbtree_pop,
  sparc64_unitedlistbtree_extract,
  sparc64_unitedlistbtree_search,
  sparc64_unitedlistbtree_handle_spill,
  sparc64_unitedlistbtree_handle_fill,
  sparc64_unitedlistbtree_drain
};

