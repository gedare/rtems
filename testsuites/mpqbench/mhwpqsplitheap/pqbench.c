
#include "../shared/pqbench.h"

/* PQ implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/splitheappq.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  SPARC64_SET_SPLITHEAPPQ_OPERATIONS(tid);
  sparc64_spillpq_initialize(tid, size);
}

void pq_insert( rtems_task_argument tid, uint64_t p ) {
  HWDS_ENQUEUE(tid, kv_key(p), kv_value(p)); // FIXME: PQ number
}

uint64_t pq_first( rtems_task_argument tid ) {
  uint64_t kv;
  HWDS_FIRST(tid, kv); // FIXME: PQ number
  return kv;
}

uint64_t pq_pop( rtems_task_argument tid ) {
  uint64_t kv;
  HWDS_FIRST(tid, kv);  // FIXME: PQ number
  if ( kv != (uint64_t)-1 )
    HWDS_EXTRACT(tid, kv);  // TODO: why not just one op for pop?
  return kv;
}

uint64_t pq_search( rtems_task_argument tid, int key ) {
  uint64_t kv;
  HWDS_SEARCH(tid, key, kv);
  return kv;
}

