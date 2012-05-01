
#include "mskiplist.h"

#include <stdlib.h>
#include <tmacros.h>
#include <rtems/chain.h>
#include "../shared/params.h"

/* data structure */
static skiplist the_skiplist[NUM_APERIODIC_TASKS];
/* data */
static node* the_nodes[NUM_APERIODIC_TASKS];
/* free storage */
rtems_chain_control freenodes[NUM_APERIODIC_TASKS];

/* helpers */
static node *alloc_node(rtems_task_argument tid) {
  node *n = rtems_chain_get_unprotected( &freenodes[tid] );
  return n;
}
static void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freenodes[tid], n );
}

static inline unsigned long seed() {
  return 0xdeadbeefUL; // FIXME: randomize
}
static void initialize_helper(rtems_task_argument tid, int size) {
  int i;
  skiplist *sl = &the_skiplist[tid];

  for ( i = 0; i < MAXLEVEL; i++ ) {
    rtems_chain_initialize_empty ( &sl->lists[i] );
  }
  sl->level = 0; /* start at the bottom */

  srand(seed());
}

static inline int next_random_bit() {
  static uint32_t random_stream = 0;
  int rv;

  if ( random_stream == 0 ) {
    random_stream = rand();
  }
  rv = random_stream & 1;
  random_stream >>= 1;
  return rv;
}

static inline int randomLevel()
{
  int level = 0;
  while (next_random_bit() && level < MAXLEVEL-1) // FIXME: hard-coded p
    level++;
  return level;
}

/* implements skip list insert according to pugh */
static void insert_helper(rtems_task_argument tid, node *new_node)
{
  rtems_chain_node *x;
  rtems_chain_node *x_forward;
  node *x_node;
  rtems_chain_control *list;
  skiplist *sl = &the_skiplist[tid];  /* list */
  int upper_level = sl->level;        /* list->level */
  int new_level = 0;
  int key = new_node->data.key;       /* searchKey */
  int i;
  rtems_chain_node *update[MAXLEVEL];

  list = &sl->lists[upper_level]; /* top */
  x = rtems_chain_head(list); /* left */
  // search left-right top-bottom
  for ( i = upper_level; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_forward = rtems_chain_next(x);
    /* Find the rightmost node of level i that is left of the insert point */
    while (!rtems_chain_is_tail(list, x) &&
           !rtems_chain_is_tail(list, x_forward) &&
           LINK_TO_NODE(x_forward, i)->data.key < key) {
      x = x_forward;
      x_forward = rtems_chain_next(x);
    }
    update[i] = x;

    /* move down to next level if it exists */
    if ( i ) {
      if ( !rtems_chain_is_head(list, x)) {
        x_node = LINK_TO_NODE(x, i);
        x = &(x_node->link[i-1]);
      } else {
        x = rtems_chain_head(&sl->lists[i-1]);
      }
    }
  }

  //assert(list == &sl->lists[0]);
  new_level = randomLevel();
  if ( new_level > upper_level ) {
    for (i = upper_level + 1; i <= new_level; i++) {
      list = &sl->lists[i];
      update[i] = rtems_chain_head(list);
    }
    sl->level = new_level;
  }
  for ( i = 0; i <= new_level; i++ ) {
    rtems_chain_insert_unprotected(update[i], &new_node->link[i]);
  }
}

/* Returns node with same key, first key greater, or tail of list */
static node* search_helper(rtems_task_argument tid, int key)
{
  rtems_chain_node *x;
  rtems_chain_node *x_forward;
  node *x_node;
  rtems_chain_control *list;
  skiplist *sl = &the_skiplist[tid];  /* list */
  int upper_level = sl->level;        /* list->level */
  int i;

  list = &sl->lists[upper_level]; /* top */
  x = rtems_chain_head(list); /* left */
  // search left-right top-bottom
  for ( i = upper_level; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_forward = rtems_chain_next(x);
    /* Find the rightmost node of level i that is left of the insert point */
    while (!rtems_chain_is_tail(list, x) &&
           !rtems_chain_is_tail(list, x_forward) &&
           LINK_TO_NODE(x_forward, i)->data.key < key) {
      x = x_forward;
      x_forward = rtems_chain_next(x);
    }

    /* move down to next level if it exists */
    if ( i ) {
      if ( !rtems_chain_is_head(list, x)) {
        x_node = LINK_TO_NODE(x, i);
        x = &(x_node->link[i-1]);
      } else {
        x = rtems_chain_head(&sl->lists[i-1]);
      }
    }
  }

  x = x_forward;
  return LINK_TO_NODE(x, 0);
}

static inline long extract_helper(rtems_task_argument tid, int key)
{
  // TODO: perhaps mark node for deletion and deal with later.
  rtems_chain_node *x;
  rtems_chain_node *x_forward;
  node *x_node;
  rtems_chain_control *list;
  skiplist *sl = &the_skiplist[tid];  /* list */
  int upper_level = sl->level;        /* list->level */
  int i;
  long kv;
  rtems_chain_node *update[MAXLEVEL];

  list = &sl->lists[upper_level]; /* top */
  x = rtems_chain_head(list); /* left */
  // search left-right top-bottom
  for ( i = upper_level; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_forward = rtems_chain_next(x);
    /* Find the rightmost node of level i that is left of the insert point */
    while (!rtems_chain_is_tail(list, x) &&
           !rtems_chain_is_tail(list, x_forward) &&
           LINK_TO_NODE(x_forward, i)->data.key < key) {
      x = x_forward;
      x_forward = rtems_chain_next(x);
    }
    update[i] = x;

    /* move down to next level if it exists */
    if ( i ) {
      if ( !rtems_chain_is_head(list, x)) {
        x_node = LINK_TO_NODE(x, i);
        x = &(x_node->link[i-1]);
      } else {
        x = rtems_chain_head(&sl->lists[i-1]);
      }
    }
  }

  x = x_forward;
  if ( !rtems_chain_is_tail(&sl->lists[0], x) ) {
    x_node = LINK_TO_NODE(x, 0);
    if ( x_node->data.key == key ) {
      kv = PQ_NODE_TO_KV(&x_node->data);
      for ( i = 0; i <= upper_level; i++ ) {
        if ( rtems_chain_next(update[i]) != &x_node->link[i] )
          break;
        rtems_chain_extract_unprotected(&x_node->link[i]);
      }
      for ( i = upper_level; i > 0; i-- ) {
        if ( rtems_chain_is_empty(&sl->lists[i]) ) {
          sl->level--;
        }
      }
      free_node(tid, x_node);
    } else {
      kv = (long)-1;
    }
  } else {
    kv = (long)-1;
  }

  return kv;
}

/**
 * benchmark interface
 */
void skiplist_initialize( rtems_task_argument tid, int size ) {
  int i;

  the_nodes[tid] = (node*)malloc(sizeof(node)*size);
  if ( ! the_nodes[tid] ) {
    printk("failed to alloc nodes\n");
    while(1);
  }

  rtems_chain_initialize(&freenodes[tid], the_nodes[tid], size, sizeof(node));

  initialize_helper(tid, size);
}

void skiplist_insert(rtems_task_argument tid, long kv ) {
  node *new_node = alloc_node(tid);
  new_node->data.key = kv_key(kv);
  new_node->data.val = kv_value(kv);
  insert_helper(tid, new_node);
}

long skiplist_min( rtems_task_argument tid ) {
  node *n;

  n = rtems_chain_first(&the_skiplist[tid].lists[0]); // unprotected
  if (n) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (long)-1;
}

long skiplist_pop_min( rtems_task_argument tid ) {
  long kv;
  node *n;
  n = rtems_chain_first(&the_skiplist[tid].lists[0]); // unprotected
  kv = extract_helper(tid, n->data.key);
  return kv;
}

long skiplist_search( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
  if(!rtems_chain_is_tail(&the_skiplist[tid].lists[0], n) && n->data.key == k) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (long)-1;
}

long skiplist_extract( rtems_task_argument tid, int k ) {
  long kv = extract_helper(tid, k);
  return kv;
}

