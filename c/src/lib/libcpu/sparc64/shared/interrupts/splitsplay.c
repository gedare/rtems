#include "spillpq.h"
#include "freelist.h"
#include "gabdebug.h"

#include "splitsplay.h"

static Freelist_Control free_nodes[NUM_QUEUES];

/* split pq: the splay */
static rtems_splay_tree trees[NUM_QUEUES];

typedef struct {
  union {
    Chain_Node Node;
    rtems_splay_tree_node st_node;
  };
  uint32_t key;
  uint32_t val;
} pq_node;

static int sparc64_splitsplay_compare(
  rtems_splay_tree_node* n1,
  rtems_splay_tree_node* n2
) {
  int key1 = _Container_of( n1, pq_node, st_node )->key;
  int key2 = _Container_of( n2, pq_node, st_node )->key;

  return key1 - key2;
}

/*
 *
 *  sptree.c:  The following code implements the basic operations on
 *  an event-set or priority-queue implemented using splay trees:
 *
 *  rtems_splay_tree *spinit( compare )	Make a new tree
 *  int spempty();		Is tree empty?
 *  rtems_splay_tree_node *spenq( n, q )	Insert n in q after all equal keys.
 *  rtems_splay_tree_node *spdeq( np )		Return first key under *np, removing it.
 *  rtems_splay_tree_node *spenqprior( n, q )	Insert n in q before all equal keys.
 *  void splay( n, q )		n (already in q) becomes the root.
 *
 *  In the above, n points to an rtems_splay_tree_node type, while q points to an
 *  rtems_splay_tree.
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

static int spempty( rtems_splay_tree *q )
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
static rtems_splay_tree_node * spenq( rtems_splay_tree_node *n, rtems_splay_tree *q )
{
  rtems_splay_tree_node * left;  /* the rightmost node in the left tree */
  rtems_splay_tree_node * right;  /* the leftmost node in the right tree */
  rtems_splay_tree_node * next;  /* the root of the unsplit part */
  rtems_splay_tree_node * temp;

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
//  key = n->key;
  left = n;
  right = n;

  /* n's left and right children will hold the right and left
     splayed trees resulting from splitting on n->key;
     note that the children will be reversed! */

  q->enqcmps++;

  /* figure out which side to start on */
  if ( sparc64_splitsplay_compare(next, n) > 0 )
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
    if( sparc64_splitsplay_compare(temp, n) > 0 )
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

  } while( sparc64_splitsplay_compare(next,n) <= 0 );  /* change sides */

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
    if( sparc64_splitsplay_compare(temp, n) <= 0 )
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

  } while( sparc64_splitsplay_compare(next, n) > 0 );  /* change sides */

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
static rtems_splay_tree_node *spdeq( rtems_splay_tree_node **np )
{
  rtems_splay_tree_node * deq;    /* one to return */
  rtems_splay_tree_node * next;   /* the next thing to deal with */
  rtems_splay_tree_node * left;  /* the left child of next */
  rtems_splay_tree_node * farleft;    /* the left child of left */
  rtems_splay_tree_node * farfarleft;  /* the left child of farleft */

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
static rtems_splay_tree_node *spenqprior( rtems_splay_tree_node *n, rtems_splay_tree *q )
{
  rtems_splay_tree_node * left;  /* the rightmost node in the left tree */
  rtems_splay_tree_node * right;  /* the leftmost node in the right tree */
  rtems_splay_tree_node * next;  /* the root of unsplit part of tree */
  rtems_splay_tree_node * temp;
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
  //key = n->key;
  left = n;
  right = n;

  /* n's left and right children will hold the right and left
     splayed trees resulting from splitting on n->key;
     note that the children will be reversed! */

  if( sparc64_splitsplay_compare(next, n) >= 0 )
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
    if( sparc64_splitsplay_compare(temp, n) >= 0 )
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

  } while( sparc64_splitsplay_compare(next, n) < 0 );  /* change sides */

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
    if( sparc64_splitsplay_compare(temp, n) < 0 )
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

  } while( sparc64_splitsplay_compare(next, n) >= 0 );  /* change sides */

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

static void splay( rtems_splay_tree_node *n, rtems_splay_tree *q )
{
  rtems_splay_tree_node * up;  /* points to the node being dealt with */
  rtems_splay_tree_node * prev;  /* a descendent of up, already dealt with */
  rtems_splay_tree_node * upup;  /* the parent of up */
  rtems_splay_tree_node * upupup;  /* the grandparent of up */
  rtems_splay_tree_node * left;  /* the top of left subtree being built */
  rtems_splay_tree_node * right;  /* the top of right subtree being built */

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
    else  /* up is to the left of n */
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

/**
 *  spfind
 *
 *  Searches for a particular key in the tree returning the first one found.
 *  If a node is found splay it and return it. Returns NULL if none is found.
 */
static rtems_splay_tree_node *spfind(rtems_splay_tree *tree, rtems_splay_tree_node *sn)
{
  rtems_splay_tree_node *iter = tree->root;
  while ( iter ) {
    if(sparc64_splitsplay_compare(iter, sn) > 0) {
      iter = iter->leftlink;
    }
    else if(sparc64_splitsplay_compare(iter, sn) < 0) {
      iter = iter->rightlink;
    }
    else {
      splay(iter, tree);
      break;
    }
  }
  return iter;
}

/**
 * spget_successor
 *
 * Find the in-order successor of n. FIXME: only works in descent..
 */
static rtems_splay_tree_node* spget_successor( rtems_splay_tree_node *n ) {
  n = n->rightlink;
  while ( n->leftlink ) {
    n = n->leftlink;
  }
  return n;
}

/**
 *  spdelete
 *
 *  Searches for and removes a particular key in the tree.
 *  If a node is pruned then return it. Return NULL if key is not found.
 */
static rtems_splay_tree_node* spdelete(rtems_splay_tree *tree, rtems_splay_tree_node *sn)
{
  rtems_splay_tree_node *node = spfind(tree, sn);
  rtems_splay_tree_node *new_root = NULL;
  rtems_splay_tree_node *left;
  rtems_splay_tree_node *right;

  if ( node ) {
    /* because of splaying in spfind() node is the root */
    if ( node->leftlink == NULL ) {
      /* replace with right child if it exists */
      right = node->rightlink;
      if ( right ) {
        right->uplink = NULL;
      }
      tree->root = right;
    } else if ( node->rightlink == NULL ) {
      /* replace with left child, which exists */
      left = node->leftlink;
      left->uplink = NULL;
      tree->root = left;
    } else {
      /* replace node with its successor */
      rtems_splay_tree_node *suc = spget_successor(node);
      right = node->rightlink;
      left = node->leftlink;
      if ( suc != node->rightlink ) {
        new_root = suc->uplink;
        if ( suc->rightlink ) {
          suc->rightlink->uplink = new_root;
          new_root->leftlink = suc->rightlink;
        } else {
          new_root->leftlink = NULL;
        }
        suc->uplink = node->uplink;
        node->rightlink->uplink = suc;
        node->leftlink->uplink = suc;
        suc->rightlink = node->rightlink;
        suc->leftlink = node->leftlink;
      } else { /* successor is the right child */
        new_root = suc;
        suc->uplink = node->uplink;
        node->leftlink->uplink = suc;
        suc->leftlink = node->leftlink;
      }
      tree->root = suc;
      splay(new_root, tree);
    }
  }
  return node;
}


int sparc64_splitsplay_initialize( int tid, size_t max_pq_size )
{
  int i;
  uint64_t reg = 0;
  freelist_initialize(&free_nodes[tid], sizeof(pq_node), max_pq_size);


  _Splay_Initialize_empty(&trees[tid], &sparc64_splitsplay_compare);
  spillpq_queue_max_size[tid] = max_pq_size;

  return 0;
}

uint64_t sparc64_splitsplay_insert(int tid, uint64_t kv)
{
  pq_node *new_node;
  new_node = freelist_get_node(&free_nodes[tid]);
  if (!new_node) {
    printk("%d\tUnable to allocate new node during insert\n", tid);
    while (1);
  }
  new_node->key = kv_key(kv);
  new_node->val = kv_value(kv); // FIXME: not full 64-bits

  spenq( &new_node->st_node, &trees[tid] );
  //rtems_splay_insert_unprotected( &trees[tid], &new_node->rbt_node );
  return 0;
}

uint64_t sparc64_splitsplay_first(int tid, uint64_t kv)
{
  pq_node *p;
  rtems_splay_tree_node *first;
 
  first = spdeq( &trees[tid].root ); // FIXME: get without remove or side-fx
  if ( first ) {
    first->rightlink = trees[tid].root;
    first->leftlink = first->uplink = NULL;
    if ( trees[tid].root )
      trees[tid].root->uplink = first;
  }

//  first = rtems_splay_min(&trees[tid]);
  if ( first ) {
    p = _Container_of(first, pq_node, st_node);
    kv = pq_node_to_kv(p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t sparc64_splitsplay_pop(int tid, uint64_t kv)
{
  rtems_splay_tree_node *first;
  pq_node *p;
  first = spdeq( &trees[tid].root );
//  first = rtems_splay_get_min_unprotected(&trees[tid]);
  if ( first ) {
    p = _Container_of(first, pq_node, st_node);
    kv = pq_node_to_kv(p);
    freelist_put_node(&free_nodes[tid], p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

static rtems_splay_tree_node* search_helper(int tid, uint64_t kv)
{
  rtems_splay_tree_node *n;
  rtems_splay_tree *tree;
  pq_node search_node;
  search_node.key = kv_key(kv);
  tree = &trees[tid];
  n = spfind(tree, &search_node);

  return n;
}

uint64_t sparc64_splitsplay_extract(int tid, uint64_t kv )
{
  rtems_splay_tree_node *n;
  rtems_splay_tree *tree;
  pq_node *p;
  pq_node search_node;

  search_node.key = kv_key(kv);

  tree = &trees[tid];
  n = spdelete(tree, &search_node);

  if ( n ) {
    p = _Container_of(n, pq_node, st_node);
    kv = pq_node_to_kv(p);
    freelist_put_node(&free_nodes[tid], p);
  } else {
    DPRINTK("%d: Failed extract: %d\t%X\n", tid, kv_key(kv), kv_value(kv));
    kv = (uint64_t)-1;
  }

  return kv;
}

uint64_t sparc64_splitsplay_search(int tid, uint64_t kv )
{
  rtems_splay_tree_node *n;
  pq_node *p;
  n = search_helper(tid, kv);

  if ( n ) {
    p = _Container_of(n, pq_node, st_node);
    kv = pq_node_to_kv(p);
  } else {
    kv = (uint64_t)-1;
  }
  return kv;
}

static inline uint64_t 
sparc64_splitsplay_spill_node(int tid)
{
  uint64_t kv;

  HWDS_SPILL(tid, kv);
  if (!kv) {
    DPRINTK("%d\tNothing to spill!\n", tid);
  } else {
    sparc64_splitsplay_insert(tid, kv);
  }

  return kv;
}

uint64_t sparc64_splitsplay_handle_spill( int tid, uint64_t count )
{
  int i = 0;

  // pop elements off tail of hwpq, merge into software pq
  while ( i < count ) {
    if (!sparc64_splitsplay_spill_node(tid))
      break;
    i++;
  }

  return i;
}

static inline uint64_t
sparc64_splitsplay_fill_node(int tid, int count)
{
  uint32_t exception;
  uint64_t kv;

  kv = sparc64_splitsplay_pop(tid, 0);

  // add node to hwpq
  HWDS_FILL(tid, kv_key(kv), kv_value(kv), exception); 

  if (exception) {
    DPRINTK("Spilling (%d,%X) while filling\n");
    return sparc64_splitsplay_handle_spill(tid, count);
  }

  return 0;
}

/*
 * Current algorithm pulls nodes from the head of the sorted sw pq
 * and fills them into the hw pq.
 */
uint64_t sparc64_splitsplay_handle_fill(int tid, uint64_t count )
{
  int i = 0;

  while (!_Splay_is_empty( &trees[tid] ) && i < count) {
    i++;
    sparc64_splitsplay_fill_node(tid, count);
  }

  return 0;
}

uint64_t sparc64_splitsplay_drain( int tid, uint64_t ignored )
{
  return 0;
}

sparc64_spillpq_operations sparc64_splitsplay_ops = {
  sparc64_splitsplay_initialize,
  sparc64_splitsplay_insert,
  sparc64_splitsplay_first,
  sparc64_splitsplay_pop,
  sparc64_splitsplay_extract,
  sparc64_splitsplay_search,
  sparc64_splitsplay_handle_spill,
  sparc64_splitsplay_handle_fill,
  sparc64_splitsplay_drain
};

