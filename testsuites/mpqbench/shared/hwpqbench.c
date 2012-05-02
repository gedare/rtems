
#include "pqbench.h"

void pq_insert( rtems_task_argument tid, long p ) {
  HWDS_ENQUEUE(tid, kv_key(p), kv_value(p));
}

long pq_first( rtems_task_argument tid ) {
  long kv;
  HWDS_FIRST(tid, kv);
  return kv;
}

long pq_pop( rtems_task_argument tid ) {
  long kv;
  HWDS_FIRST(tid, kv);
  if ( kv != (long)-1 ) {
    long kv2;
    int key = kv_key(kv);
    HWDS_EXTRACT(tid, key, kv2);  // TODO: why not just one op for pop?
  }
  return kv;
}

long pq_search( rtems_task_argument tid, int key ) {
  long kv;
  HWDS_SEARCH(tid, key, kv);
  return kv;
}

long pq_extract( rtems_task_argument tid, int key ) {
  long kv;
  HWDS_EXTRACT(tid, key, kv);
  return kv;
}
