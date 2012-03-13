
#include <hwpqlib.h>

// TODO: extra hwpq state can be tracked here.

static inline void unitedlist_initialize( int id, int size ) {
  spillpq_ops = &sparc64_unitedlistpq_ops;
  sparc64_spillpq_initialize(id, size);
}

static inline void splitheap_initialize( int id, int size ) {
  spillpq_ops = &sparc64_splitheappq_ops;
  sparc64_spillpq_initialize(id, size);
}

void hwpqlib_initialize( hwpqlib_spillpq_t type, int hwpq_id, int size ) {
  switch(type) {
    case HWPQLIB_SPILLPQ_UNITEDLIST:
      unitedlist_initialize(hwpq_id, size);
      break;

    case HWPQLIB_SPILLPQ_SPLITHEAP:
      splitheap_initialize(hwpq_id, size);
      break;

    default:
      break;
  }
}

void hwpqlib_insert( int pq_id, int key, int value ) {
  HWDS_ENQUEUE(pq_id, key, value);
}

uint64_t hwpqlib_first( int pq_id ) {
  uint64_t kv;
  HWDS_FIRST(pq_id, kv);
  return kv;
}

uint64_t hwpqlib_pop( int pq_id ) {
  uint64_t kv;
  // TODO: why not just one op for pop?
  HWDS_FIRST(pq_id, kv);
  if ( kv != (uint64_t)-1 )
    HWDS_EXTRACT(pq_id, kv);  
  return kv;
}

