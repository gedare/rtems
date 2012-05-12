/*
 *  Copyright (c) 2010 Gedare Bloom.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/system.h>
#include <rtems/score/address.h>
#include <rtems/score/rbtree.h>
#include <rtems/score/isr.h>

/** @brief  Validate and fix-up tree properties after deleting a node
 *
 *  This routine is called on a black node, @a the_node, after its deletion.
 *  This function maintains the properties of the red-black tree.
 *
 *  @note It does NOT disable interrupts to ensure the atomicity
 *        of the extract operation.
 */
static void _RBTree_Extract_validate_unprotected(
    RBTree_Node *the_node
    )
{
  RBTree_Node *parent, *sibling;
  RBTree_Direction dir;
  RBTree_Direction opp_dir;

  parent = _RBTree_Parent( the_node );
  if( !parent )
    return;

  sibling = _RBTree_Sibling( the_node );

  /* continue to correct tree as long as the_node is black and not the root */
  while ( !_RBTree_Is_red( the_node ) && parent ) {

    /* if sibling is red, switch parent (black) and sibling colors,
     * then rotate parent left, making the sibling be the_node's grandparent.
     * Now the_node has a black sibling and red parent. After rotation,
     * update sibling pointer.
     */
    if ( _RBTree_Is_red( sibling ) ) {
      _RBTree_Set_color( parent, RBT_RED );
      _RBTree_Set_color( sibling, RBT_BLACK );
      dir = the_node != parent->child[0];
      opp_dir = _RBTree_Opposite_direction( dir );
      _RBTree_Rotate( parent, dir );
      sibling = parent->child[opp_dir];
    }

    /* sibling is black, see if both of its children are also black. */
    if (
        !_RBTree_Is_red( sibling->child[RBT_RIGHT] )
        && !_RBTree_Is_red( sibling->child[RBT_LEFT] )
    ) {
      _RBTree_Set_color( sibling, RBT_RED );
      if ( _RBTree_Is_red( parent ) ) {
        _RBTree_Set_color( parent, RBT_BLACK );
        break;
      }
      if ( !_RBTree_Parent(parent) ) {
        _RBTree_Add_to_black_height(parent->parent, -1);
        break;
      }
      the_node = parent; /* done if parent is red */
      parent = _RBTree_Parent( the_node );
      sibling = _RBTree_Sibling( the_node );
    } else {
      /* at least one of sibling's children is red. we now proceed in two
       * cases, either the_node is to the left or the right of the parent.
       * In both cases, first check if one of sibling's children is black,
       * and if so rotate in the proper direction and update sibling pointer.
       * Then switch the sibling and parent colors, and rotate through parent.
       */
      dir = the_node != parent->child[0];
      opp_dir = _RBTree_Opposite_direction( dir );
      if ( !_RBTree_Is_red( sibling->child[opp_dir] ) ) {
        _RBTree_Set_color( sibling, RBT_RED );
        _RBTree_Set_color( sibling->child[dir], RBT_BLACK );
        _RBTree_Rotate( sibling, opp_dir );
        sibling = parent->child[opp_dir];
      }
      _RBTree_Copy_color( sibling, parent );
      _RBTree_Set_color( parent, RBT_BLACK );
      _RBTree_Set_color(
          sibling->child[opp_dir],
          RBT_BLACK
      );
      _RBTree_Rotate( parent, dir );
      break; /* done */
    }
  } /* while */
  if ( !_RBTree_Parent( the_node ) ) {
    _RBTree_Set_color( the_node, RBT_BLACK );
    _RBTree_Add_to_black_height(the_node->parent, 1);
  }
}

/** @brief Extract a Node (unprotected)
 *
 *  This routine extracts (removes) @a the_node from @a the_rbtree.
 *
 *  @note It does NOT disable interrupts to ensure the atomicity
 *        of the extract operation.
 */
void _RBTree_Extract_unprotected(
    RBTree_Control *the_rbtree,
    RBTree_Node *the_node
    )
{
  RBTree_Node *leaf, *target;
  RBTree_Node *lc, *rc;
  RBTree_Node *p;
  RBTree_Node *tmp;
  RBTree_Attribute victim_color;
  RBTree_Direction dir;

  if ( !the_node )
    return;

  /* check if min needs to be updated */
  if ( the_node == the_rbtree->first[RBT_LEFT] ) {
    RBTree_Node *next;
    next = _RBTree_Successor_unprotected(the_node);
    the_rbtree->first[RBT_LEFT] = next;
  }

  /* Check if max needs to be updated. min=max for 1 element trees so
   * do not use else if here. */
  if ( the_node == the_rbtree->first[RBT_RIGHT] ) {
    RBTree_Node *previous;
    previous = _RBTree_Predecessor_unprotected(the_node);
    the_rbtree->first[RBT_RIGHT] = previous;
  }

  lc = _RBTree_Child(the_node, RBT_LEFT);
  rc = _RBTree_Child(the_node, RBT_RIGHT);
  /* if the_node has at most one non-null child then it is safe to proceed
   * check if both children are non-null, if so then we must find a target node
   * either max in node->child[RBT_LEFT] or min in node->child[RBT_RIGHT],
   * and replace the_node with the target node. This maintains the binary
   * search tree property, but may violate the red-black properties.
   */
  if ( lc && rc ) {
    /* find max in node's left subtree. */
    target = lc;
    tmp = _RBTree_Child(target, RBT_RIGHT);
    while ( tmp ) {
      target = tmp;
      tmp = _RBTree_Child(target, RBT_RIGHT);
    }

    /* if the target node has a child, need to move it up the tree into
     * target's position (target is the right child of target->parent)
     * when target vacates it. if there is no child, then target->parent
     * should become NULL. This may cause the coloring to be violated.
     * For now we store the color of the node being deleted in victim_color.
     */
    leaf = _RBTree_Child(target, RBT_LEFT);
    if ( leaf ) {
      leaf->parent = target->parent;
      dir = _RBTree_Direction_from_parent(target);
      target->parent->child[dir] = leaf;
    } else {
      /* fix the tree here if the child is a null leaf. */
      _RBTree_Extract_validate_unprotected( target );
      dir = _RBTree_Direction_from_parent(target);
      target->parent->child[dir] = target->child[RBT_LEFT]; // FIXME
      _RBTree_Set_attribute(
          target->parent,
          RBTree_Attribute_left_thread << RBT_LEFT,
          RBTree_Attribute_left_thread << RBT_LEFT
      );
    }
    victim_color = _RBTree_Get_color( target );

    /* now replace the_node with target */
    dir = _RBTree_Direction_from_parent(the_node);
    the_node->parent->child[dir] = target;

    /* set target's new children to the original node's children */
    target->child[RBT_RIGHT] = the_node->child[RBT_RIGHT];
    if ( ( rc = _RBTree_Child( the_node, RBT_RIGHT ) ) )
      rc->parent = target;
    target->child[RBT_LEFT] = the_node->child[RBT_LEFT];
    if ( ( lc = _RBTree_Child( the_node, RBT_LEFT ) ) )
      lc->parent = target;

    /* finally, update the parent node and recolor. target has completely
     * replaced the_node, and target's child has moved up the tree if needed.
     * the_node is no longer part of the tree, although it has valid pointers
     * still.
     */
    target->parent = the_node->parent;
    _RBTree_Copy_color(target, the_node);
  } else {
    /* the_node has at most 1 non-null child. Move the child in to
     * the_node's location in the tree. This may cause the coloring to be
     * violated. We will fix it later.
     * For now we store the color of the node being deleted in victim_color.
     */
    if ( lc ) {
      leaf = lc;
      dir = RBT_RIGHT;
    } else {
      leaf = rc;
      dir = RBT_LEFT;
    }

    if ( leaf ) {
      tmp = leaf;
      while ( _RBTree_Child(tmp, dir) ) {
        tmp = _RBTree_Child(tmp, dir);
      }
      tmp->child[dir] = leaf;
      leaf->parent = the_node->parent;

      /* remove the_node from the tree */
      dir = _RBTree_Direction_from_parent(the_node);
      the_node->parent->child[dir] = leaf;
    } else {
      /* fix the tree here if the child is a null leaf. */
      _RBTree_Extract_validate_unprotected( the_node );
      p = _RBTree_Parent(the_node);
      if ( p ) {
        /* remove the_node from the tree */
        dir = _RBTree_Direction_from_parent(the_node);
        p->child[dir] = the_node->child[dir];
      }
    }
    victim_color = _RBTree_Get_color( the_node );
  }

  /* fix coloring. leaf has moved up the tree. The color of the deleted
   * node is in victim_color. There are two cases:
   *   1. Deleted a red node, its child must be black. Nothing must be done.
   *   2. Deleted a black node, its child must be red. Paint child black.
   */
  if ( victim_color == RBT_BLACK ) { /* eliminate case 1 */
    if ( leaf ) {
      _RBTree_Set_color( leaf, RBT_BLACK ); /* case 2 */
    }
  }

  /* Wipe the_node */
  _RBTree_Set_off_rbtree(the_node);

  /* set root to black, if it exists */
  if ( the_rbtree->root ) {
    _RBTree_Set_color(the_rbtree->root, RBT_BLACK);
  }
}


/*
 *  _RBTree_Extract
 *
 *  This kernel routine deletes the given node from a rbtree.
 *
 *  Input parameters:
 *    node - pointer to node in rbtree to be deleted
 *
 *  Output parameters:  NONE
 *
 *  INTERRUPT LATENCY:
 *    only case
 */

void _RBTree_Extract(
  RBTree_Control *the_rbtree,
  RBTree_Node *the_node
)
{
  ISR_Level level;

  _ISR_Disable( level );
    _RBTree_Extract_unprotected( the_rbtree, the_node );
  _ISR_Enable( level );
}
