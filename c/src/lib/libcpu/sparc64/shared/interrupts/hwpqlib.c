
#include <hwpqlib.h>

// TODO: extra hwpq state can be tracked here.
hwpqlib_context_t hwpqlib_context;
void hwpqlib_initialize( int hwpq_id )
{
  sparc64_spillpq_hwpq_context_initialize(
      hwpq_id,
      &hwpqlib_context.hwpq_context
  );
}

static inline void unitedlist_initialize( int id, int size ) {
  spillpq_ops = &sparc64_unitedlistpq_ops;
  sparc64_spillpq_initialize(id, size);
}

static inline void splitheap_initialize( int id, int size ) {
  spillpq_ops = &sparc64_splitheappq_ops;
  sparc64_spillpq_initialize(id, size);
}

void hwpqlib_pq_initialize( hwpqlib_spillpq_t type, int qid, int size ) {
  if ( qid == 0 ) /* hack */
    hwpqlib_initialize(qid);

  switch(type) {
    case HWPQLIB_SPILLPQ_UNITEDLIST:
      unitedlist_initialize(qid, size);
      break;

    case HWPQLIB_SPILLPQ_SPLITHEAP:
      splitheap_initialize(qid, size);
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

