
#include "../shared/pqbench.h"

/* PQ implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/unitedlistpq.h> /* bad */

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) {
  SPARC64_SET_UNITEDLISTPQ_OPERATIONS(tid);
  sparc64_spillpq_initialize(tid, size);
}

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
  int key;
  HWDS_FIRST(tid, kv);
  if ( kv != (uint64_t)-1 ) {
    uint64_t kv2;
    int key;
    key = kv_key(kv);
    HWDS_EXTRACT(tid, key, kv2);  // TODO: why not just one op for pop?
  }
  return kv;
}

uint64_t pq_search( rtems_task_argument tid, int key ) {
  uint64_t kv;
  HWDS_SEARCH(tid, key, kv);
  if ( kv == (uint64_t)-1 ) {
    HWDS_GET_PAYLOAD(tid, kv);
    if ( kv_key(kv) != key )
      kv = (uint64_t)-1;
  }
  return kv;
}

uint64_t pq_extract( rtems_task_argument tid, int key ) {
  uint64_t kv;
  HWDS_EXTRACT(tid, key, kv);
  if ( kv == (uint64_t)-1 ) {
    HWDS_GET_PAYLOAD(tid, kv);
    if ( kv_key(kv) != key ) {
      kv = (uint64_t)-1;
    }
  }
  return kv;
}
