/* For copyright information, see olden_v1.0/COPYRIGHT */

#include "perimeter.h"
#ifdef SS_PLAIN
#include "ssplain.h"
#endif SS_PLAIN

static int adj(Direction d, ChildType ct)
{
  static int adj_mat[/* Direction */4][/* ChildType */4] =
  {
    {1, 1, 0, 0}, /* n */
    {0, 1, 0, 1}, /* e */
    {0, 0, 1, 1}, /* s */
    {1, 0, 1, 0} /* w */
  };

  return adj_mat[(int)d][(int)ct];
}

static ChildType reflect(Direction d, ChildType ct) 
{
  static ChildType reflection_mat[/* Direction */4][/* ChildType */4] =
  { 
    {southwest, southeast, northwest, northeast},/* n */
    {northeast, northwest, southeast, southwest},/* e */	 
    {southwest, southeast, northwest, northeast},/* s */
    {northeast, northwest, southeast, southwest}/* w */	 
  };
  return reflection_mat[(int)d][(int)ct];
}

int CountTree(QuadTree tree) 
{
  QuadTree nw,ne,sw,se;

  nw = tree->nw; ne = tree->ne; sw = tree->sw; se = tree->se; /* bad loads */
  if (nw==NULL && ne==NULL && sw==NULL && se==NULL)
    return 1;
  else
    /* Trees should be traversed nw, ne, se, sw in that order to
       match jump pointers */
    return CountTree(nw) + CountTree(ne) + CountTree(se) +
      CountTree(sw);
}

static QuadTree child(QuadTree tree, ChildType ct)
{
  switch(ct) 
    {
    case northeast:
      return tree->ne;
    case northwest:
      return tree->nw;
    case southeast:
      return tree->se;
    case southwest:
      return tree->sw;
    }
}

static QuadTree gtequal_adj_neighbor(QuadTree tree, Direction d)
{
  QuadTree q,parent;
  ChildType ct;
  
  parent=tree->parent;
  ct=tree->childtype;
  if ((parent!=NULL) && adj(d,ct))
    q=gtequal_adj_neighbor(parent,d);
  else q=parent;
  if (q && q->color==grey) {
    return child(q,reflect(d,ct));
  }
  else return q;
}

static int sum_adjacent(QuadTree p, ChildType q1, ChildType q2, int size)
{
  if (p->color==grey) 
    {
      return sum_adjacent(child(p,q1),q1,q2,size/2) +
	sum_adjacent(child(p,q2),q1,q2,size/2);
    }
  else if (p->color==white) 
    {
      return size;
    }
  else return 0;
}

int perimeter(QuadTree tree, int size)
{
  int retval = 0;
  QuadTree neighbor;

  if (tree->color==grey)  /* bad load */
    {
      retval += perimeter(tree->nw, size/2);
      retval += perimeter(tree->ne, size/2);
      retval += perimeter(tree->sw, size/2);
      retval += perimeter(tree->se, size/2);
    }
  else if (tree->color==black)
    {
      /* South */
      neighbor=gtequal_adj_neighbor(tree,south);
      if ((neighbor==NULL) || (neighbor->color==white)) retval+=size;
      else if (neighbor->color==grey) 
	retval+=sum_adjacent(neighbor,northwest,northeast,size);
      /* West */
      neighbor=gtequal_adj_neighbor(tree,west);
      if ((neighbor==NULL) || (neighbor->color==white)) retval+=size;
      else if (neighbor->color==grey) 
	retval+=sum_adjacent(neighbor,northeast,southeast,size);
      /* North */
      neighbor=gtequal_adj_neighbor(tree,north);
      if ((neighbor==NULL) || (neighbor->color==white)) retval+=size;
      else if (neighbor->color==grey) 
	retval+=sum_adjacent(neighbor,southeast,southwest,size);
      /* East */
      neighbor=gtequal_adj_neighbor(tree,east);
      if ((neighbor==NULL) || (neighbor->color==white)) retval+=size;
      else if (neighbor->color==grey) 
	retval+=sum_adjacent(neighbor,southwest,northwest,size);
    }
  return retval;
}

extern int dealwithargs(int argc, char **argv);
 
int main(int argc, char *argv[])
{
  QuadTree tree;
  int count;
  int level;

  level = dealwithargs(argc,argv);

  chatting("Perimeter with %d levels on %d processors\n",level,__NumNodes);
  tree=MakeTree(4096,0,0,NULL,southeast,level);
  count=CountTree(tree);
  chatting("# of leaves is %d\n",count);

  count=perimeter(tree,4096);

  chatting("perimeter is %d\n",count);
//  exit(0); /* RTEMS */
  return 0; /* RTEMS */
}

