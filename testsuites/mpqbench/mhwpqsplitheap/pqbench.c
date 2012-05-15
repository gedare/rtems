
#include "../shared/pqbench.h"

/* PQ implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/splitheappq.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  sparc64_spillpq_initialize(tid, SPILLPQ_POLICY_DEFAULT, &sparc64_splitheappq_ops, size);
}

