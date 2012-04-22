#include "spillpq.h"
#include "freelist.h"
#include "gabdebug.h"

static Chain_Control queues[NUM_QUEUES];
static Freelist_Control free_nodes[NUM_QUEUES];

typedef struct {
  Chain_Node Node;
  uint32_t key;
  uint32_t val;
} pq_node;

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

uint64_t sparc64_unitedlistpq_initialize( int qid, size_t max_pq_size )
{
  int i;
  uint64_t reg = 0;
  freelist_initialize(&free_nodes[qid], sizeof(pq_node), max_pq_size);

  _Chain_Initialize_empty(&queues[qid]);
  spillpq_queue_max_size[qid] = max_pq_size;
  DPRINTK("%d\tSize: %ld\n", qid, reg);
  return 0;
}

uint64_t sparc64_unitedlistpq_insert(int queue_idx, uint64_t kv)
{
  uint32_t key;
  uint32_t val;
  Chain_Node *iter;
  Chain_Control *spill_pq;
  pq_node *new_node;

  key = kv_key(kv);
  val = kv_value(kv);

  // linear search, ugh
  spill_pq = &queues[queue_idx];
  iter = _Chain_First(spill_pq);
  while(!_Chain_Is_tail(spill_pq,iter)) {
    pq_node *p = (pq_node*)iter;
    if (p->key > key) {
      break;
    }
    iter = _Chain_Next(iter);
  }
  if ( !_Chain_Is_first(iter) )
    iter = _Chain_Previous(iter);

  new_node = freelist_get_node(&free_nodes[queue_idx]);
  if (!new_node) {
    // debug output
    sparc64_print_all_queues();
    printk("%d\tUnable to allocate new node during insert\n", queue_idx);
    while (1);
  }

  // key > iter->key, insert new node after iter
  new_node->key = key;
  new_node->val = val; // FIXME: not full 64-bits
  _Chain_Insert_unprotected(iter, (Chain_Node*)new_node);
  return 0;
}

uint64_t sparc64_unitedlistpq_first(int queue_idx, uint64_t kv)
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

uint64_t sparc64_unitedlistpq_pop(int queue_idx, uint64_t kv)
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

static pq_node* search_helper(int queue_idx, uint64_t kv)
{
  uint32_t key;
  Chain_Node *iter;
  Chain_Control *spill_pq;
  
  key = kv_key(kv);
  
  // linear search, ugh
  spill_pq = &queues[queue_idx];
  iter = _Chain_First(spill_pq);
  while(!_Chain_Is_tail(spill_pq,iter)) {
    pq_node *p = (pq_node*)iter;
    if (p->key == key) {
      return p;
    }
    iter = _Chain_Next(iter);
  }
  return NULL;
}

uint64_t sparc64_unitedlistpq_extract(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  pq_node *node = search_helper(queue_idx, kv);
  Chain_Control *spill_pq = &queues[queue_idx];

  node = search_helper(queue_idx, kv);

  if ( node ) {
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

uint64_t sparc64_unitedlistpq_search(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  pq_node *node = search_helper(queue_idx, kv);

  if (node) {
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
Chain_Node* sparc64_unitedlistpq_spill_node(
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


uint64_t sparc64_unitedlistpq_handle_spill( int queue_idx, uint64_t count )
{
  int i = 0;
  Chain_Control *spill_pq;
  Chain_Node *iter;

  spill_pq = &queues[queue_idx];
  DPRINTK("spill: queue: %d\n", queue_idx);

  iter = _Chain_Last(spill_pq);
  // pop elements off tail of hwpq, merge into software pq
  while ( i < count ) {
    if (!(iter = sparc64_unitedlistpq_spill_node(queue_idx, spill_pq, iter)))
      break;
    i++;
  }
  return i;
}

static inline uint64_t
sparc64_unitedlistpq_fill_node(
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
    return sparc64_unitedlistpq_handle_spill(queue_idx, count);
  }
  return 0;
}

/*
 * Current algorithm pulls nodes from the head of the sorted sw pq
 * and fills them into the hw pq.
 */
uint64_t sparc64_unitedlistpq_handle_fill(int queue_idx, uint64_t count)
{
 Chain_Control *spill_pq;
 int            i = 0;

 spill_pq = &queues[queue_idx];
 DPRINTK("fill: queue: %d\n", queue_idx);

  while (!_Chain_Is_empty(spill_pq) && i < count) {
    i++;
    sparc64_unitedlistpq_fill_node(queue_idx, spill_pq, count);
  }
  return 0;
}

uint64_t sparc64_unitedlistpq_drain( int qid, uint64_t ignored )
{
  Chain_Node *tmp;
  Chain_Control *spill_pq;
  pq_node *p;

  DPRINTK("%d\tdrain queue\n", qid);
  spill_pq = &queues[qid];

  return 0;
}

sparc64_spillpq_operations sparc64_unitedlistpq_ops = {
  sparc64_unitedlistpq_initialize,
  sparc64_unitedlistpq_insert,
  sparc64_unitedlistpq_first,
  sparc64_spillpq_null_handler,
  sparc64_unitedlistpq_extract,
  sparc64_unitedlistpq_search,
  sparc64_unitedlistpq_handle_spill,
  sparc64_unitedlistpq_handle_fill,
  sparc64_unitedlistpq_drain
};

