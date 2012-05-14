/*
 *  Copyright (c) 2012 Gedare Bloom.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#include <rtems/system.h>
#include <rtems/score/splay.h>

void _Splay_Initialize_empty(
  Splay_Control *the_tree,
  Splay_Compare_function compare_function
)
{
  the_tree->enqs = 0;
  the_tree->enqcmps = 0;
  the_tree->splays = 0;
  the_tree->splayloops = 0;
  the_tree->root = NULL;
  the_tree->first[0] = the_tree->first[1] = NULL;
  the_tree->compare_function = compare_function;
  the_tree->permanent_null = NULL;
}

Splay_Node *_Splay_Successor( Splay_Node *the_node )
{
  return _RBTree_Next_unprotected(the_node, TREE_RIGHT);
}

// FIXME: copied from RBTree. share it
static void _Splay_Rotate(
    Splay_Node *the_node,
    Splay_Direction dir
    )
{
  Splay_Node *c;
  if (the_node == NULL) return;
  if (the_node->child[!dir] == NULL) return;

  c = the_node->child[!dir];
  the_node->child[!dir] = c->child[dir];

  if (c->child[dir])
    c->child[dir]->parent = the_node;

  c->child[dir] = the_node;

  the_node->parent->child[the_node != the_node->parent->child[0]] = c;

  c->parent = the_node->parent;
  the_node->parent = c;
}

/*
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
 * Thu Oct  6 12:11:33 PDT 1988 (daveb) Fixed _Splay_Dequeue, which was broken
 *	handling one-node trees.  I botched the pascal translation of
 *	a VAR parameter.
 */

bool _Splay_Is_empty( Splay_Control *the_tree )
{
  return the_tree == NULL || the_tree->root == NULL;
}

/*----------------
 *
 *  _Splay_Insert() -- insert item in a tree.
 *
 *  put the_node in the_tree after all other nodes with the same key; when this is
 *  done, the_node will be the root of the splay tree representing the_tree, all nodes
 *  in the_tree with keys less than or equal to that of the_node will be in the
 *  left subtree, all with greater keys will be in the right subtree;
 *  the tree is split into these subtrees from the top down, with rotations
 *  performed along the way to shorten the left branch of the right subtree
 *  and the right branch of the left subtree
 */
Splay_Node * _Splay_Insert( Splay_Control *the_tree, Splay_Node *the_node )
{
  Splay_Node * next;  /* the root of the unsplit part */
  Splay_Node * temp;
  Splay_Node *split[2];
  Splay_Direction dir, opp_dir;
  int compare_result;

  the_tree->enqs++;
  the_node->parent = &the_tree->permanent_null;
  next = the_tree->root;
  the_tree->root = the_node;
  
  if( next == NULL )  /* trivial enq */
  {
    the_node->child[TREE_LEFT] = NULL;
    the_node->child[TREE_RIGHT] = NULL;
    the_tree->first[TREE_LEFT] = the_tree->first[TREE_RIGHT] = the_node;
    return the_node;
  }
  
  /* difficult enq */
  split[0] = the_node;
  split[1] = the_node;

  /* the_node's left and right children will hold the right and left
     splayed trees resulting from splitting on the_node->key;
     note that the children will be reversed! */

  the_tree->enqcmps++;

  while ( 1 ) {
    compare_result = the_tree->compare_function(next, the_node);
    dir = (compare_result > 0); // use >= for prepending tied nodes
    opp_dir = !dir;

    temp = next->child[opp_dir];
    if ( temp == NULL ) {
      split[dir]->child[opp_dir] = next;
      next->parent = split[dir];
      split[opp_dir]->child[dir] = NULL;
      break;
    }
    the_tree->enqcmps++;
    compare_result = the_tree->compare_function(temp, the_node);
    if ( (compare_result > 0) != dir ) {
      // switch sides
      split[dir]->child[opp_dir] = next;
      next->parent = split[dir];
      split[dir] = next;
      next = temp;
      continue;
    }
    next->child[opp_dir] = temp->child[dir];
    if ( temp->child[dir] != NULL ) {
      temp->child[dir]->parent = next;
    }
    split[dir]->child[opp_dir] = temp;
    temp->parent = split[dir];
    temp->child[dir] = next;
    next->parent = temp;
    split[dir] = temp;
    next = temp->child[opp_dir];
    if ( next == NULL ) {
      split[opp_dir]->child[dir] = NULL;
      break;
    }
    the_tree->enqcmps++;
  }

  // swap the_node->left with the_node->right
  temp = the_node->child[dir];
  the_node->child[dir] = the_node->child[opp_dir];
  the_node->child[opp_dir] = temp;

  // update min/max
  if ( !the_node->child[dir] ) {
    the_tree->first[dir] = the_node;
  }
  if ( !the_node->child[opp_dir] ) {
    the_tree->first[opp_dir] = the_node;
  }
  return the_node;
} /* _Splay_Insert */

/*----------------
 *
 *  _Splay_Dequeue() -- return and remove head node from a subtree.
 *
 *  remove and return the head node from the node set; this deletes
 *  (and returns) the leftmost node from the_tree, replacing it with its right
 *  subtree (if there is one); on the way to the leftmost node, rotations
 *  are performed to shorten the left branch of the tree
 */
// FIXME: add non-splaying version
Splay_Node *_Splay_Dequeue( Splay_Control *the_tree, Splay_Direction dir )
{
  Splay_Node * deq;
  Splay_Node * next;
  Splay_Node * c;
  Splay_Node * cc;
  Splay_Direction opp_dir;

  opp_dir = !dir;
  next = the_tree->root;

  if( next == NULL ) {
    return NULL;
  }

  // handle root as special case.
  c = next->child[dir];
  if( c == NULL ) {
    the_tree->first[dir] = _Splay_Successor(next);
    the_tree->root = next->child[opp_dir];

    if( the_tree->root != NULL )
      the_tree->root->parent = &the_tree->permanent_null;
    return next;
  }

  // loop invariants: next is not null, c is not null.
  while ( 1 ) {
    /* next is not it, c is not NULL, might be it */
    cc = c->child[dir];
    if( cc == NULL ) {
      deq = c;
      the_tree->first[dir] = _Splay_Successor(deq);
      next->child[dir] = c->child[opp_dir];
      if( c->child[opp_dir] != NULL )
        c->child[opp_dir]->parent = next;
      break;
    }

    /* next, c are not it, cc is not NULL, might be it */
    if( cc->child[dir] == NULL ) {
      deq = cc;
      the_tree->first[dir] = _Splay_Successor(deq);
      c->child[dir] = cc->child[opp_dir];
      if( cc->child[opp_dir] != NULL )
        cc->child[opp_dir]->parent = c;
      break;
    }

    /* next, c, cc are not it, rotate */
    _Splay_Rotate(c, opp_dir);
    next = cc;
    c = cc->child[dir];
  }

  return deq;
} /* _Splay_Dequeue */

/*----------------
 *
 *  _Splay_Splay() -- reorganize the tree.
 *
 *  the tree is reorganized so that the_node is the root of the
 *  splay tree representing the_tree; results are unpredictable if the_node is not
 *  in the_tree to start with; the_tree is split from the_node up to the old root, with all
 *  nodes to the left of the_node ending up in the left subtree, and all nodes
 *  to the right of the_node ending up in the right subtree; the left branch of
 *  the right subtree and the right branch of the left subtree are
 *  shortened in the process
 *
 *  this code assumes that the_node is not NULL and is in the_tree;
 *  it can sometimes detect the_node not in the_tree and complain
 */
void _Splay_Splay( Splay_Control *the_tree, Splay_Node *the_node )
{
  Splay_Node * up;  /* points to the node being dealt with */
  Splay_Node * prev;  /* a descendent of up, already dealt with */
  Splay_Node * upup;  /* the parent of up */
  Splay_Node * upupup;  /* the grandparent of up */
  Splay_Node *split[2]; /* left and right subtree roots */

  Splay_Direction dir;
  Splay_Direction opp_dir;

  split[TREE_LEFT] = the_node->child[TREE_LEFT];
  split[TREE_RIGHT] = the_node->child[TREE_RIGHT];
  up = the_node->parent;
  dir = the_node != up->child[0];

  the_tree->splays++;

  /* bottom-up splaying:
   * walk up the tree towards the root, splaying all to the left of
   * the_node into the left subtree, all to right into the right subtree */
  while( up->parent != NULL ) {
    the_tree->splayloops++;

    upup = up->parent;
    opp_dir = !dir;

    if ( upup->parent != NULL && upup->child[dir] == up ) {
      _Splay_Rotate(upup, opp_dir);
    }
    
    up->child[dir] = split[opp_dir];
    if( split[opp_dir] != NULL )
      split[opp_dir]->parent = up;
    split[opp_dir] = up;

    dir = up != up->parent->child[0];
    up = up->parent;
  }

  the_node->child[TREE_LEFT] = split[TREE_LEFT];
  the_node->child[TREE_RIGHT] = split[TREE_RIGHT];
  if ( split[TREE_LEFT] != NULL ) {
    split[TREE_LEFT]->parent = the_node;
  }
  if ( split[TREE_RIGHT] != NULL ) {
    split[TREE_RIGHT]->parent = the_node;
  }
  the_tree->root = the_node;
  the_node->parent = &the_tree->permanent_null;
} /* splay */
/* /sptree.c */

/**
 *  _Splay_Find
 *
 *  Searches for a particular key in the tree returning the first one found.
 *  If a node is found splay it and return it. Returns NULL if none is found.
 */
// FIXME: should splay when the node is not found? add non-splaying version
Splay_Node *_Splay_Find(Splay_Control *tree, Splay_Node *search_node)
{
  Splay_Node *iter = tree->root;
  while ( iter ) {
    if(tree->compare_function(iter, search_node) > 0) {
      iter = iter->child[TREE_LEFT];
    }
    else if(tree->compare_function(iter, search_node) < 0) {
      iter = iter->child[TREE_RIGHT];
    }
    else {
      _Splay_Splay(tree, iter);
      break;
    }
  }
  return iter;
}

/**
 *  _Splay_Extract
 *
 *  Searches for and removes a particular key in the tree.
 *  If a node is pruned then return it. Return NULL if key is not found.
 */
// FIXME: add non-splaying version
Splay_Node* _Splay_Extract(Splay_Control *tree, Splay_Node *the_node)
{
  Splay_Node *node = _Splay_Find(tree, the_node);
  Splay_Node *new_root = NULL;
  Splay_Node *left;
  Splay_Node *right;

  if ( node ) {
    /* because of splaying in _Splay_Find() node is the root */
    if ( node->child[TREE_LEFT] == NULL ) {
      /* replace with right child if it exists */
      right = node->child[TREE_RIGHT];
      if ( right ) {
        right->parent = &tree->permanent_null;
      }
      tree->root = right;
    } else if ( node->child[TREE_RIGHT] == NULL ) {
      /* replace with left child, which exists */
      left = node->child[TREE_LEFT];
      left->parent = &tree->permanent_null;
      tree->root = left;
    } else {
      /* replace node with its successor */
      Splay_Node *suc = _Splay_Successor(node);
      right = node->child[TREE_RIGHT];
      left = node->child[TREE_LEFT];
      if ( suc != node->child[TREE_RIGHT] ) {
        new_root = suc->parent;
        if ( suc->child[TREE_RIGHT] ) {
          suc->child[TREE_RIGHT]->parent = new_root;
          new_root->child[TREE_LEFT] = suc->child[TREE_RIGHT];
        } else {
          new_root->child[TREE_LEFT] = NULL;
        }
        suc->parent = node->parent;
        node->child[TREE_RIGHT]->parent = suc;
        node->child[TREE_LEFT]->parent = suc;
        suc->child[TREE_RIGHT] = node->child[TREE_RIGHT];
        suc->child[TREE_LEFT] = node->child[TREE_LEFT];
      } else { /* successor is the right child */
        new_root = suc;
        suc->parent = node->parent;
        node->child[TREE_LEFT]->parent = suc;
        suc->child[TREE_LEFT] = node->child[TREE_LEFT];
      }
      tree->root = suc;
      _Splay_Splay(tree, new_root);
    }
  }
  return node;
}

