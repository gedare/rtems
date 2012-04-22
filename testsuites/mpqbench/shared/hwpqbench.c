
#include "pqbench.h"

void pq_insert( rtems_task_argument tid, uint64_t p ) {
  HWDS_ENQUEUE(tid, kv_key(p), kv_value(p));
}

uint64_t pq_first( rtems_task_argument tid ) {
  uint64_t kv;
  HWDS_FIRST(tid, kv);
  return kv;
}

uint64_t pq_pop( rtems_task_argument tid ) {
  uint64_t kv;
  HWDS_FIRST(tid, kv);
  if ( kv != (uint64_t)-1 ) {
    uint64_t kv2;
    int key = kv_key(kv);
    HWDS_EXTRACT(tid, key, kv2);  // TODO: why not just one op for pop?
  }
  return kv;
}

uint64_t pq_search( rtems_task_argument tid, int key ) {
  uint64_t kv;
  HWDS_SEARCH(tid, key, kv);
  return kv;
}

uint64_t pq_extract( rtems_task_argument tid, int key ) {
  uint64_t kv;
  HWDS_EXTRACT(tid, key, kv);
  return kv;
}
