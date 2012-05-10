/**
 * @file
 *
 * @ingroup ScoreRBTree
 *
 * @brief _RBTree_Find_finger, _RBTree_Find_unprotected, and _RBTree_Find.
 */

/*
 *  Copyright (c) 2010-2012 Gedare Bloom.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/system.h>
#include <rtems/score/address.h>
#include <rtems/score/rbtree.h>
#include <rtems/score/isr.h>

RBTree_Node *_RBTree_Find_finger(
  RBTree_Control *the_rbtree,
  RBTree_Node *the_node,
  RBTree_Node *finger
)
{
  RBTree_Node* iter_node;
  RBTree_Node* found = NULL;
  RBTree_Direction dir;
  int compare_result;

  /* first make sure the finger is good. this can be cheaper if a finger
   * path is kept to the root instead of reconstructing it here. */
  iter_node = _RBTree_Common_ancestor(the_rbtree, the_node, finger);

  while ( iter_node ) {
    compare_result = the_rbtree->compare_function( the_node, iter_node );
    if ( _RBTree_Is_equal( compare_result ) ) {
      found = iter_node;
      if ( !_RBTree_Is_stable(the_rbtree) )
        break;
    }

    dir = (RBTree_Direction)_RBTree_Is_greater( compare_result );
    iter_node = iter_node->child[dir];
  }

  return found;
}

RBTree_Node *_RBTree_Find_unprotected(
  RBTree_Control *the_rbtree,
  RBTree_Node *the_node
)
{
  RBTree_Node* root = the_rbtree->root;
  return _RBTree_Find_finger(the_rbtree, the_node, root);
}

RBTree_Node *_RBTree_Find(
  RBTree_Control *the_rbtree,
  RBTree_Node *search_node
)
{
  ISR_Level          level;
  RBTree_Node *return_node;

  return_node = NULL;
  _ISR_Disable( level );
  return_node = _RBTree_Find_unprotected( the_rbtree, search_node );
  _ISR_Enable( level );
  return return_node;
}
