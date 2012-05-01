
/* b tree */

#include "mbptree.h"

#include <stdlib.h>
#include <tmacros.h>
#include <rtems/chain.h>
#include "../shared/params.h"

/* data structure */
static bptree the_tree[NUM_APERIODIC_TASKS];
//static rtems_chain_control *the_list[NUM_APERIODIC_TASKS];

/* data */
static node* the_nodes[NUM_APERIODIC_TASKS];
static bptree_block* the_blocks[NUM_APERIODIC_TASKS];
/* free storage */
static rtems_chain_control freenodes[NUM_APERIODIC_TASKS];
static rtems_chain_control freeblocks[NUM_APERIODIC_TASKS];

/* helpers */
static node *alloc_node(rtems_task_argument tid) {
  node *n = (node*)rtems_chain_get_unprotected( &freenodes[tid] );
  if (!n) {
    printf("%d\tFAILED ALLOCATION OF NODE\n", tid);
  }
  return n;
}
static void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freenodes[tid], (rtems_chain_node*)n );
}

static bptree_block *alloc_block(rtems_task_argument tid) {
  bptree_block *n=(bptree_block*)rtems_chain_get_unprotected(&freeblocks[tid]);
  if (!n) {
    printf("%d\tFAILED ALLOCATION OF BLOCK\n", tid);
  }
  return n;
}
static void free_block(rtems_task_argument tid, bptree_block *n) {
  rtems_chain_append_unprotected( &freeblocks[tid], (rtems_chain_node*)n );
}

static void print_block(bptree_block *b) {
  int i;
  printf("%X\tParent: %X\tKey: %d\n", b, b->parent, b->key);
  if (!b->is_leaf) {
    printf("\tChildren: ");
    for ( i = 0; i <= b->num_nodes; i++ ) {
      printf("%X (%d)\t",b->children[i], b->children[i]->key);
    }
  } else {
    printf("\tNodes: ");
    for ( i = 0; i < b->num_nodes; i++ ) {
      printf("%X (%d)\t",b->nodes[i], b->nodes[i]->data.key);
    }
  }
  printf("\n");
}

static void print_tree(bptree_block *b) {
  int i = 0;
  if (!b->is_leaf) {
    printf("Interior node ");
    print_block(b);
    for ( i = 0; i <= b->num_nodes; i++ ) {
      if ( b->children[i] )
        print_tree(b->children[i]);
    }
  } else {
    print_block(b);
  }
}

static bool bptree_block_is_overfull(bptree_block *block)
{
  return (block->num_nodes == 1+NODES_PER_BLOCK);
}

/* returns index of key, first key greater, or number of nodes */
static int bptree_index_in_block(bptree_block *block, int key)
{
  int i;

  /* TODO: binary search? */
//  assert(block);
//  assert(block->num_nodes);
  for ( i = 0; i < block->num_nodes; i++ ) {
    if ( key < block->children[i]->key ) {
      return i;
    }
  }

  return block->num_nodes;
}

static node* bptree_node_in_leaf(bptree_block *leaf, int key)
{
  int i;

  /* FIXME: binary search? */
  for ( i = 0; i < leaf->num_nodes; i++ ) {
    if ( key <= leaf->nodes[i]->data.key ) {
      return leaf->nodes[i];
    }
  }
//  return rtems_chain_next(nodes[i]);
  return NULL;
}

static bptree_block* bptree_find_block(bptree *tree, int key)
{
  bptree_block *iter = tree->root;
  while (!iter->is_leaf) {
    iter = iter->children[bptree_index_in_block(iter, key)];
  }
  return iter;
}

static void
bptree_add_to_node(bptree_block *b, bptree_block *left, bptree_block *right)
{
  int i;
  int left_index;
 
  left_index = bptree_index_in_block(b, left->key);

  // shift and add
  for ( i = b->num_nodes; i > left_index; i-- ) {
    b->children[i] = b->children[i-1];
  }
  b->children[left_index+1] = right;
  right->parent = b;
  b->num_nodes++;
}

static void bptree_add_to_leaf(bptree_block *b, node *n)
{
  int i;
  for (i = b->num_nodes; i > 0 && b->nodes[i-1]->data.key > n->data.key; i--) {
    b->nodes[i] = b->nodes[i-1];
  }
  b->nodes[i] = n;
  b->key = b->nodes[0]->data.key;
  b->num_nodes++;
}

static bptree_block*
bptree_add_root(bptree *tree, bptree_block *left, bptree_block *right)
{
  bptree_block *new_root;
  new_root = alloc_block(tree->id);
  new_root->is_leaf = false;
  new_root->parent = NULL;
  new_root->children[0] = left;
  new_root->children[1] = right;
  new_root->num_nodes = 1;
  new_root->key = left->key;
  left->parent = new_root;
  right->parent = new_root;
  tree->root = new_root;
  return new_root;
}

static bptree_block*
bptree_split(bptree *tree, bptree_block *v)
{
  bptree_block *x = v->parent;
  bptree_block *right;
  int l_index, r_index, split_index;
  int num_to_right;
  bool leaf = v->is_leaf;

  right = alloc_block(tree->id);
  right->is_leaf = leaf;

  /* put right-half of left into right */
  split_index = (v->num_nodes+1)/2 - 1; // the median
  l_index = v->num_nodes;

  /* split always goes to the right */
  num_to_right = v->num_nodes - split_index;
  r_index = num_to_right;

  for ( ; l_index >= split_index; l_index--, r_index-- ) {
    right->children[r_index] = v->children[l_index];
    if (!leaf)
      right->children[r_index]->parent = right;
  }
  if (!leaf) {
    right->key = right->children[0]->key;
    if ( v->key > v->children[0]->key )
      v->key = v->children[0]->key;
  } else {
    right->key = right->nodes[0]->data.key;
    if ( v->key > v->nodes[0]->data.key )
      v->key = v->nodes[0]->data.key;
  }
  right->num_nodes = num_to_right;
  v->num_nodes = split_index;

  if (!x) {
    x = bptree_add_root(tree, v, right);
  } else {
    bptree_add_to_node(x, v, right);
  }
  return x;
}

static void initialize_helper(rtems_task_argument tid, int size)
{
  the_blocks[tid] = (node*)malloc(sizeof(bptree_block)*size);
  if ( ! the_nodes[tid] ) {
    printk("failed to alloc nodes\n");
    while(1);
  }

  rtems_chain_initialize(
      &freeblocks[tid],
      the_blocks[tid],
      size, /* smaller but ok */
      sizeof(bptree_block)
  );

//  rtems_chain_initialize_empty ( &the_list[tid] );
  the_tree[tid].root = alloc_block(tid);
  the_tree[tid].root->parent = NULL;
  the_tree[tid].root->num_nodes = 0;
  the_tree[tid].root->key = -1;
  the_tree[tid].root->is_leaf = true;
  the_tree[tid].t = 0;
  the_tree[tid].s = 1;
  the_tree[tid].id = tid;

}

static void insert_helper(rtems_task_argument tid, node *new_node)
{
  bptree *tree = &the_tree[tid];
  bptree_block *v = bptree_find_block(tree, new_node->data.key);
  bptree_add_to_leaf(v, new_node);
  while (bptree_block_is_overfull(v)) {
    v = bptree_split(tree, v);
  }
  if (v->is_leaf) {
    if ( v->key > v->nodes[0]->data.key )
      v->key = v->nodes[0]->data.key;
    v = v->parent;
  }
  while ( v && v->children[0]->key < v->key ) {
    v->key = v->children[0]->key;
    v = v->parent;
  }
  print_tree(tree->root);
}

/* FIXME: keep a pointer to min (max) */
static node* min_helper( rtems_task_argument tid ) {
  bptree_block *iter = the_tree[tid].root;
  while (!iter->is_leaf) {
    if (iter->children[0])
      iter = iter->children[0];
    else
      iter = iter->children[1];
  }
  return iter->nodes[0];
}

/* Returns node with same key, first key greater, or tail of list */
static node* search_helper(rtems_task_argument tid, int key)
{
  bptree_block *the_block = bptree_find_block(&the_tree[tid], key);
  return bptree_node_in_leaf(the_block, key);
 }

static node* bptree_remove_from_leaf(bptree_block *leaf, int key)
{
  int i;
  node *n;

  /* TODO: binary search? */
  for ( i = 0; i < leaf->num_nodes; i++ ) {
    if ( key <= leaf->nodes[i]->data.key ) {
      break;
    }
  }
  n = leaf->nodes[i];
  for ( ; i < leaf->num_nodes; i++ ) {
    leaf->nodes[i] = leaf->nodes[i+1];
  }
  leaf->num_nodes--;
  leaf->key = leaf->nodes[0]->data.key;
  return n;
}

static bool bptree_block_is_underfull(bptree_block *block)
{
  return (block->num_nodes == MIN_NODES - 1);
}

static bool bptree_block_can_fuse(bptree *tree, bptree_block *y)
{
  return (y->num_nodes <= MIN_NODES + tree->t);
}

// add right_nodes of right block's children to left block
static void
fuse_blocks(bptree_block *left, bptree_block *right, int right_nodes)
{
  int i, j;
  int left_nodes = left->num_nodes;
  bool leaf = left->is_leaf;

  i = left_nodes;
  if ( right->children[0] ) {
    left->children[i+1] = right->children[0];
    if ( !leaf )
      left->children[i+1]->parent = left;
    ++i;
  }
  for ( j = 1 ; j <= right_nodes; i++, j++ ) {
    left->children[i+1] = right->children[j];
    if ( !leaf )
      left->children[i+1]->parent = left;
  }

  left->num_nodes = i - 1;
  right->num_nodes -= right_nodes;
}

static void delete_from_block(bptree_block *p, bptree_block *d)
{
  int i, d_index;
  bool leaf = d->is_leaf;
  d_index = bptree_index_in_block(p, d->key);
  /*
  if ( leaf ) {
    d_index = bptree_index_in_block(p, d->key);
  } else if ( d->children[0] ) {
    d_index = bptree_index_in_block(p, d->children[0]->key);
  } else {
    d_index = bptree_index_in_block(p, d->children[1]->key);
  }*/

  for ( i = d_index; i < p->num_nodes; i++ ) {
    p->children[i] = p->children[i+1];
  }
  p->children[p->num_nodes - 1] = p->children[p->num_nodes];

  if (!d_index) {
    p->key = p->children[0]->key; // FIXME: necessary / useful? 
  }

  p->num_nodes--;
}

static bptree_block*
bptree_block_fuse(bptree* tree, bptree_block *v, bptree_block *y, int y_side)
{
  // does not matter whether v or y is kept/deleted. choose to keep the left
  // and delete the right sibling. work probably could be saved by choosing
  // to keep the larger and deleting the smaller, but with more code
  if ( y_side < 0 ) {
    bptree_block *tmp = v;
    v = y;
    y = tmp;
  }

  fuse_blocks(v, y, y->num_nodes);
  delete_from_block(y->parent, y);
  free_block(tree->id, y);
  return v;
}

static void
bptree_block_share(bptree* tree, bptree_block *v, bptree_block *y, int y_side)
{
  int y_nodes = y->num_nodes;
  int v_nodes = v->num_nodes;
  int s_nodes = tree->s;
  int i;
  bool leaf = v->is_leaf;

  // unlike fusing, sharing must move children from y into v.
  if ( y_side < 0 ) { 
    // y is to left of v, move end of y into start of v
    // make room at start of v
    for ( i = v_nodes + s_nodes; i > s_nodes; i-- ) {
      v->children[i] = v->children[i - s_nodes - 1];
    }
    // move end of y into start of v
    for ( i = 0; i < s_nodes; i++ ) {
      v->children[s_nodes - 1 - i] = y->children[y_nodes - i];
      if ( !leaf )
        v->children[s_nodes - 1 - i]->parent = v;
    }
    if ( !leaf ) {
      v->key = v->children[0]->key;
    }

    v->num_nodes += s_nodes;
    y->num_nodes -= s_nodes;
  } else {  // y is to right of v and can use similar logic as fusing
    fuse_blocks(v, y, s_nodes);
    y->key = y->children[0]->key;
  }
}

static void bptree_block_delete_root(bptree *tree, bptree_block *r)
{
  bptree_block *c;

  if (r->children[0]) {
    c = r->children[0];
  } else {
    c = r->children[1];
  }

  tree->root = c;
  free_block(tree->id, r);
}

// retval: -1 if y is left of v, 1 if y is right of v, 0 if no sibling.
// writes to y
static int bptree_block_sibling(bptree_block *v, bptree_block **y)
{
  int v_index, y_index;

  if (!v->parent) {
    *y = 0;
    return 0;
  }

  v_index = bptree_index_in_block(v->parent, v->key);
  if ( v_index > 0 ) {
    y_index = v_index - 1;
  } else {
    y_index = v_index + 1;
  }

  if ( y_index >= v->parent->num_nodes ) {
    *y = 0;
    return 0;
  }

  *y = v->parent->children[y_index];
  return y_index - v_index;
}

static void continue_extract(bptree *tree, bptree_block *v)
{
  bptree_block *y;
  int y_side;
  y_side = bptree_block_sibling(v, &y); // sibling exists

  while (bptree_block_is_underfull(v) && bptree_block_can_fuse(tree, y)) {
    v = bptree_block_fuse(tree, v, y, y_side);
    v = v->parent;
    y_side = bptree_block_sibling(v, &y);
    if (!y) {
      // v is the root
      if (v->num_nodes == 0) {
        bptree_block_delete_root(tree, v);
      }
      return;
    }
  }

  if (bptree_block_is_underfull(v)) {
    bptree_block_share(tree, v, y, y_side);
  }
}

static long extract_helper(rtems_task_argument tid, int key) {
  long kv;
  bptree *tree = &the_tree[tid];
  bptree_block *v = bptree_find_block(tree, key);
  node *w = bptree_remove_from_leaf(v, key); // FIXME: need tree?
  if ( w ) {
    kv = PQ_NODE_TO_KV(&w->data);
    free_node(tid, w);
    if ( v->parent && bptree_block_is_underfull(v) ) {
      continue_extract(tree, v);
    }
  } else {
    kv = (long)-1;
  }

  return kv;
}

/**
 * benchmark interface
 */
void bptree_initialize( rtems_task_argument tid, int size ) {
  the_nodes[tid] = (node*)malloc(sizeof(node)*size);
  if ( ! the_nodes[tid] ) {
    printk("failed to alloc nodes\n");
    while(1);
  }

  rtems_chain_initialize(&freenodes[tid], the_nodes[tid], size, sizeof(node));

  initialize_helper(tid, size);
}

void bptree_insert(rtems_task_argument tid, long kv ) {
  node *new_node = alloc_node(tid);
  if (!new_node)
    return;
  new_node->data.key = kv_key(kv);
  new_node->data.val = kv_value(kv);
  insert_helper(tid, new_node);
}

long bptree_min( rtems_task_argument tid ) {
  node *n;

  n = min_helper(tid);
  if (n) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (long)-1;
}

long bptree_pop_min( rtems_task_argument tid ) {
  long kv;
  node *n;
  //  FIXME
//  n = min_helper(tid);
  n = NULL;
  if (n) {
    kv = PQ_NODE_TO_KV(&n->data);
    free_node(tid, n);
  } else {
    kv = (long)-1;
  }
  return kv;
}

long bptree_search( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
//  if (!rtems_chain_is_tail(&the_list[tid], n) && n->data.key == k) {
  if ( n && n->data.key == k ) {
    return PQ_NODE_TO_KV(&n->data);
  }
  print_tree(the_tree[tid].root);
  return (long)-1;
}

long bptree_extract( rtems_task_argument tid, int k ) {
  long kv = extract_helper(tid, k);
  return kv;
}

