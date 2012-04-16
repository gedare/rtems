
#include "../shared/pqbench.h"

/* ST implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/unitedlistpq.h> /* bad */

/* test interface */
void st_initialize( rtems_task_argument tid, int size ) {
  SPARC64_SET_UNITEDLISTPQ_OPERATIONS(tid);
  sparc64_spillpq_initialize(tid, size);
}

void st_insert( rtems_task_argument tid, uint64_t p ) {
  HWDS_ENQUEUE(tid, kv_key(p), kv_value(p));
}

uint64_t st_first( rtems_task_argument tid ) {
  uint64_t kv;
  HWDS_FIRST(tid, kv);
  return kv;
}

uint64_t st_pop( rtems_task_argument tid ) {
  uint64_t kv;
  HWDS_FIRST(tid, kv);
  if ( kv != (uint64_t)-1 )
    HWDS_EXTRACT(tid, kv);  // TODO: why not just one op for pop?
  return kv;
}

