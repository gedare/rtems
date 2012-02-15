
#include "msplay.h"

#include <stdlib.h>
#include <tmacros.h>

#if defined(GAB_DEBUG)
#include <assert.h>
#else
#define assert(x) { ; }
#endif

static splay_tree the_tree[NUM_APERIODIC_TASKS];

node the_nodes[NUM_NODES][NUM_APERIODIC_TASKS];
rtems_chain_control freelist[NUM_APERIODIC_TASKS];

node *alloc_node(rtems_task_argument tid) {
  node *n = rtems_chain_get_unprotected( &freelist[tid] );
  return n;
}
void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freelist[tid], n );
}

/*
 *
 *  sptree.c:  The following code implements the basic operations on
 *  an event-set or priority-queue implemented using splay trees:
 *
 *  splay_tree *spinit( compare )	Make a new tree
 *  int spempty();		Is tree empty?
 *  splay_tree_node *spenq( n, q )	Insert n in q after all equal keys.
 *  splay_tree_node *spdeq( np )		Return first key under *np, removing it.
 *  splay_tree_node *spenqprior( n, q )	Insert n in q before all equal keys.
 *  void splay( n, q )		n (already in q) becomes the root.
 *
 *  In the above, n points to an splay_tree_node type, while q points to an
 *  splay_tree.
 *
 *  The implementation used here is based on the implementation
 *  which was used in the tests of splay trees reported in:
 *
 *    An Empirical Comparison of Priority-Queue and Event-Set Implementations,
 *	by Douglas W. Jones, Comm. ACM 29, 4 (Apr. 1986) 300-311.
 *
 *  The changes made include the addition of the enqprior
 *  operation and the addition of up-links to allow for the splay
 *  operation.  The basic splay tree algorithms were originally
 *  presented in:
 *
 *	Self Adjusting Binary Trees,
 *		by D. D. Sleator and R. E. Tarjan,
 *			Proc. ACM SIGACT Symposium on Theory
 *			of Computing (Boston, Apr 1983) 235-245.
 *
 *  The enq and enqprior routines use variations on the
 *  top-down splay operation, while the splay routine is bottom-up.
 *  All are coded for speed.
 *
 *  Written by:
 *    Douglas W. Jones
 *
 *  Translated to C by:
 *    David Brower, daveb@rtech.uucp
 *
 * Thu Oct  6 12:11:33 PDT 1988 (daveb) Fixed spdeq, which was broken
 *	handling one-node trees.  I botched the pascal translation of
 *	a VAR parameter.
 */

static int spempty( splay_tree *q )
{
  return q == NULL || q->root == NULL;
}

/*----------------
 *
 *  spenq() -- insert item in a tree.
 *
 *  put n in q after all other nodes with the same key; when this is
 *  done, n will be the root of the splay tree representing q, all nodes
 *  in q with keys less than or equal to that of n will be in the
 *  left subtree, all with greater keys will be in the right subtree;
 *  the tree is split into these subtrees from the top down, with rotations
 *  performed along the way to shorten the left branch of the right subtree
 *  and the right branch of the left subtree
 */
static splay_tree_node * spenq( splay_tree_node *n, splay_tree *q )
{
  splay_tree_node * left;  /* the rightmost node in the left tree */
  splay_tree_node * right;  /* the leftmost node in the right tree */
  splay_tree_node * next;  /* the root of the unsplit part */
  splay_tree_node * temp;

  int key;

  q->enqs++;
  n->uplink = NULL;
  next = q->root;
  q->root = n;
  
  if( next == NULL )  /* trivial enq */
  {
    n->leftlink = NULL;
    n->rightlink = NULL;
    n->cnt++;
    return n;
  }
  
  /* difficult enq */
  key = n->key;
  left = n;
  right = n;

  /* n's left and right children will hold the right and left
     splayed trees resulting from splitting on n->key;
     note that the children will be reversed! */

  q->enqcmps++;

  /* figure out which side to start on */
  if ( next->key > key )
    goto two;

one:  /* assert next->key <= key */

  do  /* walk to the right in the left tree */
  {
    temp = next->rightlink;
    if( temp == NULL )
    {
      left->rightlink = next;
      next->uplink = left;
      right->leftlink = NULL;
      goto done;  /* job done, entire tree split */
    }

    q->enqcmps++;
    if( temp->key > key )
    {
      left->rightlink = next;
      next->uplink = left;
      left = next;
      next = temp;
      goto two;  /* change sides */
    }

    next->rightlink = temp->leftlink;
    if( temp->leftlink != NULL )
      temp->leftlink->uplink = next;
    left->rightlink = temp;
    temp->uplink = left;
    temp->leftlink = next;
    next->uplink = temp;
    left = temp;
    next = temp->rightlink;
    if( next == NULL )
    {
      right->leftlink = NULL;
      goto done;  /* job done, entire tree split */
    }

    q->enqcmps++;

  } while( next->key <= key );  /* change sides */

two:  /* assert next->key > key */

  do  /* walk to the left in the right tree */
  {
    temp = next->leftlink;
    if( temp == NULL )
    {
      right->leftlink = next;
      next->uplink = right;
      left->rightlink = NULL;
      goto done;  /* job done, entire tree split */
    }

    q->enqcmps++;
    if( temp->key <= key )
    {
      right->leftlink = next;
      next->uplink = right;
      right = next;
      next = temp;
      goto one;  /* change sides */
    }
    next->leftlink = temp->rightlink;
    if( temp->rightlink != NULL )
      temp->rightlink->uplink = next;
    right->leftlink = temp;
    temp->uplink = right;
    temp->rightlink = next;
    next->uplink = temp;
    right = temp;
    next = temp->leftlink;
    if( next == NULL )
    {
      left->rightlink = NULL;
      goto done;  /* job done, entire tree split */
    }

    q->enqcmps++;

  } while( next->key > key );  /* change sides */

  goto one;

done:  /* split is done, branches of n need reversal */

  temp = n->leftlink;
  n->leftlink = n->rightlink;
  n->rightlink = temp;

  n->cnt++;
  return( n );

} /* spenq */


/*----------------
 *
 *  spdeq() -- return and remove head node from a subtree.
 *
 *  remove and return the head node from the node set; this deletes
 *  (and returns) the leftmost node from q, replacing it with its right
 *  subtree (if there is one); on the way to the leftmost node, rotations
 *  are performed to shorten the left branch of the tree
 */
static splay_tree_node *spdeq( splay_tree_node **np )
{
  splay_tree_node * deq;    /* one to return */
  splay_tree_node * next;         /* the next thing to deal with */
  splay_tree_node * left;        /* the left child of next */
  splay_tree_node * farleft;    /* the left child of left */
  splay_tree_node * farfarleft;  /* the left child of farleft */

  if( np == NULL || *np == NULL )
  {
    deq = NULL;
  }
  else
  {
    next = *np;
    left = next->leftlink;
    if( left == NULL )
    {
      deq = next;
      *np = next->rightlink;

      if( *np != NULL )
        (*np)->uplink = NULL;

    }
    else for(;;)  /* left is not null */
    {
      /* next is not it, left is not NULL, might be it */
      farleft = left->leftlink;
      if( farleft == NULL )
      {
        deq = left;
        next->leftlink = left->rightlink;
        if( left->rightlink != NULL )
          left->rightlink->uplink = next;
        break;
      }

      /* next, left are not it, farleft is not NULL, might be it */
      farfarleft = farleft->leftlink;
      if( farfarleft == NULL )
      {
        deq = farleft;
        left->leftlink = farleft->rightlink;
        if( farleft->rightlink != NULL )
          farleft->rightlink->uplink = left;
        break;
      }

      /* next, left, farleft are not it, rotate */
      next->leftlink = farleft;
      farleft->uplink = next;
      left->leftlink = farleft->rightlink;
      if( farleft->rightlink != NULL )
        farleft->rightlink->uplink = left;
      farleft->rightlink = left;
      left->uplink = farleft;
      next = farleft;
      left = farfarleft;
    }
  }

  return( deq );

} /* spdeq */


/*----------------
 *
 *  spenqprior() -- insert into tree before other equal keys.
 *
 *  put n in q before all other nodes with the same key; after this is
 *  done, n will be the root of the splay tree representing q, all nodes in
 *  q with keys less than that of n will be in the left subtree, all with
 *  greater or equal keys will be in the right subtree; the tree is split
 *  into these subtrees from the top down, with rotations performed along
 *  the way to shorten the left branch of the right subtree and the right
 *  branch of the left subtree; the logic of spenqprior is exactly the
 *  same as that of spenq except for a substitution of comparison
 *  operators
 */
static splay_tree_node *spenqprior( splay_tree_node *n, splay_tree *q )
{
  splay_tree_node * left;  /* the rightmost node in the left tree */
  splay_tree_node * right;  /* the leftmost node in the right tree */
  splay_tree_node * next;  /* the root of unsplit part of tree */
  splay_tree_node * temp;
  int key;

  n->uplink = NULL;
  next = q->root;
  q->root = n;
  if( next == NULL )  /* trivial enq */
  {
    n->leftlink = NULL;
    n->rightlink = NULL;
    return n;
  }
  /* difficult enq */
  key = n->key;
  left = n;
  right = n;

  /* n's left and right children will hold the right and left
     splayed trees resulting from splitting on n->key;
     note that the children will be reversed! */

  if( next->key >= key )
    goto two;

one:  /* assert next->key < key */

  do  /* walk to the right in the left tree */
  {
    temp = next->rightlink;
    if( temp == NULL )
    {
      left->rightlink = next;
      next->uplink = left;
      right->leftlink = NULL;
      goto done;  /* job done, entire tree split */
    }
    if( temp->key >= key )
    {
      left->rightlink = next;
      next->uplink = left;
      left = next;
      next = temp;
      goto two;  /* change sides */
    }
    next->rightlink = temp->leftlink;
    if( temp->leftlink != NULL )
      temp->leftlink->uplink = next;
    left->rightlink = temp;
    temp->uplink = left;
    temp->leftlink = next;
    next->uplink = temp;
    left = temp;
    next = temp->rightlink;
    if( next == NULL )
    {
      right->leftlink = NULL;
      goto done;  /* job done, entire tree split */
    }

  } while( next->key < key );  /* change sides */

two:  /* assert next->key >= key */

  do   /* walk to the left in the right tree */
  {
    temp = next->leftlink;
    if( temp == NULL )
    {
      right->leftlink = next;
      next->uplink = right;
      left->rightlink = NULL;
      goto done;  /* job done, entire tree split */
    }
    if( temp->key < key )
    {
      right->leftlink = next;
      next->uplink = right;
      right = next;
      next = temp;
      goto one;  /* change sides */
    }
    next->leftlink = temp->rightlink;
    if( temp->rightlink != NULL )
      temp->rightlink->uplink = next;
    right->leftlink = temp;
    temp->uplink = right;
    temp->rightlink = next;
    next->uplink = temp;
    right = temp;
    next = temp->leftlink;
    if( next == NULL )
    {
      left->rightlink = NULL;
      goto done;  /* job done, entire tree split */
    }

  } while( next->key >= key );  /* change sides */

  goto one;

done:  /* split is done, branches of n need reversal */

  temp = n->leftlink;
  n->leftlink = n->rightlink;
  n->rightlink = temp;

  return( n );

} /* spenqprior */

/*----------------
 *
 *  splay() -- reorganize the tree.
 *
 *  the tree is reorganized so that n is the root of the
 *  splay tree representing q; results are unpredictable if n is not
 *  in q to start with; q is split from n up to the old root, with all
 *  nodes to the left of n ending up in the left subtree, and all nodes
 *  to the right of n ending up in the right subtree; the left branch of
 *  the right subtree and the right branch of the left subtree are
 *  shortened in the process
 *
 *  this code assumes that n is not NULL and is in q; it can sometimes
 *  detect n not in q and complain
 */

static void splay( splay_tree_node *n, splay_tree *q )
{
  splay_tree_node * up;  /* points to the node being dealt with */
  splay_tree_node * prev;  /* a descendent of up, already dealt with */
  splay_tree_node * upup;  /* the parent of up */
  splay_tree_node * upupup;  /* the grandparent of up */
  splay_tree_node * left;  /* the top of left subtree being built */
  splay_tree_node * right;  /* the top of right subtree being built */

  n->cnt++;  /* bump reference count */

  left = n->leftlink;
  right = n->rightlink;
  prev = n;
  up = prev->uplink;

  q->splays++;

  while( up != NULL )
  {
    q->splayloops++;

    /* walk up the tree towards the root, splaying all to the left of
       n into the left subtree, all to right into the right subtree */

    upup = up->uplink;
    if( up->leftlink == prev )  /* up is to the right of n */
    {
      if( upup != NULL && upup->leftlink == up )  /* rotate */
      {
        upupup = upup->uplink;
        upup->leftlink = up->rightlink;
        if( upup->leftlink != NULL )
          upup->leftlink->uplink = upup;
        up->rightlink = upup;
        upup->uplink = up;
        if( upupup == NULL )
          q->root = up;
        else if( upupup->leftlink == upup )
          upupup->leftlink = up;
        else
          upupup->rightlink = up;
        up->uplink = upupup;
        upup = upupup;
      }
      up->leftlink = right;
      if( right != NULL )
        right->uplink = up;
      right = up;

    }
    else        /* up is to the left of n */
    {
      if( upup != NULL && upup->rightlink == up )  /* rotate */
      {
        upupup = upup->uplink;
        upup->rightlink = up->leftlink;
        if( upup->rightlink != NULL )
          upup->rightlink->uplink = upup;
        up->leftlink = upup;
        upup->uplink = up;
        if( upupup == NULL )
          q->root = up;
        else if( upupup->rightlink == upup )
          upupup->rightlink = up;
        else
          upupup->leftlink = up;
        up->uplink = upupup;
        upup = upupup;
      }
      up->rightlink = left;
      if( left != NULL )
        left->uplink = up;
      left = up;
    }
    prev = up;
    up = upup;
  }

# ifdef DEBUG
  if( q->root != prev )
  {
    /*  fprintf(stderr, " *** bug in splay: n not in q *** " ); */
    abort();
  }
# endif

  n->leftlink = left;
  n->rightlink = right;
  if( left != NULL )
    left->uplink = n;
  if( right != NULL )
    right->uplink = n;
  q->root = n;
  n->uplink = NULL;

} /* splay */
/* /sptree.c */

void splay_initialize(rtems_task_argument tid, int size ) {
  int i;

  rtems_chain_initialize_empty ( &freelist[tid] );
  for ( i = 0; i < size; i++ ) {
    rtems_chain_append(&freelist[tid], &the_nodes[i][tid].link);
  }

  the_tree[tid].lookups = 0;
  the_tree[tid].lkpcmps = 0;
  the_tree[tid].enqs = 0;
  the_tree[tid].enqcmps = 0;
  the_tree[tid].splays = 0;
  the_tree[tid].splayloops = 0;
  the_tree[tid].root = NULL;
}

void splay_insert( rtems_task_argument tid,uint64_t kv ) {
  node *n = alloc_node(tid);
  pq_node *pn = &n->data;
  pn->key = kv_key(kv);
  pn->val = kv_value(kv);
  n->st_node.key = pn->key;
  spenq( &n->st_node, &the_tree[tid] );
}

uint64_t splay_min(rtems_task_argument tid ) {
  uint64_t kv;
  splay_tree_node *stn;
  node *n;
  pq_node *p;

  stn = spdeq( &the_tree[tid].root );

  if ( stn ) {
    stn->rightlink = the_tree[tid].root;
    stn->leftlink = NULL;
    stn->uplink = NULL;
    if ( the_tree[tid].root )
      the_tree[tid].root->uplink = stn;
  }
  the_tree[tid].root = stn;

  if ( stn ) {
    n = ST_NODE_TO_NODE(stn);
    p = &n->data;
    assert(p->key == stn->key);
    kv = PQ_NODE_TO_KV(p);
    return kv;
  } 
  return (uint64_t)-1; // FIXME: error handling
}

uint64_t splay_pop_min( rtems_task_argument tid) {
  uint64_t kv;
  node *n;
  pq_node *p;
  splay_tree_node *stn;

  stn = spdeq( &the_tree[tid].root ); // TODO: use O(1) dequeue

  if ( stn ) {
    n = ST_NODE_TO_NODE(stn);
    p = &n->data;
    assert(p->key == stn->key);
    kv = PQ_NODE_TO_KV(p);
    free_node(tid,n);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}
