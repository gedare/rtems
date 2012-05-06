
#include "mdetskiplist.h"

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

/*
 * a deterministic 1-2 skiplist enforces a property that at most two nodes 
 * exist at level i-1 between two nodes at level i. this debug helper routine
 * verifies the 1-2 property.
 */
static bool skiplist_verify(skiplist *sl, int min, int max) {
  rtems_chain_node *x;
  rtems_chain_node *x_forward;
  rtems_chain_node *x_forward_down;
  rtems_chain_node *x_down;
  rtems_chain_node *iter;
  node *x_forward_node;
  node *x_node;
  rtems_chain_control *list;
  int count;
  int block;
  int i;
  bool rv = true;
  
  // handle the top level as a special case. first deal with above top
  list = &sl->lists[sl->level + 1];
  if ( !rtems_chain_is_empty( list ) ) {
    printk("level %d above top is non-empty\n", sl->level + 1);
    rv = false;
  }
  // then deal with top
  list = &sl->lists[sl->level];
  if ( rtems_chain_is_empty( list ) ) {
    if (sl->level) {
      printk("top is empty\n");
      rv = false;
    } else {
      printk("empty list\n");
      return rv; // empty list
    }
  }
  x = rtems_chain_first(list);
  count = 0;
  while ( !rtems_chain_is_tail( list, x ) ) {
    x = rtems_chain_next(x);
    count++;
  }
  if ( count < min || count > max ) {
    printk("%d nodes in [top] level %d\n", count, sl->level);
    rv = false; // this should be allowed, since skiplist could saturate.
  }

  // scan left-right top-bottom excluding the highest and lowest levels
  for ( i = sl->level; i > 0; i-- ) {
    list = &sl->lists[i];
    if ( rtems_chain_is_empty(list) ) {
      printk("level %d is empty\n", i);
      rv = false;
    }
    x = rtems_chain_first(list);
    x_forward = rtems_chain_next(x);
    block = 0;

    // deal with nodes at start of list
    x_node = LINK_TO_NODE(x, i);
    x_down = &x_node->link[i-1];
    count = 0;
    iter = rtems_chain_first(&sl->lists[i-1]);
    while ( iter != x_down ) {
      count++;
      iter = rtems_chain_next(iter);
    }
    if ( count < min || count > max ) {
      printk("%d nodes in level %d at head block %d\n",
          count, i-1, block);
      rv = false;
    }
    block++;

    while ( !rtems_chain_is_tail(list, x_forward) ) {
      x_forward_node = LINK_TO_NODE(x_forward, i);
      x_forward_down = &(x_forward_node->link[i-1]);
      x_node = LINK_TO_NODE(x, i);
      x_down = &(x_node->link[i-1]);

      /* count nodes between x and x_forward in next level down */
      iter = rtems_chain_next(x_down);
      count = 0;
      while ( iter != x_forward_down ) {
        iter = rtems_chain_next(iter);
        count++;
      }
      if ( count < min || count > max ) {
        printk("%d nodes in level %d at block %d\n",
            count, i-1, block);
        rv = false;
      }

      x = x_forward;
      x_forward = rtems_chain_next(x);
      block++;
    }

    // deal with nodes at end of list
    x_node = LINK_TO_NODE(x, i);
    x_down = &x_node->link[i-1];
    count = 0;
    iter = x_down;
    while ( !rtems_chain_is_last(iter) ) {
      count++;
      iter = rtems_chain_next(iter);
    }
    if ( count < min || count > max ) {
      printk("%d nodes in level %d at tail block %d\n",
          count, i-1, block);
      rv = false;
    }
  }
  return rv;
}



static void initialize_helper(rtems_task_argument tid, int size) {
  int i;
  skiplist *sl = &the_skiplist[tid];

  for ( i = 0; i <= MAXLEVEL; i++ ) {
    rtems_chain_initialize_empty ( &sl->lists[i] );
  }
  sl->level = 0; /* start at the bottom */
}

static inline
rtems_chain_node* skiplist_down(skiplist *sl, rtems_chain_node *x, int index)
{
  rtems_chain_control *list;
  rtems_chain_control *downlist;
  node *x_node;
  rtems_chain_node *x_down;

  if ( index <= 0 ) {
    return x;
  }

  list = &sl->lists[index];
  downlist = &sl->lists[index-1];
  if ( !rtems_chain_is_head(list, x) ) {
    x_node = LINK_TO_NODE(x, index);
    x_down = &(x_node->link[index-1]);
  } else {
    x_down = rtems_chain_head(downlist);
  }
  return x_down;
}

static inline
rtems_chain_node* skiplist_right(skiplist *sl, rtems_chain_node *x, int index)
{
  rtems_chain_control *list;
  rtems_chain_node *x_right;
  if ( index < 0 ) {
    return x;
  }

  list = &sl->lists[index];
  if ( !rtems_chain_is_tail(list, x) ) {
    x_right = rtems_chain_next(x);
  } else {
    x_right = x;
  }
  return x_right;
}

static void print_node(rtems_chain_node *n, int index) {
  printk("%X\tprev: %X\tnext: %X\tkey: %d\n",
      n, rtems_chain_previous(n), rtems_chain_next(n),
      LINK_TO_NODE(n, index)->data.key);
}

static void print_list(rtems_chain_control *list, int index) {
  rtems_chain_node *node;

  if ( rtems_chain_is_empty(list) ) {
    printk("empty\n");
    return;
  }

  node = rtems_chain_first(list);
  while ( !rtems_chain_is_tail(list, node) ) {
    print_node(node, index);
    node = rtems_chain_next(node);
  }
}


static void print_skiplist_node(rtems_chain_node *n, int index)
{
  printf("%d-", LINK_TO_NODE(n, index)->data.key);
  return;
}

static void
print_skiplist_list(
    rtems_chain_control *list,
    int index,
    rtems_chain_control *all
)
{
  rtems_chain_node *n;
  rtems_chain_node *iter;
 
  printf("%d::-", index);
  if ( rtems_chain_is_empty(list) ) {
    return;
  }

  n = rtems_chain_first(list);
  iter = rtems_chain_first(all);
  while ( !rtems_chain_is_tail(list, n) ) {
    while ( LINK_TO_NODE(n,index) != LINK_TO_NODE(iter,0) ) {
      iter = rtems_chain_next(iter);
      printf("xxxx-");
    }
    print_skiplist_node(n, index);
    n = rtems_chain_next(n);
    iter = rtems_chain_next(iter);
  }
  printf("x\n");
}

static void print_skiplist( skiplist *sl ) {
  int i;

  for ( i = sl->level; i >= 0; i-- ) {
    print_skiplist_list(&sl->lists[i], i, &sl->lists[0]);
  }
  printf("\n");
}

static void insert_helper(rtems_task_argument tid, node *new_node)
{
  rtems_chain_node *x;
  rtems_chain_node *x_forward;
  rtems_chain_node *x_dff;
  node *xdff_node;
  rtems_chain_control *list;
  rtems_chain_control *downlist;
  skiplist *sl = &the_skiplist[tid];  /* list */
  int upper_level = sl->level + 1;    /* list->level */
  int key = new_node->data.key;       /* searchKey */
  int i;
  rtems_chain_node *update[MAXLEVEL+1];

  list = &sl->lists[upper_level]; /* top */
  x = rtems_chain_head(list); /* left */
  // search left-right top-bottom
  for ( i = upper_level; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_forward = skiplist_right(sl, x, i);
    /* Find the rightmost node of level i that is left of the insert point */
    while ( !rtems_chain_is_tail(list, x_forward) &&
            LINK_TO_NODE(x_forward, i)->data.key < key ) {
      x = x_forward;
      x_forward = rtems_chain_next(x);
    }

    /* move down to next level if it exists.
     * see if the next level down needs a promotion.
     * we could store widths to facilitate this check? */
    if ( i ) {
      /* promote the next->next vertical list at level i-1 if it exists and
       * has key less than the next vertical list at level i, i.e. if there
       * exist two vertical lists between x and x_forward */
      downlist = &(sl->lists[i-1]);
      x_dff = skiplist_down(sl, x, i);
      x_dff = skiplist_right(sl, x_dff, i-1);
      x_dff = skiplist_right(sl, x_dff, i-1);
      if ( !rtems_chain_is_tail(downlist, x_dff) ) {
        xdff_node = LINK_TO_NODE(x_dff, i-1);
        if ( rtems_chain_is_tail(list, x_forward) ||
            xdff_node->data.key < LINK_TO_NODE(x_forward, i)->data.key
           ) {
          // promote x_dff ...
          if ( i < upper_level ) { 
            rtems_chain_insert_unprotected(x, &(xdff_node->link[i]));
            if ( xdff_node->data.key < key ) {
              x = rtems_chain_next(x);
            }
          } else if ( sl->level < MAXLEVEL-1 ) {
            rtems_chain_insert_unprotected(x, &(xdff_node->link[i]));
            if ( xdff_node->data.key < key ) {
              x = rtems_chain_next(x);
            }
            sl->level++;
          }
        }
      }
    }
    update[i] = x;
    x = skiplist_down(sl, x, i);
  }

  //assert(list == &sl->lists[0]);

  /* Insert new node only on bottom */
  rtems_chain_insert_unprotected(update[0], &new_node->link[0]);
//  skiplist_verify(sl, 1, 2);
  print_skiplist(sl);
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

