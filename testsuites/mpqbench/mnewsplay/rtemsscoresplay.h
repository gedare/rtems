/**
 *  @file  rtems/score/splay.h
 *
 *  This include file contains all the constants and structures associated
 *  with the Splay Tree Handler.
 */

/*
 *  Copyright (c) 2012 Gedare Bloom.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#ifndef _RTEMS_SCORE_SPLAY_H
#define _RTEMS_SCORE_SPLAY_H

/**
 *  @defgroup ScoreSplay Splay Tree Handler
 *
 *  @ingroup Score
 *
 *  The Splay Tree Handler is used to manage sets of entities.  This handler
 *  provides two data structures.  The Splay Node data structure is included
 *  as part of every data structure that will be placed in a splay tree.
 *  The second data structure is Splay Control which is used
 *  to manage a tree of splay nodes.
 */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Splay_Node_struct Splay_Node;
struct Splay_Node_struct
{
    Splay_Node  * uplink;
    Splay_Node  * leftlink;
    Splay_Node  * rightlink;
};

typedef int (*Splay_Compare_function)(
  const Splay_Node *node1,
  const Splay_Node *node2
);

typedef struct
{
    Splay_Node * root;    /* root node */
    Splay_Node * min;
    Splay_Node * max;
    Splay_Compare_function compare_function;

    /* Statistics, not strictly necessary, but handy for tuning  */

    int    lookups;  /* number of splookup()s */
    int    lkpcmps;  /* number of lookup comparisons */
    
    int    enqs;    /* number of spenq()s */
    int    enqcmps;  /* compares in spenq */
    
    int    splays;
    int    splayloops;
} Splay_Control;


bool _Splay_Is_empty( Splay_Control *the_tree );
Splay_Node * _Splay_Insert( Splay_Control *the_tree, Splay_Node *the_node );
Splay_Node *_Splay_Dequeue( Splay_Node **node_pointer );
Splay_Node *_Splay_Insert_before(Splay_Control *the_tree, Splay_Node *the_node);
void _Splay_Splay( Splay_Control *the_tree, Splay_Node *the_node );
Splay_Node *_Splay_Find(Splay_Control *the_tree, Splay_Node *search_node);
Splay_Node* _Splay_Extract(Splay_Control *the_tree, Splay_Node *search_node);

static void inline _Splay_Initialize_empty(
  Splay_Control *the_tree,
  Splay_Compare_function compare_function
)
{
  the_tree->lookups = 0;
  the_tree->lkpcmps = 0;
  the_tree->enqs = 0;
  the_tree->enqcmps = 0;
  the_tree->splays = 0;
  the_tree->splayloops = 0;
  the_tree->root = NULL;
  the_tree->min = the_tree->max = NULL;
  the_tree->compare_function = compare_function;
}

#ifdef __cplusplus
}
#endif

/**@}*/

#endif
