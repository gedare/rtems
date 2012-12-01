/* For copyright information, see olden_v1.0/COPYRIGHT */
 

/*
 * CODE.C: export version of hierarchical N-body code.
 * Copyright (c) 1991, Joshua E. Barnes, Honolulu, HI.
 * 	    It's free because it's yours.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef SS_PLAIN
#include "ssplain.h"
#endif SS_PLAIN

#include "defs.h"
#include "code.h"

int nbody;
double xrand(), my_rand();
extern icstruct intcoord(bodyptr p, treeptr t);
extern int BhDebug;

void computegrav(treeptr t);
nodeptr maketree(int nbody, treeptr t);
void vp(bodyptr q, int nstep);

typedef struct {
	vector cmr;
	vector cmv;
	bodyptr list;
   bodyptr tail;
} datapoints;

datapoints uniform_testdata(int nbody, int seedfactor);
void stepsystem(treeptr t, int nstep);
treeptr old_main();
void my_free(nodeptr n);
bodyptr ubody_alloc();
void freetree(nodeptr n);

/* Used to setup runtime system, get arguments-- see old_main */
void
main(int argc, char *argv)
{
  dealwithargs(argc, argv);

  chatting("nbody = %d, numnodes = %d\n", nbody, 1);
  old_main();
  chatting("Time = %f seconds\n", (double)0);
  __ShutDown(0);
}

/* Main routine from original program */
treeptr old_main()
{
  real tnow;
  int i, nsteps;
  treeptr t;
  bodyptr p;
  vector cmr, cmv;
  int tmp=0;
  int bodiesper;
  bodyptr ptrper;
  datapoints points;

  srand(123);					/*   set random generator   */

/* Tree data structure is global, points to root, and bodylist, has size info */
  t = (treeptr) mymalloc(sizeof(tree));
  Root(t) = NULL;
  t->rmin[0] = -2.0;
  t->rmin[1] = -2.0;
  t->rmin[2] = -2.0;
  t->rsize = -2.0 * -2.0;  /*t->rmin[0];*/

  CLRV(cmr);
  CLRV(cmv);

  /* Creates a list of bodies */
  points=uniform_testdata(nbody, 1);
  t->bodytab=points.list;
  ADDV(cmr,cmr,points.cmr);
  ADDV(cmv,cmv,points.cmv);

  chatting("bodies created \n");
  DIVVS(cmr, cmr, (real) nbody);		/* normalize cm coords */
  DIVVS(cmv, cmv, (real) nbody);

  bodiesper = 0;
  ptrper = NULL;

  /* Recall particles are in lists by processor */
  for (p = t->bodytab; p != NULL; p = Next(p)) {/* loop over particles */
    SUBV(Pos(p), Pos(p), cmr);		/*   offset by cm coords    */
    SUBV(Vel(p), Vel(p), cmv);

    bodiesper++;
    Proc_Next(p) = ptrper;
    ptrper = p;
  }
  chatting("Bodies per 0 = %d\n" ,bodiesper);
  t->bodiesperproc=ptrper;

  tmp = 0;
  tnow = 0.0;
  i = 0;

  /* Go through sequence of iterations */
  nsteps = 1;
  chatting("About to perform %d iters from %f to %f by %f\n",
            nsteps,tnow,tstop,dtime);

  while ((tnow < tstop + 0.1*dtime) && (i < nsteps)) {
    chatting("Step %d\n", i);
    stepsystem(t, i);     /*   advance N-body system  */
    tnow = tnow + dtime;
    /*chatting("tnow = %f sp = 0x%x\n", tnow, __getsp());*/
    i++;
  }

  /* Return the tree to calling routine */
  return t;
}

/*
 * STEPSYSTEM: advance N-body system one time-step.
 */
extern int EventCount;

void stepsystem(treeptr t, int nstep)
{ 
  if (Root(t) != NULL) {
    freetree(Root(t));
    Root(t) = NULL;
  }

  Root(t) = maketree(nbody, t);
  computegrav(t);
  vp(t->bodiesperproc,nstep);
  chatting("M %f, CG %f, VP %f, sp 0x%x,\n", 
	   (double)0, (double)0, (double)0, 0);
}

void freetree(nodeptr n)
{
  nodeptr r;
  int i;
  
  if ((n == NULL) || (Type(n) == BODY))
    return;

  /* Type(n) == CELL */
  for (i=NSUB-1; i >= 0; i--) {
    r = Subp((cellptr) n)[i];
    if (r != NULL) {
      freetree(r);
    }
  }

  my_free(n);
}

nodeptr cp_free_list = NULL;
bodyptr bp_free_list = NULL;


/* should never be called with NULL */
void my_free(nodeptr n)
{
  if (Type(n) == BODY) {
    Next((bodyptr) n) = bp_free_list;
    bp_free_list = (bodyptr) n;
  }
  else /* CELL */ {
    FL_Next((cellptr) n) = (cellptr) cp_free_list;
    cp_free_list = n;
  }
}
    
bodyptr ubody_alloc()
{ 
  bodyptr tmp = (bodyptr) mymalloc(sizeof(body));
  Type(tmp) = BODY;
  Proc_Next(tmp) = NULL;
  return tmp;
}


cellptr cell_alloc()
{ 
  cellptr tmp;
  int i;

  if (cp_free_list != NULL) {
    tmp = (cellptr) cp_free_list;
    cp_free_list = (nodeptr) FL_Next((cellptr) cp_free_list);
  }
  else 
    tmp = (cellptr) mymalloc(sizeof(cell));
  Type(tmp) = CELL;
  for (i=0; i < NSUB; i++)
    Subp(tmp)[i] = NULL;

  return tmp;
}

#define MFRAC  0.999		/* mass cut off at MFRAC of total */
#define DENSITY 0.5

datapoints uniform_testdata(int nbody, int seedfactor)
{
  datapoints retval;
  real rsc, vsc, r, v, x, y;
  bodyptr head, p, prev;
  register i;
  double temp, t1;
  double seed = 123.0 * (double) seedfactor;
  register int k;
  double rsq, rsc1;
  real rad;
  real coeff;

  /* Go to the desired processor */

  rsc = 3 * PI / 16;				/* set length scale factor  */
  vsc = sqrt(1.0 / rsc);			/* and recip. speed scale   */
  CLRV(retval.cmr);					/* init cm pos, vel         */
  CLRV(retval.cmv);
  head = (bodyptr) ubody_alloc();
  prev = head;

  for (i = 0; i < nbody; i++) {	        /* loop over particles      */
    p = ubody_alloc();
						/* alloc space for bodies   */
    if (p == NULL)  			/* check space is available */
      error("testdata: not enough memory\n");	/*   if not, cry            */
    Next(prev) = p;
    prev = p;
    Type(p) = BODY;				/*   tag as a body          */
    Mass(p) = 1.0 / nbody;			/*   set masses equal       */
    seed = my_rand(seed);
    t1 = xrand(0.0, MFRAC, seed);
    temp = pow(t1,	                        /*   pick r in struct units */
			 -2.0/3.0) - 1;
    r = 1 / sqrt(temp);
    
    coeff = 4.0; /* exp(log(nbody/DENSITY)/3.0); */
    for (k=0; k < NDIM; k++) {
      seed = my_rand(seed);
      r = xrand(0.0, MFRAC, seed);
      Pos(p)[k] = coeff*r;
    }

    ADDV(retval.cmr, retval.cmr, Pos(p));	/*   add to running sum     */
    do {					/*   select from fn g(x)    */
      seed = my_rand(seed);
      x = xrand(0.0, 1.0, seed);   		/*     for x in range 0:1   */
      seed = my_rand(seed);
      y = xrand(0.0, 0.1, seed);  		/*     max of g(x) is 0.092 */
    } while (y > x*x * pow(1 - x*x, 3.5));	/*   using von Neumann tech */
    v = sqrt(2.0) * x / pow(1 + r*r, 0.25);	/*   find v in struct units */

    /*   pick scaled velocity   */
    rad = vsc*v;
    
    do {					/* pick point in NDIM-space */
      for (k = 0; k < NDIM; k++)	{	/* loop over dimensions   */
	seed = my_rand(seed);
	Vel(p)[k] = xrand(-1.0, 1.0, seed);	/* pick from unit cube  */
      }
      DOTVP(rsq, Vel(p), Vel(p));		/*   compute radius squared */
    } while (rsq > 1.0);                	/* reject if outside sphere */
    rsc1 = rad / sqrt(rsq);		        /* compute scaling factor   */
    MULVS(Vel(p), Vel(p), rsc1);		/* rescale to radius given  */
    ADDV(retval.cmv, retval.cmv, Vel(p));	/*   add to running sum     */
  }


  Next(prev) = NULL;                          /*   mark end of the list   */
  retval.list = Next(head);                   /*   toss the dummy node    */
  retval.tail = prev;

  return retval;
}
/*
 * GRAV.C: routines to compute gravity. Public routines: hackgrav().
 * Copyright (c) 1991, Joshua E. Barnes, Honolulu, HI.
 * 	    It's free because it's yours.
 */

typedef struct {
  bodyptr pskip;		/* body to skip in force evaluation */
  vector pos0;			/* point at which to evaluate field */
  real phi0;			/* computed potential at pos0       */
  vector acc0;			/* computed acceleration at pos0    */
} hgstruct, *hgsptr;

hgstruct gravsub(nodeptr p, hgstruct hg);
hgstruct walksub(nodeptr p, hgstruct hg);
void grav(nodeptr rt, bodyptr q);
void vp(bodyptr q, int nstep);
void hackgrav(bodyptr p, nodeptr rt);

void computegrav(treeptr t)
{  
  grav(Root(t),t->bodiesperproc);
}


void grav(nodeptr rt, bodyptr bodies)
{
  bodyptr q;

  for (q=bodies; q; q=Proc_Next(q)) 
    hackgrav(q, rt);
}

void vp(bodyptr q, int nstep)
{
  real dthf;
  vector acc1, dacc, dvel, vel1, dpos;

  dthf = 0.5 * dtime;				/* set basic half-step      */

  for (; q != NULL; q = Proc_Next(q))  {
    SETV(acc1, New_Acc(q));
    if (nstep > 0) {			/*   past the first step?   */
      SUBV(dacc, acc1, Acc(q));   /*     use change in accel  */
      MULVS(dvel, dacc, dthf);		/*     to make 2nd order    */
      ADDV(dvel, Vel(q), dvel);
      SETV(Vel(q), dvel);
    }
    SETV(Acc(q), acc1);
    MULVS(dvel, Acc(q), dthf);	        /*   use current accel'n    */
    ADDV(vel1, Vel(q), dvel);		/*   find vel at midpoint   */
    MULVS(dpos, vel1, dtime);		/*   find pos at endpoint   */
    ADDV(dpos, Pos(q), dpos);   	/*   advance position       */
    SETV(Pos(q),dpos);
    ADDV(Vel(q), vel1, dvel);		/*   advance velocity       */
  }
}

  
/*
 * HACKGRAV: evaluate grav field at a given particle.
 */


void hackgrav(bodyptr p, nodeptr rt)
{
  hgstruct hg;

  hg.pskip = p;					/* exclude from force calc. */
  SETV(hg.pos0, Pos(p));			/* eval force on this point */
  hg.phi0 = 0.0;				/* init grav. potential and */
  CLRV(hg.acc0);
  hg = walksub(rt, hg);                         /* recursively scan tree    */
  Phi(p) = hg.phi0;				/* stash resulting pot. and */
  SETV(New_Acc(p), hg.acc0);			/* acceleration in body p   */
}

/*
 * GRAVSUB: compute a single body-body or body-cell interaction.
 */

hgstruct gravsub(nodeptr p, hgstruct hg)
{
  real drabs, phii, mor3;
  vector ai, dr;
  real drsq;


  SUBV(dr, Pos(p), hg.pos0);            /*  find seperation   */
  DOTVP(drsq, dr, dr);			/*   and square of distance */

  drsq += eps*eps;                      /* use standard softening   */
  drabs = sqrt((double) drsq);		/* find norm of distance    */
  phii = Mass(p) / drabs;		/* and contribution to phi  */
  hg.phi0 -= phii;                      /* add to total potential   */
  mor3 = phii / drsq;  			/* form mass / radius qubed */
  MULVS(ai, dr, mor3);			/* and contribution to acc. */
  ADDV(hg.acc0, hg.acc0, ai);           /* add to net acceleration  */

  return hg;
}

/*
 * WALKSUB: recursive routine to do hackwalk operation.
 * p: pointer into body-tree 
 * dsq: size of box squared 
 */

hgstruct walksub(nodeptr p, hgstruct hg)
{
  register int k;
  register nodeptr r;

  if (Type(p) != BODY) {           /* bad load */
    for (k = 0; k < NSUB; k++) {   
      r = Subp((cellptr) p)[k];    /* bad load */
      if (r != NULL)               
	hg = walksub(r, hg);
    }
  }
  else if (p != (nodeptr) hg.pskip)   {         /* should p be included?  */
    hg = gravsub(p, hg);                           /* then use interaction   */
  }


  return hg;
}

/*
 * LOAD.C: routines to create body-tree.  Public routines: maketree().
 * Copyright (c) 1991, Joshua E. Barnes, Honolulu, HI.
 * 	    It's free because it's yours.
 */

bodyptr ubody_alloc();
cellptr cell_alloc();

nodeptr loadtree(bodyptr p, icstruct xpic, nodeptr t, int l, treeptr tr);
void expandbox(bodyptr p, treeptr t);
icstruct intcoord1(double rp0, double rp1, double rp2,  treeptr t);
icstruct intcoord(bodyptr p,  treeptr t);
int ic_test(bodyptr p, treeptr t);
int subindex(bodyptr p, treeptr t, int l);
int old_subindex(icstruct ic, int l);
real hackcofm(nodeptr q);


/*
 *  MAKETREE: initialize tree structure for hack force calculation.
 */

nodeptr maketree(int nb, treeptr t)
{  
  bodyptr q;
  icstruct xqic;

  Root(t) = NULL;
  nbody = nb;

  for (q = t->bodiesperproc; q; q=Proc_Next(q)) { 
    if (Mass(q) != 0.0) {			/* only load massive ones */
      expandbox(q, t);                /* expand root to fit   */
      xqic = intcoord(q,t);
      Root(t) = loadtree(q, xqic, Root(t), IMAX >> 1, t);
    } /* if Mass... */
  } /* for q = btab... */
  
  hackcofm(Root(t));				/* find c-of-m coordinates  */
  return Root(t);
}
  

 

/*
 * New EXPANDBOX: enlarge cubical "box", salvaging existing tree structure.
 * p - body to be loaded 
 * t - tree 
 */

void
expandbox(bodyptr p, treeptr t)       
{
    icstruct ic;
    int k;
    vector rmid;
    cellptr  newt;
    real rsize;
    int inbox;

    inbox = ic_test(p, t);
    while (!inbox) {            		/* expand box (rarely)      */
      rsize = Rsize(t);
      /*chatting("expanding box 0x%x, size=%f\n",p,rsize);*/
      ADDVS(rmid, Rmin(t), 0.5 * rsize);    /*   find box midpoint      */
                                            /*   loop over dimensions   */
      for (k=0; k < NDIM; k++)
        if (Pos(p)[k] < rmid[k])            /*     is p left of mid?    */
           {
             real rmin;
             rmin = Rmin(t)[k];
	          Rmin(t)[k] = rmin - rsize;   /*       extend to left     */
           }
      /*chatting("rsize now = %f\n",rsize);*/
      Rsize(t) = 2.0 * rsize;               /*   double length of box   */


      rsize = Rsize(t);
      /*chatting("rsize now = %f\n",rsize);*/
      if (Root(t) != NULL) {                  /*   repot existing tree?   */
	   newt = cell_alloc();

	   ic = intcoord1(rmid[0], rmid[1], rmid[2], t);
	   /*   locate old root cell   */
	   k = old_subindex(ic, IMAX >> 1);    /*     find old tree index  */
	   Subp(newt)[k] = Root(t);            /*     graft old on new     */
	   Root(t) = (nodeptr) newt;           /*     plant new tree       */
	   inbox = ic_test(p, t);
      } /* if Root... */
    } /* while !inbox */
}

/*
 * New LOADTREE: descend tree and insert particle.
 * p - body to be loaded 
 * xp - integer coordinates of p
 * t - tree
 * l - current level in tree 
 */

nodeptr loadtree(bodyptr p, icstruct xpic, nodeptr t, int l, treeptr tr)
{
  int si;
  cellptr c;
  nodeptr rt;

  if (t == NULL)
    return ((nodeptr) p);

  if (Type(t) == BODY) {        		/*   reached a "leaf"?      */
    c = (cellptr) cell_alloc();
    si = subindex((bodyptr) t, tr, l); 
     
    Subp(c)[si] = (nodeptr) t;        	/*     put body in cell     */
    t = (nodeptr) c;	        	/*     link cell in tree    */
  }

  si = old_subindex(xpic, l);     /* move down one level      */
  rt = Subp((cellptr) t)[si];
  Subp((cellptr) t)[si] = loadtree(p, xpic, rt, l >> 1, tr);

  return (t);
}


/*
 * INTCOORD: compute integerized coordinates.
 * Returns: TRUE unless rp was out of bounds.
 */
/* called from expandbox */
icstruct intcoord1(double rp0, double rp1, double rp2, treeptr t)
{
    double xsc, floor();
    icstruct ic;

    ic.inb = TRUE;				/* use to check bounds      */

    xsc = (rp0 - t->rmin[0]) / t->rsize;    /*   scale to range [0,1)   */
    if (0.0 <= xsc && xsc < 1.0)              /*   within unit interval?  */
      ic.xp[0] = floor(IMAX * xsc);           /*     then integerize      */
    else                                     /*   out of range           */
      ic.inb = FALSE;                         /*     then remember that   */

    xsc = (rp1 - t->rmin[1]) / t->rsize;    /*   scale to range [0,1)   */
    if (0.0 <= xsc && xsc < 1.0)              /*   within unit interval?  */
      ic.xp[1] = floor(IMAX * xsc);           /*     then integerize      */
    else                                     /*   out of range           */
      ic.inb = FALSE;                         /*     then remember that   */

    xsc = (rp2 - t->rmin[2]) / t->rsize;    /*   scale to range [0,1)   */
    if (0.0 <= xsc && xsc < 1.0)              /*   within unit interval?  */
      ic.xp[2] = floor(IMAX * xsc);           /*     then integerize      */
    else                                     /*   out of range           */
      ic.inb = FALSE;                         /*     then remember that   */

    return (ic);
}


/*
 * INTCOORD: compute integerized coordinates.
 * Returns: TRUE unless rp was out of bounds.
 */

icstruct intcoord(bodyptr p, treeptr t)
{
    double xsc;
    double floor();
    icstruct ic;
    real rsize;
    vector pos;

    ic.inb = TRUE;				/* use to check bounds      */
    rsize = t->rsize;

    pos[0] = Pos(p)[0];
    pos[1] = Pos(p)[1];
    pos[2] = Pos(p)[2];
 
    xsc = (pos[0] - t->rmin[0]) / rsize;    /*   scale to range [0,1)   */
    if (0.0 <= xsc && xsc < 1.0)              /*   within unit interval?  */
      ic.xp[0] = floor(IMAX * xsc);           /*     then integerize      */
    else                                      /*   out of range           */
      ic.inb = FALSE;                         /*     then remember that   */


    xsc = (pos[1] - t->rmin[1]) / rsize;    /*   scale to range [0,1)   */
    if (0.0 <= xsc && xsc < 1.0)              /*   within unit interval?  */
      ic.xp[1] = floor(IMAX * xsc);           /*     then integerize      */
    else                                      /*   out of range           */
      ic.inb = FALSE;                         /*     then remember that   */


    xsc = (pos[2] - t->rmin[2]) / rsize;    /*   scale to range [0,1)   */
    if (0.0 <= xsc && xsc < 1.0)              /*   within unit interval?  */
      ic.xp[2] = floor(IMAX * xsc);           /*     then integerize      */
    else                                      /*   out of range           */
      ic.inb = FALSE;                         /*     then remember that   */

    return (ic);
}


int ic_test(bodyptr p, treeptr t)
{
    double xsc, rsize;
    int result;
    vector pos;

    result = TRUE;				/* use to check bounds      */

    pos[0] = Pos(p)[0];
    pos[1] = Pos(p)[1];
    pos[2] = Pos(p)[2];
    rsize = t->rsize;

    xsc = (pos[0] - t->rmin[0]) / rsize;    /*   scale to range [0,1)   */
    if (!(0.0 <= xsc && xsc < 1.0))         /*   within unit interval?  */
      result = FALSE;                       /*     then remember that   */

    xsc = (pos[1] - t->rmin[1]) / rsize;    /*   scale to range [0,1)   */
    if (!(0.0 <= xsc && xsc < 1.0))         /*   within unit interval?  */
      result = FALSE;                       /*     then remember that   */

    xsc = (pos[2] - t->rmin[2]) / rsize;    /*   scale to range [0,1)   */
    if (!(0.0 <= xsc && xsc < 1.0))         /*   within unit interval?  */
      result = FALSE;                       /*     then remember that   */

    return (result);
}




/*
 * SUBINDEX: determine which subcell to select.  Rolled intcoord & subindex together.
 */

int subindex(bodyptr p, treeptr t , int l)
{
    int i, k;
    real rsize;
    double xsc, floor();
    int xp[NDIM];
    vector pos;

    pos[0] = Pos(p)[0];
    pos[1] = Pos(p)[1];
    pos[2] = Pos(p)[2];

    rsize = t->rsize;

    xsc = (pos[0] - t->rmin[0]) / rsize;     /*   scale to range [0,1)   */
    xp[0] = floor(IMAX * xsc);                  /*   then integerize      */

    xsc = (pos[1] - t->rmin[1]) / rsize;     /*   scale to range [0,1)   */
    xp[1] = floor(IMAX * xsc);                  /*   then integerize      */

    xsc = (pos[2] - t->rmin[2]) / rsize;     /*   scale to range [0,1)   */
    xp[2] = floor(IMAX * xsc);                  /*   then integerize      */


    i = 0;                                      /* sum index in i           */
    for (k = 0; k < NDIM; k++)                  /* check each dimension     */
        if (xp[k] & l)                          /*   if beyond midpoint     */
            i += NSUB >> (k + 1);               /*     skip over subcells   */

    return (i);
}



int old_subindex(icstruct ic, int l)
{
    int i, k;

    i = 0;                                      /* sum index in i           */
    for (k = 0; k < NDIM; k++)                  /* check each dimension     */
        if (ic.xp[k] & l)                       /*   if beyond midpoint     */
            i += NSUB >> (k + 1);               /*     skip over subcells   */
    return (i);
}


/*
 * HACKCOFM: descend tree finding center-of-mass coordinates.
 */

real hackcofm(nodeptr q)
{
  int i;
  nodeptr r;
  vector tmpv;
  vector tmp_pos;
  real mq, mr;
  
  if (Type(q) == CELL) {                      /* is this a cell?          */
    mq = 0.0;
    CLRV(tmp_pos);				/*   and c. of m.           */
    for (i=0; i < NSUB; i++) {
      r = Subp((cellptr) q)[i];
      if (r != NULL) {
	mr = hackcofm(r);
	mq = mr + mq;
	MULVS(tmpv, Pos(r), mr);   /*       find moment        */
	ADDV(tmp_pos, tmp_pos, tmpv);     /*       sum tot. moment    */
      }
    }
    Mass(q) = mq;
    Pos(q)[0] = tmp_pos[0];
    Pos(q)[1] = tmp_pos[1];
    Pos(q)[2] = tmp_pos[2];
    DIVVS(Pos(q), Pos(q), Mass(q));         /*   rescale cms position   */
    return mq;
  }
  mq = Mass(q);
  return mq;
}







   

