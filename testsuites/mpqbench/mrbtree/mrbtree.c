
#include "mrbtree.h"

#include <stdlib.h>

node the_nodes[NUM_NODES][NUM_APERIODIC_TASKS];
rtems_rbtree_control the_rbtree[NUM_APERIODIC_TASKS];
rtems_chain_control freelist[NUM_APERIODIC_TASKS];

node *alloc_node(rtems_task_argument tid) {
  node *n = rtems_chain_get_unprotected( &freelist[tid] );
  return n;
}
void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freelist[tid], n );
}

static int rbtree_compare(
  rtems_rbtree_node* n1,
  rtems_rbtree_node* n2
) {
  int key1 = rtems_rbtree_container_of( n1, node, rbt_node )->data.key; 
  int key2 = rtems_rbtree_container_of( n2, node, rbt_node )->data.key;

  return key1 - key2;
}

//#define USE_RB_ASSERT
static int rb_assert ( rtems_rbtree_node *root )
{
  int lh, rh;

  if ( root == NULL )
    return 1;
  else {
    rtems_rbtree_node *ln = rtems_rbtree_left(root);
    rtems_rbtree_node *rn = rtems_rbtree_right(root);

    /* Consecutive red links */
    if ( root->color == RBT_RED ) {
      if ((ln && ln->color == RBT_RED)  || (rn && rn->color == RBT_RED)) {
        puts ( "Red violation" );
        return -1;
      }
    }

      lh = rb_assert ( ln );
      rh = rb_assert ( rn );

    /* Invalid binary search tree */
    if ( ( ln != NULL && rbtree_compare(ln, root) > 0 )
        || ( rn != NULL && rbtree_compare(rn, root) < 0 ) )
    {
      puts ( "Binary tree violation" );
      return -1;
    }

    /* Black height mismatch */
    if ( lh != -1 && rh != -1 && lh != rh ) {
      puts ( "Black violation" );
      return -1;
    }

    /* Only count black links */
    if ( lh != -1 && rh != -1 )
      return ( root->color == RBT_RED ) ? lh : lh + 1;
    else
      return -1;
  }
}

#if 0
/** @brief Find the node with given key in the tree
 *
 *  This function returns a pointer to the node in @a the_rbtree 
 *  having key equal to key of  @a the_node if it exists,
 *  and NULL if not. @a the_node has to be made up before a search.
 */
rtems_rbtree_node* test_rbtree_find_int32(
    rtems_rbtree_control *the_rbtree,
    rtems_rbtree_node *the_node
    )
{
  int32_t find_key = rtems_rbtree_container_of(the_node, node, rbt_node)->data.key;
  int32_t iter_key;
  rtems_rbtree_node* iter_node = the_rbtree->root;
  rtems_rbtree_node* found = NULL;
  while (iter_node) {
    iter_key = rtems_rbtree_container_of( iter_node, node, rbt_node )->data.key;
    if (find_key == iter_key)
      found = iter_node;

    rtems_rbtree_direction dir = find_key > iter_key;
    iter_node = iter_node->child[dir];
  } /* while(iter_node) */

  return found;
}

/** @brief Insert a Node (unprotected)
 *
 *  This routine inserts @a the_node on the Red-Black Tree @a the_rbtree.
 *
 *  @retval 0 Successfully inserted.
 *  @retval -1 NULL @a the_node.
 *  @retval RBTree_Node* if one with equal key to the key of @a the_node exists
 *          in @a the_rbtree.
 *
 *  @note It does NOT disable interrupts to ensure the atomicity
 *        of the extract operation.
 */
rtems_rbtree_node* test_rbtree_insert_int32(
    rtems_rbtree_control *the_rbtree,
    rtems_rbtree_node *the_node
    )
{
  int32_t the_key = rtems_rbtree_container_of(the_node, node, rbt_node)->data.key;
  int32_t iter_key;
  rtems_rbtree_direction dir;

  if(!the_node) 
    return (rtems_rbtree_node*)-1;

  rtems_rbtree_node *iter_node = the_rbtree->root;

  if (!iter_node) { /* special case: first node inserted */
    the_node->color = RBT_BLACK;
    the_rbtree->root = the_node;
    the_rbtree->first[0] = the_rbtree->first[1] = the_node;
    the_node->parent = (rtems_rbtree_node *) the_rbtree;
    the_node->child[RBT_LEFT] = the_node->child[RBT_RIGHT] = NULL;
  } else {
    /* typical binary search tree insert, descend tree to leaf and insert */
    while (iter_node) {
      iter_key = rtems_rbtree_container_of( iter_node, node, rbt_node )->data.key;
      dir = the_key >= iter_key;
      if (!iter_node->child[dir]) {
        the_node->child[RBT_LEFT] = the_node->child[RBT_RIGHT] = NULL;
        the_node->color = RBT_RED;
        iter_node->child[dir] = the_node;
        the_node->parent = iter_node;
        /* update min/max */
        if (rtems_rbtree_is_first(the_rbtree, iter_node, dir)) {
          the_rbtree->first[dir] = the_node;
        }
        break;
      } else {
        iter_node = iter_node->child[dir];
      }
    } /* while(iter_node) */
  }
  return (rtems_rbtree_node*)0;
}
#endif

void rbtree_initialize( rtems_task_argument tid, int size ) {
  int i;

  rtems_chain_initialize_empty ( &freelist[tid] );
  for ( i = 0; i < size; i++ ) {
    rtems_chain_append(&freelist[tid], &the_nodes[i][tid].link);
  }

  rtems_rbtree_initialize_empty(
      &the_rbtree[tid],
      &rbtree_compare,
      false
  );

#if 0
  rtems_rbtree_initialize_empty(
      &the_rbtree[tid],
      &test_rbtree_insert_int32,
      &test_rbtree_find_int32,
      RTEMS_RBTREE_DUPLICATE
  );
#endif
}

void rbtree_insert( rtems_task_argument tid,  long kv ) {
  node *n = alloc_node(tid);
  pq_node *pn = &n->data;
  pn->key = kv_key(kv);
  pn->val = kv_value(kv);
  rtems_rbtree_insert_unprotected( &the_rbtree[tid], &n->rbt_node );
#if defined(USE_RB_ASSERT)
  rb_assert(the_rbtree[tid].root);
#endif
}

long rbtree_min( rtems_task_argument tid ) {
  long kv;
  rtems_rbtree_node *rn;
  node *n;
  pq_node *p;

  rn = rtems_rbtree_min(&the_rbtree[tid]);

  if ( rn ) {
    n = rtems_rbtree_container_of(rn, node, rbt_node);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
    return kv;
  } 
  return (long)-1; // FIXME: error handling
}

long rbtree_pop_min( rtems_task_argument tid ) {
  long kv;
  rtems_rbtree_node *rn;
  node *n;
  pq_node *p;

  rn = rtems_rbtree_get_min_unprotected(&the_rbtree[tid]);

  if ( rn ) {
    n = rtems_rbtree_container_of(rn, node, rbt_node);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
    free_node(tid, n);
  } else {
    kv = (long)-1;
  }
#if defined(USE_RB_ASSERT)
  rb_assert(the_rbtree[tid].root);
#endif
  return kv;
}

long rbtree_search( rtems_task_argument tid, int k )
{
  rtems_rbtree_node *rn;
  node search_node;

  node *n;
  pq_node *p;
  long kv;

  search_node.data.key = k;

  rn = rtems_rbtree_find_unprotected(&the_rbtree[tid], &search_node.rbt_node);
  if ( rn ) {
    n = rtems_rbtree_container_of(rn, node, rbt_node);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
  } else {
    kv = (long)-1;
  }
  return kv;
}

long rbtree_extract( rtems_task_argument tid, int k )
{
  rtems_rbtree_node *rn;
  node search_node;

  node *n;
  pq_node *p;
  long kv;

  search_node.data.key = k;

  rn = rtems_rbtree_find_unprotected(&the_rbtree[tid], &search_node.rbt_node);
  if ( rn ) {
    rtems_rbtree_extract_unprotected(&the_rbtree[tid], rn);
    n = rtems_rbtree_container_of(rn, node, rbt_node);
    p = &n->data;
    kv = PQ_NODE_TO_KV(p);
    free_node(tid, n);
  } else {
    kv = (long)-1;
  }
#if defined(USE_RB_ASSERT)
  rb_assert(the_rbtree[tid].root);
#endif
  return kv;
}

