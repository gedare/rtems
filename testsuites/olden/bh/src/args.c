/* For copyright information, see olden_v1.0/COPYRIGHT */

extern int nbody;
#ifdef SS_PLAIN
#include "ssplain.h"
#endif SS_PLAIN

void dealwithargs(int argc, char *argv[])
{
  if (argc > 1)
    nbody = atoi(argv[1]);
  else
    nbody = 2048;
}
