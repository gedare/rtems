/**
 * @file rtems/rbtree.h
 *
 * @ingroup ScoreSplay
 * 
 * This include file contains all the constants and structures associated
 * with the Splay Tree API in RTEMS.
 */

/*
 *  Copyright (c) 2012 Gedare Bloom.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#ifndef _RTEMS_SPLAY_H
#define _RTEMS_SPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtems/system.h>
#include "rtemsscoresplay.h"

typedef Splay_Node rtems_splay_node;
typedef Splay_Control rtems_splay_control;
typedef Splay_Compare_function rtems_splay_compare_function;

static void inline rtems_splay_initialize_empty(
  rtems_splay_control *the_tree,
  rtems_splay_compare_function compare_function
)
{
  _Splay_Initialize_empty(the_tree, compare_function);
}

static inline rtems_splay_node * rtems_splay_insert(
  rtems_splay_control *the_tree, rtems_splay_node *the_node 
)
{
  return _Splay_Insert(the_tree, the_node);
}

static inline rtems_splay_node *rtems_splay_dequeue(
  rtems_splay_control *the_tree
)
{
  return _Splay_Dequeue(the_tree);
}

static inline void rtems_splay_splay(
  rtems_splay_control *the_tree,
  rtems_splay_node *the_node 
)
{
  return _Splay_Splay(the_tree, the_node);
}
static inline rtems_splay_node *rtems_splay_find(
  rtems_splay_control *the_tree,
  rtems_splay_node *search_node
)
{
  return _Splay_Find(the_tree, search_node);
}

static inline rtems_splay_node* rtems_splay_extract(
  rtems_splay_control *the_tree,
  rtems_splay_node *the_node
)
{
  return _Splay_Extract(the_tree, the_node);
}

#ifdef __cplusplus
}
#endif

#endif
