
#include "../shared/pqbench.h"

/* PQ implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/unitedskiplist.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  SPARC64_SET_UNITEDSKIPLIST_OPERATIONS(tid);
  sparc64_spillpq_initialize(tid, size);
}

