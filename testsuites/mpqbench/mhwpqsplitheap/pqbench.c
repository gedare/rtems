
#include "../shared/pqbench.h"

/* PQ implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/splitheappq.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  SPARC64_SET_SPLITHEAPPQ_OPERATIONS(tid);
  sparc64_spillpq_initialize(tid, size);
}

