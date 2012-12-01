#include <stdio.h>
#include "em3d.h"

void dealwithargs(int argc, char *argv[])
{
  if (argc > 1)
    n_nodes = atoi(argv[1]);
  else
    n_nodes = e_nodes = 10;

  if (argc > 2)
    d_nodes = atoi(argv[2]);
  else
    d_nodes = 3;

  if (argc > 3)
    iters = atoi(argv[3]);
  else
    iters = 100;

  if (argc > 4)
    e_nodes = atoi(argv[4]);
  else
    e_nodes = n_nodes;
}
