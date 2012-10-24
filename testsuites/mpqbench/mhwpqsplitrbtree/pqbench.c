
#include "../shared/pqbench.h"

/* PQ implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/splitrbtree.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  pqbench_policy[tid].spill_from = 1;
  sparc64_spillpq_initialize(tid, &pqbench_policy[tid], &sparc64_splitrbtree_ops, size);
}

