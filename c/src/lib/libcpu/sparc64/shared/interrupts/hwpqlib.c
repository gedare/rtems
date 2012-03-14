
#include <hwpqlib.h>

// TODO: extra hwpq state can be tracked here.
hwpqlib_context_t hwpqlib_context = {
  .pq_context = NULL,
  .num_pqs = 0,
  .hwpq_context = {
    .max_size = 0,
    .current_qid = 0
  }
};

void hwpqlib_initialize( int hwpq_id, int num_pqs )
{
  sparc64_spillpq_hwpq_context_initialize(
      hwpq_id,
      &hwpqlib_context.hwpq_context
  );

  hwpqlib_context.pq_context = 
    _Workspace_Allocate(
      num_pqs * sizeof(hwpqlib_pq_context_t)
    );
  if (!hwpqlib_context.pq_context) {
    printk("Unable to allocate hwpqlib_context.pq_context\n");
    while (1);
  }
  hwpqlib_context.num_pqs = num_pqs;
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

