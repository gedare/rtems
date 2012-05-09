/**
 * @file
 *
 * @ingroup ScoreRBTree
 *
 * @brief _RBTree_Insert_finger, _RBTree_Insert_unprotected, and _RBTree_Insert.
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

#include <rtems/score/rbtree.h>
#include <rtems/score/isr.h>

/*
 * Validate and fix-up tree properties for a newly inserted/colored node.
 *
 * Checks and fixes the red-black tree properties based on the_node being
 * just added to the tree as a leaf with the color red.
 *
 * Note: the insert root case is handled already so no special care is needed.
 */
static void _RBTree_Validate_insert_unprotected(
    RBTree_Node    *the_node
)
{
  RBTree_Node *u;
  RBTree_Node *g;

  /* if the parent is black, nothing needs to be done
   * otherwise may need to loop a few times */
  while ( _RBTree_Is_red( _RBTree_Parent( the_node ) ) ) {
    u = _RBTree_Parent_sibling( the_node );
    g = the_node->parent->parent;

    /* if uncle is red, repaint uncle/parent black and grandparent red */
    if( _RBTree_Is_red( u ) ) {
      _RBTree_Set_color(the_node->parent, RBT_BLACK);
      _RBTree_Set_color(u, RBT_BLACK);
      _RBTree_Set_color(g, RBT_RED);
      the_node = g;
    } else { /* if uncle is black */
      RBTree_Direction dir;
      RBTree_Direction pdir;
      dir = the_node != the_node->parent->child[0];
      pdir = the_node->parent != g->child[0];

      /* ensure node is on the same branch direction as parent */
      if ( dir != pdir ) {
        _RBTree_Rotate( the_node->parent, pdir );
        the_node = the_node->child[pdir];
      }
      _RBTree_Set_color(the_node->parent, RBT_BLACK);
      _RBTree_Set_color(g, RBT_RED);

      /* now rotate grandparent in the other branch direction (toward uncle) */
      _RBTree_Rotate( g, ( 1 - pdir ) );
    }
  }

  /* if the_node is now the root recolor it black */
  if ( !the_node->parent->parent ) {
    _RBTree_Set_color(the_node, RBT_BLACK);
  }
}

void _RBTree_Insert_finger(
  RBTree_Control *the_rbtree,
  RBTree_Node *the_node,
  RBTree_Node *finger
)
{
  int compare_result;
  RBTree_Direction dir;
  RBTree_Node* iter_node;

  /* first make sure the finger is good. this can be cheaper if a finger
   * path is kept to the root instead of reconstructing it here. */
  iter_node = _RBTree_Common_ancestor(the_rbtree, the_node, finger);

  /* descend tree to leaf and insert */
  while ( iter_node ) {
    compare_result = the_rbtree->compare_function( the_node, iter_node );
    dir = !_RBTree_Is_lesser( compare_result );
    if ( !iter_node->child[dir] ) {
      /* found insertion point: iter_node->child[dir] */
      the_node->child[RBT_LEFT] = the_node->child[RBT_RIGHT] = NULL;
      _RBTree_Set_color(the_node, RBT_RED);
      iter_node->child[dir] = the_node;
      the_node->parent = iter_node;

      /* update min/max */
      compare_result = the_rbtree->compare_function(
          the_node,
          _RBTree_First(the_rbtree, dir)
      );
      if ( ( !dir && _RBTree_Is_lesser( compare_result ) ) ||
           ( dir && _RBTree_Is_greater( compare_result ) ) ) {
        the_rbtree->first[dir] = the_node;
      }
      break;
    } else {
      iter_node = iter_node->child[dir];
    }
  } /* while(iter_node) */

  /* verify red-black properties */
  _RBTree_Validate_insert_unprotected(the_node);
}

RBTree_Node *_RBTree_Insert_unprotected(
  RBTree_Control *the_rbtree,
  RBTree_Node *the_node
)
{
  RBTree_Node *root_node;

  if ( !the_node ) {
    return NULL;
  }

  root_node = the_rbtree->root;

  if ( !root_node ) {
    /* special case: node inserted to empty tree */
    _RBTree_Set_color(the_node, RBT_BLACK);
    the_rbtree->root = the_node;
    the_rbtree->first[0] = the_rbtree->first[1] = the_node;
    the_node->parent = (RBTree_Node *) the_rbtree;
    the_node->child[RBT_LEFT] = the_node->child[RBT_RIGHT] = NULL;
  } else {
    _RBTree_Insert_finger( the_rbtree, the_node, root_node );
  }
  return the_node;
}

RBTree_Node *_RBTree_Insert(
  RBTree_Control *tree,
  RBTree_Node *node
)
{
  ISR_Level level;
  RBTree_Node *return_node;

  _ISR_Disable( level );
  return_node = _RBTree_Insert_unprotected( tree, node );
  _ISR_Enable( level );
  return return_node;
}
