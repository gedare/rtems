
#include "../shared/pqbench.h"

/* PQ implementation */
#include <libcpu/hwpqlib.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  if ( tid == 0 )
    hwpqlib_initialize(0, NUM_TASKS); // FIXME
  hwpqlib_pq_initialize( HWPQLIB_SPILLPQ_UNITEDLIST, tid, size );//FIXME spillpq
}

void pq_insert( rtems_task_argument tid, uint64_t p ) {
  hwpqlib_insert(tid, kv_key(p), kv_value(p));// FIXME: PQ number
}

uint64_t pq_first(  rtems_task_argument tid ) {
  return hwpqlib_first( tid ); // FIXME: PQ number
}

uint64_t pq_pop( rtems_task_argument tid ) {
  return hwpqlib_pop( tid );  // FIXME: PQ number
}

