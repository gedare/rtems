
#include "../shared/pqbench.h"

/* PQ implementation */
#include <libcpu/hwpqlib.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  if (hwpqlib_context.pq_context == NULL)
    hwpqlib_initialize(0, NUM_TASKS);

  // FIXME: how to determine the spillpq structure?
  if ( tid%2 == 0 )
    hwpqlib_pq_initialize( HWPQLIB_SPILLPQ_UNITEDLIST, tid, size );
  else
    hwpqlib_pq_initialize( HWPQLIB_SPILLPQ_SPLITHEAP, tid, size );

}

void pq_insert( rtems_task_argument tid, uint64_t p ) {
  hwpqlib_insert(tid, kv_key(p), kv_value(p));
}

uint64_t pq_first(  rtems_task_argument tid ) {
  return hwpqlib_first( tid );
}

uint64_t pq_pop( rtems_task_argument tid ) {
  return hwpqlib_pop( tid );
}

uint64_t pq_search( rtems_task_argument tid, int key ) {
  return hwpqlib_search(tid, key);
}

uint64_t pq_extract( rtems_task_argument tid, int key ) {
  return hwpqlib_extract(tid, key);
}
