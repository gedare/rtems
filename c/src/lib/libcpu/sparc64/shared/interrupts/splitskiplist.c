#include "spillpq.h"
#include "freelist.h"
#include "gabdebug.h"

#define MAXLEVEL (16)

typedef struct {
  Chain_Node link[MAXLEVEL];
  uint32_t height;
  uint32_t key;
  uint32_t val;
} pq_node;

typedef struct {
  Chain_Control *lists;

  int level;
} skiplist;

// container-of magic
#define LINK_TO_NODE(cn, index) \
  ((pq_node*)((uintptr_t)cn - ((uintptr_t)(&(((pq_node *)0)->link[index])))))

#define PQ_NODE_TO_KV(n) ((((long)(n)->key) << (sizeof(long)*4L)) | (long)(n)->val)


static skiplist the_skiplist[NUM_QUEUES];
static Freelist_Control free_nodes[NUM_QUEUES];

static void print_skiplist_node(Chain_Node *n, int index)
{
  printk("%d-", LINK_TO_NODE(n, index)->key);
  return;
}

static void print_skiplist_list(
    Chain_Control *list,
    int index,
    Chain_Control *all
)
{
  Chain_Node *n;
  Chain_Node *iter;
 
  printk("%d::-", index);
  if ( _Chain_Is_empty(list) ) {
    return;
  }

  n = _Chain_First(list);
  iter = _Chain_First(all);
  while ( !_Chain_Is_tail(list, n) ) {
    while ( LINK_TO_NODE(n,index) != LINK_TO_NODE(iter,0) ) {
      iter = _Chain_Next(iter);
      printk("xxxx-");
    }
    print_skiplist_node(n, index);
    n = _Chain_Next(n);
    iter = _Chain_Next(iter);
  }
  printk("x\n");
}

static void print_skiplist( skiplist *sl ) {
  int i;

  for ( i = sl->level; i >= 0; i-- ) {
    print_skiplist_list(&sl->lists[i], i, &sl->lists[0]);
  }
  printk("\n");
}

static void sparc64_print_all_queues()
{
  int k;
  skiplist *sl;
  for ( k = 0; k < NUM_QUEUES; k++ ) {
    sl = &the_skiplist[k];
    print_skiplist(sl);
  }
}

static inline unsigned long seed(void) {
  return 0xdeadbeefUL; // FIXME: randomize
}

static inline int next_random_bit(void) {
  static uint32_t random_stream = 0;
  int rv;

  if ( random_stream == 0 ) {
    random_stream = rand();
  }
  rv = random_stream & 1;
  random_stream >>= 1;
  return rv;
}

static inline int randomLevel(void)
{
  int level = 0;
  while (next_random_bit() && level < MAXLEVEL-1) // FIXME: hard-coded p
    level++;
  return level;
}

/* implements skip list insert according to pugh */
static void insert_helper(int qid, pq_node *new_node)
{
  Chain_Node *x;
  Chain_Node *x_forward;
  pq_node *x_node;
  Chain_Control *list;
  skiplist *sl = &the_skiplist[qid];  /* list */
  int upper_level = sl->level;        /* list->level */
  int new_level = 0;
  int key = new_node->key;       /* searchKey */
  int i;
  Chain_Node *update[MAXLEVEL];

  list = &sl->lists[upper_level]; /* top */
  x = _Chain_Head(list); /* left */
  // search left-right top-bottom
  for ( i = upper_level; i >= 0; i-- ) {
    list = &sl->lists[i];
    if ( _Chain_Is_tail(list, x) ) {
      x_forward = x;
    } else {
      x_forward = _Chain_Next(x);
    }
    /* Find the rightmost node of level i that is left of the insert point */
    while ( !_Chain_Is_tail(list, x_forward) &&
            LINK_TO_NODE(x_forward, i)->key < key ) {
      x = x_forward;
      x_forward = _Chain_Next(x);
    }
    update[i] = x;

    /* move down to next level if it exists */
    if ( i ) {
      if ( !_Chain_Is_head(list, x)) {
        x_node = LINK_TO_NODE(x, i);
        x = &(x_node->link[i-1]);
      } else {
        x = _Chain_Head(&sl->lists[i-1]);
      }
    }
  }

  //assert(list == &sl->lists[0]);
  new_level = new_node->height;
  if ( new_level > upper_level ) {
    for (i = upper_level + 1; i <= new_level; i++) {
      list = &sl->lists[i];
      update[i] = _Chain_Head(list);
    }
    sl->level = new_level;
  }
  for ( i = 0; i <= new_level; i++ ) {
    _Chain_Insert_unprotected(update[i], &new_node->link[i]);
  }
  //skiplist_verify(sl, 1, 4);
  //print_skiplist(sl);
}

/* Returns node with same key, first key greater, or tail of list */
static pq_node* search_helper(int qid, int key)
{
  Chain_Node *x;
  Chain_Node *x_forward;
  pq_node *x_node;
  Chain_Control *list;
  skiplist *sl = &the_skiplist[qid];  /* list */
  int upper_level = sl->level;        /* list->level */
  int i;

  list = &sl->lists[upper_level]; /* top */
  x = _Chain_Head(list); /* left */
  // search left-right top-bottom
  for ( i = upper_level; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_forward = _Chain_Next(x);
    /* Find the rightmost node of level i that is left of the insert point */
    while (!_Chain_Is_tail(list, x) &&
           !_Chain_Is_tail(list, x_forward) &&
           LINK_TO_NODE(x_forward, i)->key < key) {
      x = x_forward;
      x_forward = _Chain_Next(x);
    }

    /* move down to next level if it exists */
    if ( i ) {
      if ( !_Chain_Is_head(list, x)) {
        x_node = LINK_TO_NODE(x, i);
        x = &(x_node->link[i-1]);
      } else {
        x = _Chain_Head(&sl->lists[i-1]);
      }
    }
  }

  x = x_forward;
  return LINK_TO_NODE(x, 0);
}

static inline long extract_helper(int qid, int key)
{
  // TODO: perhaps mark node for deletion and deal with later.
  Chain_Node *x;
  Chain_Node *x_forward;
  pq_node *x_node;
  Chain_Control *list;
  skiplist *sl = &the_skiplist[qid];  /* list */
  int upper_level = sl->level;        /* list->level */
  int i;
  long kv;
  Chain_Node *update[MAXLEVEL];

  list = &sl->lists[upper_level]; /* top */
  x = _Chain_Head(list); /* left */
  // search left-right top-bottom
  for ( i = upper_level; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_forward = _Chain_Next(x);
    /* Find the rightmost node of level i that is left of the insert point */
    while (!_Chain_Is_tail(list, x) &&
           !_Chain_Is_tail(list, x_forward) &&
           LINK_TO_NODE(x_forward, i)->key < key) {
      x = x_forward;
      x_forward = _Chain_Next(x);
    }
    update[i] = x;

    /* move down to next level if it exists */
    if ( i ) {
      if ( !_Chain_Is_head(list, x)) {
        x_node = LINK_TO_NODE(x, i);
        x = &(x_node->link[i-1]);
      } else {
        x = _Chain_Head(&sl->lists[i-1]);
      }
    }
  }

  x = x_forward;
  if ( !_Chain_Is_tail(&sl->lists[0], x) ) {
    x_node = LINK_TO_NODE(x, 0);
    if ( x_node->key == key ) {
      kv = PQ_NODE_TO_KV(x_node);
      for ( i = 0; i <= upper_level; i++ ) {
        if ( _Chain_Next(update[i]) != &x_node->link[i] )
          break;
        _Chain_Extract_unprotected(&x_node->link[i]);
      }
      for ( i = upper_level; i > 0; i-- ) {
        if ( _Chain_Is_empty(&sl->lists[i]) ) {
          sl->level--;
        }
      }
      freelist_put_node(&free_nodes[qid], x_node);
    } else {
      kv = (long)-1;
    }
  } else {
    kv = (long)-1;
  }

  return kv;
}

uint64_t sparc64_splitskiplist_initialize( int qid, size_t max_pq_size )
{
  int i;
  uint64_t reg = 0;
  skiplist *sl = &the_skiplist[qid];
  pq_node *pnode;
  Chain_Node *n;

  freelist_initialize(&free_nodes[qid], sizeof(pq_node), max_pq_size, NULL);

  // FIXME: MAXLEVEL
  sl->lists = malloc(sizeof(Chain_Control)*(MAXLEVEL+1));
  if ( !sl->lists ) {
    printk("Failed to allocate list headers\n");
    while(1);
  }
  for ( i = 0; i <= MAXLEVEL; i++ ) {
    _Chain_Initialize_empty ( &sl->lists[i] );
  }
  sl->level = 0; /* start at the bottom */

  srand(seed());

  // precompute node heights
  // FIXME: add initializer callout to freelist..
  n = _Chain_First(&free_nodes[qid].freelist);
  while (!_Chain_Is_tail(&free_nodes[qid].freelist, n)) {
    pnode = (pq_node*)n;
    pnode->height = randomLevel();
    n = _Chain_Next(n);
  }

  spillpq[qid].max_size = max_pq_size;
  DPRINTK("%d\tSize: %ld\n", qid, reg);
  return 0;
}

uint64_t sparc64_splitskiplist_insert(int qid, uint64_t kv)
{
  pq_node *new_node;
  new_node = freelist_get_node(&free_nodes[qid]);
  if (!new_node) {
    printk("%d\tUnable to allocate new node during insert\n", qid);
    while (1);
  }
  new_node->key = kv_key(kv);
  new_node->val = kv_value(kv); // FIXME: not full 64-bits

  insert_helper(qid, new_node);
  return 0;
}

uint64_t sparc64_splitskiplist_first(int qid, uint64_t kv)
{
  pq_node *p;
  Chain_Control *spill_pq;
  Chain_Node *first;
  spill_pq = &the_skiplist[qid].lists[0];
  
  first = _Chain_First(spill_pq);
  if ( !_Chain_Is_tail(spill_pq, first) ) {
    p = LINK_TO_NODE(first, 0);
    kv = pq_node_to_kv(p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t sparc64_splitskiplist_pop(int qid, uint64_t kv)
{
  pq_node *p;
  Chain_Control *spill_pq;
  Chain_Node *first;
  int i;
  spill_pq = &the_skiplist[qid].lists[0];
  
  first = _Chain_First(spill_pq);

  if ( !_Chain_Is_tail(spill_pq, first) ) {
    p = LINK_TO_NODE(first, 0);
    kv = pq_node_to_kv(p);
    for ( i = 0; i <= p->height; i++ ) {
      _Chain_Extract_unprotected(&p->link[i]);
    }
    freelist_put_node(&free_nodes[qid], p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t sparc64_splitskiplist_extract(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  uint32_t key = kv_key(kv);
  
  rv = (uint64_t)extract_helper(queue_idx, key);
  return rv;
}

uint64_t sparc64_splitskiplist_search(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  uint32_t key = kv_key(kv);
  Chain_Control *spill_pq = &the_skiplist[queue_idx].lists[0];
  pq_node *node = search_helper(queue_idx, key);

  if ( !_Chain_Is_tail(spill_pq, (Chain_Node*)node) && node->key == key) {
    rv = pq_node_to_kv(node);
  } else {
    DPRINTK("%d\tFailed search: %d\t%X\n",queue_idx, kv_key(kv), kv_value(kv));
    rv = (uint64_t)-1;
  }

  return rv;
}

static inline 
Chain_Node* sparc64_splitskiplist_spill_node(
    int queue_idx
)
{
  uint64_t kv;

  HWDS_SPILL(queue_idx, spillpq[queue_idx].policy.spill_from, kv);
  if (!kv) {
    DPRINTK("%d\tNothing to spill!\n",queue_idx);
  } else {
    sparc64_splitskiplist_insert(queue_idx, kv);
  }

  return kv;
}

uint64_t sparc64_splitskiplist_handle_spill( int queue_idx, uint64_t count )
{
  int i = 0;

  // pop elements off tail of hwpq, merge into software pq
  while ( i < count ) {
    if (!sparc64_splitskiplist_spill_node(queue_idx))
      break;
    i++;
  }
  return i;
}

static inline uint64_t
sparc64_splitskiplist_fill_node(
    int queue_idx,
    int count
) {
  uint32_t exception;
  pq_node *p;
  uint64_t kv;

  kv = sparc64_splitskiplist_pop(queue_idx, 0);

  // add node to hw pq 
  HWDS_FILL(queue_idx, kv_key(kv), kv_value(kv), exception); 

  if (exception) {
    DPRINTK("%d\tSpilling while filling\n", queue_idx);
    return sparc64_splitskiplist_handle_spill(queue_idx, count);
  }
  return 0;
}

/*
 * Current algorithm pulls nodes from the head of the sorted sw pq
 * and fills them into the hw pq.
 */
uint64_t sparc64_splitskiplist_handle_fill(int queue_idx, uint64_t count)
{
 Chain_Control *spill_pq;
 int            i = 0;

 spill_pq = &the_skiplist[queue_idx].lists[0];
 DPRINTK("fill: queue: %d\n", queue_idx);

  while (!_Chain_Is_empty(spill_pq) && i < count) {
    i++;
    sparc64_splitskiplist_fill_node(queue_idx, count);
  }
  return 0;
}

uint64_t sparc64_splitskiplist_drain( int qid, uint64_t ignored )
{

  DPRINTK("%d\tdrain queue unimplemented\n", qid);

  return 0;
}

sparc64_spillpq_operations sparc64_splitskiplist_ops = {
  sparc64_splitskiplist_initialize,
  sparc64_spillpq_null_handler,
  sparc64_spillpq_null_handler,
  sparc64_spillpq_null_handler,
  sparc64_splitskiplist_extract,
  sparc64_splitskiplist_search,
  sparc64_splitskiplist_handle_spill,
  sparc64_splitskiplist_handle_fill,
  sparc64_splitskiplist_drain
};

