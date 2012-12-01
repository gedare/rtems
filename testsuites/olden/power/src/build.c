/* For copyright information, see olden_v1.0/COPYRIGHT */

/* build.c
 *
 * By:  Martin C. Carlisle
 * 6/15/94
 * builds the tree for the Power Pricing problem
 *
 * based on code by:  Steve Lumetta, Sherry Li, and Ismail Khalil
 * University of California at Berkeley
 */

#include "power.h"

#ifdef SS_PLAIN
#include "ssplain.h"
#endif SS_PLAIN

Root build_tree() 
{
  int i;
  Root t;
  
  t = (Root) malloc(sizeof(*t));
  for (i=0; i<NUM_FEEDERS; i++) 
    t->feeders[i] = build_lateral(i*LATERALS_PER_FEEDER,LATERALS_PER_FEEDER);

  t->theta_R = 0.8;
  t->theta_I = 0.16;
  return t;
}

Lateral build_lateral(int i, int num)
{
  Lateral l;
 
  if (num == 0) return NULL;

  l = (Lateral) malloc (sizeof(*l));

  l->next_lateral = build_lateral(i,num-1);
  l->branch = build_branch(i*BRANCHES_PER_LATERAL,(num-1)*BRANCHES_PER_LATERAL,
    BRANCHES_PER_LATERAL);

  l->R = 1/300000.0;
  l->X = 0.000001;
  l->alpha = 0.0;
  l->beta = 0.0;
  return l;
}

Branch build_branch(int i, int j, int num)
{
  Branch b;

  if (num == 0) return NULL;

  b = (Branch) malloc (sizeof(*b));
  
  /* fill in children */
  b->next_branch= build_branch(i,j,num-1);

  for (i=0; i<LEAVES_PER_BRANCH; i++) 
    b->leaves[i] = build_leaf();
  
  /* fill in values */
  b->R = 0.0001;
  b->X = 0.00002;
  b->alpha = 0.0;
  b->beta = 0.0;
  return b;
}

Leaf build_leaf()
{
  register Leaf l;

  l = (Leaf) mymalloc(sizeof(*l));
  l->D.P = 1.0;
  l->D.Q = 1.0;
  return l;
}

