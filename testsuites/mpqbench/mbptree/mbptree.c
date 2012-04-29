
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
  return n;
}
static void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freenodes[tid], (rtems_chain_node*)n );
}

static bptree_block *alloc_block(rtems_task_argument tid) {
  bptree_block *n=(bptree_block*)rtems_chain_get_unprotected(&freeblocks[tid]);
  return n;
}
static void free_block(rtems_task_argument tid, bptree_block *n) {
  rtems_chain_append_unprotected( &freeblocks[tid], (rtems_chain_node*)n );
}

static void print_block(bptree_block *b) {
  int i;
  printf("%X\tParent: %X\tNodes: ", b, b->parent);
  for ( i = 0; i < b->num_nodes; i++ ) {
    printf("%d\t",b->nodes[i]->data.key);
  }
  if (!b->is_leaf) {
    printf("\n\tChildren: ");
    for ( i = 0; i <= b->num_nodes; i++ ) {
      printf("%X (%d)\t",b->children[i], b->children[i]->nodes[0]->data.key);
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

static inline bool bptree_block_is_overfull(bptree_block *block)
{
  return (block->num_nodes == 1+NODES_PER_BLOCK);
}
static inline bool bptree_block_is_full(bptree_block *block)
{
  return (block->num_nodes == NODES_PER_BLOCK);
}

/* returns index of key, first key greater, or number of nodes */
static inline int bptree_index_in_block(bptree_block *block, int key)
{
  int i;

  /* TODO: binary search? */
  for ( i = 0; i < block->num_nodes; i++ ) {
    if ( key < block->nodes[i]->data.key ) {
      return i;
    }
  }

  return block->num_nodes;
}

static inline node* bptree_node_in_block(bptree_block *block, int key)
{
  int i;

//  printk("Search: %X\t%d\n", block, key);

  /* FIXME: binary search? */
  for ( i = 0; i < block->num_nodes; i++ ) {
//    printk("nodes[%d]->data.key == %d\n", i, block->nodes[i]->data.key);
    if ( key <= block->nodes[i]->data.key ) {
      return block->nodes[i];
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

static inline void
bptree_add_to_node(bptree_block *b, bptree_block *left, bptree_block *right)
{
  int i;
  int left_index = bptree_index_in_block(b, left->nodes[0]->data.key);
  //printf("bptree_add_to_node\n");

  for ( i = b->num_nodes; i > left_index; i-- ) {
    b->nodes[i] = b->nodes[i-1];
    b->children[i+1] = b->children[i];
  }
  b->nodes[left_index] = right->nodes[0];
  b->children[left_index+1] = right;
  b->num_nodes++;
  right->parent = b;
}

static inline void bptree_add_to_leaf(bptree_block *b, node *n, int index)
{
  int i;
  //printf("bptree_add_to_leaf\n");
  for ( i = b->num_nodes; i > index; i-- ) {
    b->nodes[i] = b->nodes[i-1];
  }
  b->nodes[index] = n;
  b->num_nodes++;
}

static inline bptree_block*
bptree_add_root(bptree *tree, bptree_block *left, bptree_block *right)
{
  bptree_block *new_root;
  //printf("bptree_add_root\n");
  new_root = alloc_block(tree->id);
  new_root->is_leaf = false;
  new_root->parent = NULL;
  new_root->children[0] = left;
  new_root->children[1] = right;
  new_root->num_nodes = 1;
  new_root->nodes[0] = right->nodes[0];
  left->parent = new_root;
  right->parent = new_root;
  tree->root = new_root;
  return new_root;
}

static void
bptree_split(bptree *tree, bptree_block *l, bptree_block *n);

/* assume left already exists, adding right and updating child links */
static inline void
bptree_add_to_parent(bptree *tree, bptree_block *left, bptree_block *right)
{
  bptree_block *p = left->parent;
  printf("bptree_add_to_parent\n");
  if ( !p ) {
    bptree_add_root(tree, left, right);
  } else if (bptree_block_is_full(p)) {
    bptree_split(tree, p, right); // calls bptree_add_to_parent
  } else {
   bptree_add_to_node(p, left, right); 
  }
}

static void
bptree_split(bptree *tree, bptree_block *left, bptree_block *n)
{
  bptree_block *right;
  int split_index;
  int n_index;
  int l_index;
  int r_index;
  int num_to_right;

  right = alloc_block(tree->id);
  right->is_leaf = false;

  printf("bptree_split\n");
  n_index = bptree_index_in_block(left, n->nodes[0]->data.key);
  printf("%d\n",n_index);
  /* put right half of left into right */
  split_index = (left->num_nodes+3)/2 - 1; // the median including n
//  printf("split_index: %d\n", split_index);
  l_index = left->num_nodes - 1;
  /* split always goes to the right */
  num_to_right = left->num_nodes - split_index + 1;
//  printf("num_to_right: %d\n", num_to_right);
  r_index = num_to_right-1;
  if ( n_index >= split_index ) {
    printf("a\n");
    for ( ; l_index >= n_index; l_index--, r_index-- ) {
      right->nodes[r_index] = left->nodes[l_index];
      right->children[r_index + 1] = left->children[l_index+1];
      right->children[r_index + 1]->parent = right;
    }
    right->nodes[r_index] = n->nodes[0];
    right->children[r_index+1] = n;
    right->children[r_index + 1]->parent = right;
    r_index--;
    for ( ; l_index >= split_index; l_index--, r_index-- ) {
      right->nodes[r_index] = left->nodes[l_index];
      right->children[r_index + 1] = left->children[l_index+1];
      right->children[r_index + 1]->parent = right;
    }
    // l_index+1 == split_index
    right->children[0] = NULL; // FIXME: is this correct?
//    right->children[r_index+1] = left->children[l_index+1];
//    right->children[r_index+1]->parent = right;
  } else {
    printf("b\n");
    for ( ; l_index >= split_index-1; l_index--, r_index-- ) {
      right->nodes[r_index] = left->nodes[l_index];
      right->children[r_index + 1] = left->children[l_index+1];
      right->children[r_index + 1]->parent = right;
    }
    right->children[r_index+1] = left->children[l_index+1];
    right->children[r_index+1]->parent = right;
    for ( ; l_index >= n_index; l_index-- ) {
      printf("%d\n", l_index);
      left->nodes[l_index+1] = left->nodes[l_index];
      left->children[l_index+2] = left->children[l_index+1];
    }
    left->nodes[n_index] = n->nodes[0];
    left->children[n_index+1] = n;
    n->parent = left;
  }
  right->num_nodes = num_to_right;
  left->num_nodes = left->num_nodes + 1 - num_to_right;
  right->parent = left->parent;

  /* update parent's links to children */
  bptree_add_to_parent(tree, left, right);
}


/* similar to bptree_split, but keep separate for now. This version is
 * simplified by using the node directly and not updating children links */
static void bptree_split_leaf(bptree *tree, bptree_block *left_leaf, node *n)
{
  bptree_block *right_leaf;
  int split_index;
  int n_index;
  int l_index;
  int r_index;
  int num_to_right;

  right_leaf = alloc_block(tree->id);
  right_leaf->is_leaf = true;

  printf("bptree_split_leaf\n");
  n_index = bptree_index_in_block(left_leaf, n->data.key);

  /* put right half of left_leaf into the right_leaf */
  split_index = (left_leaf->num_nodes+3)/2 - 1; // the median including n
  l_index = left_leaf->num_nodes - 1;
  /* split always goes to the right */
  num_to_right = left_leaf->num_nodes - split_index + 1;
  r_index = num_to_right-1;
  if ( n_index >= split_index ) { // n goes to right of split
    printf("a\n");
    for ( ; l_index >= n_index; l_index--, r_index-- ) {
      right_leaf->nodes[r_index] = left_leaf->nodes[l_index];
    }
    right_leaf->nodes[r_index] = n;
    r_index--;
    for ( ; l_index >= split_index; l_index--, r_index-- ) {
      right_leaf->nodes[r_index] = left_leaf->nodes[l_index];
    }
  } else {
    printf("b\n");
    /* go past split_index so that split always goes to the right */
    for ( ; l_index >= split_index-1; l_index--, r_index-- ) {
      right_leaf->nodes[r_index] = left_leaf->nodes[l_index];
    }
    for ( ; l_index >= n_index; l_index-- ) {
      left_leaf->nodes[l_index+1] = left_leaf->nodes[l_index];
    }
    left_leaf->nodes[n_index] = n;
  }
  right_leaf->num_nodes = num_to_right;
  left_leaf->num_nodes = left_leaf->num_nodes + 1 - num_to_right;
  
  /* update parent's links to children */
  bptree_add_to_parent(tree, left_leaf, right_leaf);
  left_leaf->parent->is_leaf = false;
}


static bptree_block*
bptree_split_hm(bptree *tree, bptree_block *v)
{
  bptree_block *x = v->parent;
  bptree_block *right;
  int l_index, r_index, split_index;
  int num_to_right;

  //printf("bptree_split_hm\n");
  right = alloc_block(tree->id);
  right->is_leaf = v->is_leaf;

  //printf("x: %X\n", x);

  /* put right-half of left into right */
  split_index = (v->num_nodes+1)/2 - 1; // the median including n
  //printf("split_index: %d\n", split_index);
  l_index = v->num_nodes - 1;
  //printf("l_index: %d\n", l_index);
  /* split always goes to the right */
  num_to_right = v->num_nodes - split_index;
  //printf("num_to_right: %d\n", num_to_right);
  r_index = num_to_right-1;
  //printf("r_index: %d\n", r_index);

  for ( ; l_index >= split_index; l_index--, r_index-- ) {
    right->nodes[r_index] = v->nodes[l_index];
    right->children[r_index + 1] = v->children[l_index+1];
    right->children[r_index + 1]->parent = right;
  }
  //printf("r_index: %d\n", r_index);
  right->children[0] = NULL; // FIXME: is this correct?
  
  // what about right->children[0] and right->nodes[0]??

  right->num_nodes = num_to_right;
  v->num_nodes = v->num_nodes - num_to_right;

  if (!x) {
    x = bptree_add_root(tree, v, right);
  } else {
    bptree_add_to_node(x, v, right);
  }
  return x;
}

static inline void initialize_helper(rtems_task_argument tid, int size)
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
  the_tree[tid].root->is_leaf = true;
  the_tree[tid].id = tid;
}

static inline void insert_helper(rtems_task_argument tid, node *new_node)
{
  bptree *tree = &the_tree[tid];

#if 1
  // Huddleston-Mehlhorn
  bptree_block *v = bptree_find_block(tree, new_node->data.key);
  bptree_add_to_leaf(v, new_node, bptree_index_in_block(v,new_node->data.key));
  while (bptree_block_is_overfull(v)) {
    v = bptree_split_hm(tree, v);
  }
#endif

#if 0
  bptree_block *b;
  b = bptree_find_block(tree, new_node->data.key);

  if (bptree_block_is_full(b)) {
    bptree_split_leaf(tree, b, new_node);
  } else {
    bptree_add_to_leaf(b,new_node,bptree_index_in_block(b,new_node->data.key));
  }
#endif
  //print_tree(tree->root);
}

/* FIXME: keep a pointer to min (max) */
static inline node* min_helper( rtems_task_argument tid ) {
  bptree_block *iter = the_tree[tid].root;
  while (!iter->is_leaf) {
    iter = iter->children[0];
  }
  return iter->nodes[0];
}

/* Returns node with same key, first key greater, or tail of list */
static inline node* search_helper(rtems_task_argument tid, int key)
{
  bptree_block *the_block = bptree_find_block(&the_tree[tid], key);
  return bptree_node_in_block(the_block, key);
 }

static inline uint64_t extract_helper(rtems_task_argument tid, int k) {
  uint64_t kv;

  //rtems_chain_extract_unprotected(n);
  //free_node(tid, n);
  kv = (uint64_t)-1;

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

void bptree_insert(rtems_task_argument tid, uint64_t kv ) {
  node *new_node = alloc_node(tid);
  new_node->data.key = kv_key(kv);
  new_node->data.val = kv_value(kv);
  insert_helper(tid, new_node);
}

uint64_t bptree_min( rtems_task_argument tid ) {
  node *n;

  n = min_helper(tid);
  if (n) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (uint64_t)-1;
}

uint64_t bptree_pop_min( rtems_task_argument tid ) {
  uint64_t kv;
  node *n;
  //  FIXME
//  n = min_helper(tid);
  n = NULL;
  if (n) {
    kv = PQ_NODE_TO_KV(&n->data);
    free_node(tid, n);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t bptree_search( rtems_task_argument tid, int k ) {
  node* n = search_helper(tid, k);
//  if (!rtems_chain_is_tail(&the_list[tid], n) && n->data.key == k) {
  if ( n && n->data.key == k ) {
    return PQ_NODE_TO_KV(&n->data);
  }
  return (uint64_t)-1;
}

uint64_t bptree_extract( rtems_task_argument tid, int k ) {
  uint64_t kv = extract_helper(tid, k);
  return kv;
}

