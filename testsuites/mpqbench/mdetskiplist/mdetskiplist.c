
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
  node *n = (node*)rtems_chain_get_unprotected( &freenodes[tid] );
  return n;
}
static void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freenodes[tid], (rtems_chain_node*)n );
}

/*
 * a deterministic 1-2 skiplist enforces a property that between any two nodes
 * at level i there are either 1 or 2 nodes at level i - 1.
 * this debug helper routine verifies the 1-2 property but for any min/max.
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
  int count_gaps[10];
  int block;
  int i;
  bool rv = true;
  
  for ( i = 0; i < 10; i++ ) {
    count_gaps[i] = 0;
  }

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
    printf("%d nodes in [top] level %d\n", count, i);
    rv = false; // this should be allowed, since skiplist could saturate.
  }
  if ( count >= 9 ) {
    count_gaps[9]++;
  }
  else {
    count_gaps[count]++;
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
      printf("%d nodes in level %d (head)\n", count, i);
      rv = false;
    }
    if ( count >= 9 ) {
      count_gaps[9]++;
    }
    else {
      count_gaps[count]++;
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
        printf("%d nodes in level %d\n", count, i);
        rv = false;
      }
      if ( count >= 9 ) {
        count_gaps[9]++;
      }
      else {
        count_gaps[count]++;
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
      printf("%d nodes in level %d (tail)\n", count, i);
      rv = false;
    }
    if ( count >= 9 ) {
      count_gaps[9]++;
    }
    else {
      count_gaps[count]++;
    }
  }

  if ( !rv ) {
    printf("Gaps: ");
    for ( i = 0; i < 9; i++ ) {
      printf("%d ",count_gaps[i]);
    }
    printf("%d\n", count_gaps[9]);
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
  if ( rtems_chain_is_head(list, x) ) {
    x_down = rtems_chain_head(downlist);
  } else if ( rtems_chain_is_tail(list, x) ) {
    x_down = rtems_chain_tail(downlist);
  } else {
    x_node = LINK_TO_NODE(x, index);
    x_down = &(x_node->link[index-1]);
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

#if 0
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
#endif

static void print_skiplist_node(rtems_chain_node *x, int index)
{
  node *n = LINK_TO_NODE(x, index);
  printf("%d(%d)-", n->data.key, n->height);
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

static void print_skiplist_between(node *start, node *stop)
{
  int i;
  rtems_chain_node *x;
  node *n;

  for ( i = start->height - 1; i >= 0; i-- ) {
    printf("%d::-", i);
    x = &start->link[i];
    n = LINK_TO_NODE(x,i);
    while ( n->data.key <= stop->data.key ) {
      print_skiplist_node(x, i);
      x = rtems_chain_next(x);
      n = LINK_TO_NODE(x,i);
    }
    print_skiplist_node(x, i);
    printf("x\n");
  }
}

static void print_skiplist_neighbors(node *x, int index)
{
  rtems_chain_node *left, *right;

  left = rtems_chain_previous(&x->link[index]);
  right = rtems_chain_next(&x->link[index]);

  print_skiplist_node( left, index );
  printf("->");
  print_skiplist_node( &x->link[index], index );
  printf("->");
  print_skiplist_node( right, index );
}

static node* find_gap_start(node *inner, int height)
{
  rtems_chain_node *iter;
  node *iter_node;

  iter_node = inner;
  iter = &iter_node->link[height-1];
  while ( !rtems_chain_is_first(iter) && iter_node->height == height ) {
    iter = rtems_chain_previous(iter);
    iter_node = LINK_TO_NODE(iter, height-1);
  }
  return iter_node;
}

static node* find_gap_stop(node *inner, int height)
{
  rtems_chain_node *iter;
  node *iter_node;

  iter_node = inner;
  iter = &iter_node->link[height-1];
  while ( !rtems_chain_is_last(iter) && iter_node->height == height ) {
    iter = rtems_chain_next(iter);
    iter_node = LINK_TO_NODE(iter, height-1);
  }
  return iter_node;
}


static void print_skiplist_gap(node *x)
{
  node *first_node;
  node *last_node;
  int h = x->height;

  first_node = find_gap_start(x, h);
  last_node = find_gap_stop(x, h);
  print_skiplist_between(first_node, last_node);
}

static void print_skiplist( skiplist *sl ) {
  int i;

  for ( i = sl->level; i >= 0; i-- ) {
    print_skiplist_list(&sl->lists[i], i, &sl->lists[0]);
  }
  printf("\n");
}

static void skiplist_promote(
    skiplist *sl,
    rtems_chain_node *before,
    node *promote_me,
    int top,
    int index
)
{
  if ( index <= top || sl->level < MAXLEVEL-1 ) { 
    rtems_chain_insert_unprotected(before, &(promote_me->link[index]));
    promote_me->height++;
    if ( index > top ) {
      sl->level++;
    }
  }
}

static void skiplist_demote(
    skiplist *sl,
    node *demote_me,
    int index
)
{
  rtems_chain_node *x = &(demote_me->link[index]);
  rtems_chain_extract_unprotected(x);
  demote_me->height--;
}

static void
skiplist_fix_after_extract(skiplist *sl, rtems_chain_node **update, int top)
{
  int i;
#if 0
  int count;
  node *start, *stop;

  node *x;

  for ( i = 0; i <= top; i++ ) {
    x = update[i+1];
    count = count_from(sl, x, i+1);
    if ( count == 0 ) {

    } else if ( count > 2 ) {

    }



    start = find_gap_start(x, i+1);
    stop = find_gap_stop(x, i+1);

  }
#endif

  for ( i = top; i > 0; i-- ) {
    if ( rtems_chain_is_empty(&sl->lists[i]) ) {
      sl->level--;
    } else {
      break;
    }
  }
}

static void
skiplist_fix_after_insert(skiplist *sl, rtems_chain_node **update, int top)
{
  int i;
  int count;
  rtems_chain_node *x;
  rtems_chain_node *xf;
  rtems_chain_node *xd;
  rtems_chain_node *xfd;
  rtems_chain_node *iter;
  rtems_chain_node *promote;
  node *p_node;

  /* 
   * trace back through the skiplist fixing violations
   */
  for ( i = 0; i <= top; i++ ) {
    x = update[i + 1];
    xf = skiplist_right(sl, x, i+1);
    xd = skiplist_down(sl, x, i+1);
    xfd = skiplist_down(sl, xf, i+1);
    iter = xd;
    count = -1;
    while ( iter != xfd ) {
      count++;
      iter = rtems_chain_next(iter);
    }
    if ( count > 3 ) {
      printf("ERROR: INVALID COUNT: %d\n", count);
    }
    if ( count > 2 ) {
      promote = xd;
      promote = rtems_chain_next(xd);
      promote = rtems_chain_next(promote);
      p_node = LINK_TO_NODE(promote, i);
      skiplist_promote(sl, x, p_node, top, i+1);
    } else {
      break;
    }
  }
}

static void insert_helper(rtems_task_argument tid, node *new_node)
{
  rtems_chain_node *x;
  rtems_chain_node *x_f;  /* x_f */
  node *xf_node;
  rtems_chain_control *list;
  skiplist *sl = &the_skiplist[tid];  /* list */
  int top = sl->level;    /* list->level */
  int key = new_node->data.key;       /* searchKey */
  int i;
  rtems_chain_node *update[MAXLEVEL+1];

  update[top + 1] = rtems_chain_head(&sl->lists[top+1]);

  list = &sl->lists[top]; /* top */
  x = rtems_chain_head(list); /* left */
  // search left-right top-bottom
  for ( i = top; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_f = skiplist_right(sl, x, i);
    xf_node = LINK_TO_NODE(x_f, i);
    /* Find the rightmost node of level i that is left of the insert point */
    while ( !rtems_chain_is_tail(list, x_f) && xf_node->data.key < key ) {
      x = x_f;
      x_f = rtems_chain_next(x);
      xf_node = LINK_TO_NODE(x_f, i);
    }
    update[i] = x;
    x = skiplist_down(sl, x, i);
  }

  //assert(list == &sl->lists[0]);

  /* Insert new node only on bottom */
  skiplist_promote(sl, update[0], new_node, top, 0);
  
  /* fix-up violations */
  skiplist_fix_after_insert(sl, update, top);

  /* debug */
  //print_skiplist(sl);
  skiplist_verify(sl, 1, 2);
}

/* Returns node with same key, first key greater, or tail of list */
static node* search_helper(rtems_task_argument tid, int key)
{
  rtems_chain_node *x;
  rtems_chain_node *x_f;
  node *xf_node;
  rtems_chain_control *list;
  skiplist *sl = &the_skiplist[tid];  /* list */
  int top = sl->level;        /* list->level */
  int i;

  list = &sl->lists[top]; /* top */
  x = rtems_chain_head(list); /* left */
  // search left-right top-bottom
  for ( i = top; i >= 0; i-- ) {
    list = &sl->lists[i];
    x_f = skiplist_right(sl, x, i);
    xf_node = LINK_TO_NODE(x_f, i);
    /* Find the rightmost node of level i that is left of the insert point */
    while (!rtems_chain_is_tail(list, x_f) && xf_node->data.key < key) {
      x = x_f;
      x_f = rtems_chain_next(x);
      xf_node = LINK_TO_NODE(x_f, i);
    }
    x = skiplist_down(sl, x, i);
  }

  x = x_f;
  return LINK_TO_NODE(x, 0);
}

static inline int count_from(skiplist *sl, rtems_chain_node *x, int index)
{
  rtems_chain_node *x_f;
  rtems_chain_node *x_fd;
  rtems_chain_node *x_d;
  rtems_chain_node *iter;
  node *x_node;
  int count;

  x_f = skiplist_right(sl, x, index);
  x_fd = skiplist_down(sl, x_f, index);
  x_d = skiplist_down(sl, x, index);

  iter = x_d;
  count = -1;
  while ( iter != x_fd ) {
    iter = rtems_chain_next(iter);
    count++;
  }
  return count;
}

static inline long extract_helper(rtems_task_argument tid, int key)
{
  // TODO: perhaps mark node for deletion and deal with later.
  rtems_chain_node *x;
  rtems_chain_node *x_f;
  node *x_node;
  node *xf_node;
  rtems_chain_control *list;
  skiplist *sl = &the_skiplist[tid];  /* list */
  int top = sl->level;        /* list->level */
  int i;
  int count;
  long kv;
  rtems_chain_node *update[MAXLEVEL + 1];
  update[top + 1] = rtems_chain_head(&sl->lists[top+1]);

  list = &sl->lists[top]; /* top */
  x = rtems_chain_head(list); /* left */
  // search left-right top-bottom
  for ( i = top + 1; i >= 0; i-- ) {
    list = &sl->lists[i];
    count = 0;
    x_f = skiplist_right(sl, x, i);
    xf_node = LINK_TO_NODE(x_f, i);
    /* Find the rightmost node of level i that is left of the insert point */
    while (!rtems_chain_is_tail(list, x_f) && xf_node->data.key < key) {
      count++;
      x = x_f;
      x_f = rtems_chain_next(x);
      xf_node = LINK_TO_NODE(x_f, i);
    }
    /* check for raising/lowering */
#if 0
    if ( i ) {
      if ( !count ) {
        count = count_from(sl, x, i);
        if ( count == 1 ) {
          count = count_from(sl, x_f, i);
          skiplist_demote(sl, xf_node, i);
          if ( count > 1 ) {
            rtems_chain_node *n = rtems_chain_next(x_f);
            node *n_node = LINK_TO_NODE(n, i-1);
            skiplist_promote(sl, x, n_node, top, i);
          }
        }
      } else {
        rtems_chain_node *x_b = rtems_chain_previous(x);
        count = count_from(sl, x, i);
        if ( count == 1) {
          count = count_from(sl, x_b, i);
          x_node = LINK_TO_NODE(x, i);
          skiplist_demote(sl, x_node, i);
          if ( count > 1 ) {
            rtems_chain_node *n = rtems_chain_previous(x);
            node *n_node = LINK_TO_NODE(n, i-1);
            skiplist_promote(sl, x_b, n_node, top, i);
            x = &(n_node->link[i]);
          } else {
            x = x_b;
          }
        }
      }
      printf("%d\n",i);
      skiplist_verify(sl, 1, 2);
    }
#endif
    update[i] = x;
    x = skiplist_down(sl, x, i);
  }

  x = x_f;
  if ( !rtems_chain_is_tail(&sl->lists[0], x) ) {
    x_node = LINK_TO_NODE(x, 0);
    if ( x_node->data.key == key ) {
      node *target_node;
      kv = PQ_NODE_TO_KV(&x_node->data);
#if 0
      /* swap node with neighbor having level 0 */
      print_skiplist_gap(x_node);
      if ( x_node->height > 1 ) {
        rtems_chain_node *neighbor = rtems_chain_next(x);
        node *neighbor_node = LINK_TO_NODE(neighbor, 0);
        target_node = neighbor_node;
        printf("invalid node x: ");
        print_skiplist_node(x, 0);
        printf("\n");

        if ( neighbor_node->height > 1 ) {
          printf("invalid right neighbor: ");
          print_skiplist_node(neighbor, 0);
          printf("\n");

          neighbor = rtems_chain_previous(x);
          neighbor_node = LINK_TO_NODE(neighbor, 0);
          target_node = neighbor_node;
        }
        if ( neighbor_node->height > 1 ) {
          printf("invalid left neighbor: ");
          print_skiplist_node(neighbor, 0);
          printf("\n");
        } else {
          printf("target neighborhood 0: ");
          print_skiplist_neighbors(target_node, 0);
          printf("\n");

          print_skiplist_gap(target_node);

          skiplist_demote(sl, target_node, 0);
          printf("target neighborhood 0: ");
          print_skiplist_neighbors(target_node, 0);
          printf("\n");
          for ( i = 0; i < x_node->height; i++ ) {
            skiplist_promote(sl, &x_node->link[i], target_node, top, i);
            printf("target neighborhood %d: ", i);
            print_skiplist_neighbors(target_node, i);
            printf("\n");
          }
          print_skiplist_gap(target_node);

          printf("target neighborhood 0: ");
          print_skiplist_neighbors(target_node, 0);
          printf("\n");

          for ( i = 0; i < x_node->height; i++ ) {
            printf("x neighborhood %d: ", i);
            print_skiplist_neighbors(x_node, i);
            printf("\n");
          }
        }
      } else {
        printf("good node x neighborhood: ");
        print_skiplist_neighbors(x_node, 0);
        printf("\n");
      }
#endif
      for ( i = 0; i < x_node->height; i++ ) {
        rtems_chain_extract_unprotected(&x_node->link[i]);
      }
      skiplist_fix_after_extract(sl, update, top);
      free_node(tid, x_node);
    } else {
      kv = (long)-1;
    }
  } else {
    kv = (long)-1;
  }

  /* debug */
  //print_skiplist(sl);
  skiplist_verify(sl, 1, 2);

  return kv;
}

/**
 * benchmark interface
 */
void skiplist_initialize( rtems_task_argument tid, int size ) {

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
  new_node->height = 0;
  insert_helper(tid, new_node);
}

long skiplist_min( rtems_task_argument tid ) {
  node *n;

  n = (node*)rtems_chain_first(&the_skiplist[tid].lists[0]); // unprotected
  if (n) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (long)-1;
}

long skiplist_pop_min( rtems_task_argument tid ) {
  long kv;
  node *n;
  n = (node*)rtems_chain_first(&the_skiplist[tid].lists[0]); // unprotected
  kv = extract_helper(tid, n->data.key);
  return kv;
}

long skiplist_search( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
  if ( !rtems_chain_is_tail(&the_skiplist[tid].lists[0], (rtems_chain_node*)n)
      && n->data.key == k) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (long)-1;
}

long skiplist_extract( rtems_task_argument tid, int k ) {
  long kv = extract_helper(tid, k);
  return kv;
}

