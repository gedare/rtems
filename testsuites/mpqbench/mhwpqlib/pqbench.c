
#include "../shared/pqbench.h"

/* PQ implementation */
#include <libcpu/hwpqlib.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  if (hwpqlib_context.pq_context == NULL)
    hwpqlib_initialize(0, NUM_TASKS);

  // FIXME: how to determine the spillpq structure?
  //if ( tid%2 == 0 )
  //  hwpqlib_pq_initialize( HWPQLIB_SPILLPQ_UNITEDLIST, tid, size );
  //else
  //  hwpqlib_pq_initialize( HWPQLIB_SPILLPQ_SPLITHEAP, tid, size );
  
  
  //hwpqlib_pq_initialize( HWPQLIB_SPILLPQ_UNITEDLIST, tid, size );
  hwpqlib_pq_initialize(tid, &pqbench_policy[tid], &sparc64_splitrbtree_ops, size);

}

void pq_insert( rtems_task_argument tid, long p ) {
  hwpqlib_insert(tid, kv_key(p), kv_value(p));
}

long pq_first(  rtems_task_argument tid ) {
  return hwpqlib_first( tid );
}

long pq_pop( rtems_task_argument tid ) {
  return hwpqlib_pop( tid );
}

long pq_search( rtems_task_argument tid, int key ) {
  return hwpqlib_search(tid, key);
}

long pq_extract( rtems_task_argument tid, int key ) {
  return hwpqlib_extract(tid, key);
}
