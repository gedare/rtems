
#include <hwpqlib.h>
#include <rtems/score/wkspace.h>

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
  hwpqlib_pq_context_t *pq_context;
  int i;

  sparc64_spillpq_hwpq_context_initialize(
      hwpq_id,
      &hwpqlib_context.hwpq_context
  );

  pq_context = _Workspace_Allocate(
      num_pqs * sizeof(hwpqlib_pq_context_t)
    );
  if (!pq_context) {
    printk("Unable to allocate hwpqlib_context.pq_context\n");
    while (1);
  }

  // initialize pq contexts
  for ( i = 0; i < num_pqs; i++ ) {
    pq_context[i].current_size = 0;
    pq_context[i].allowed = true;
  }

  hwpqlib_context.pq_context = pq_context;
  hwpqlib_context.num_pqs = num_pqs;
}

void hwpqlib_pq_initialize(int qid, spillpq_policy_t *policy,
    sparc64_spillpq_operations *ops, int size )
{
  sparc64_spillpq_initialize(qid, policy, ops, size);
}

static inline bool is_available( int pq_id ) {
  if ( hwpqlib_context.hwpq_context.current_qid == pq_id ) {
    return true;
  }
}

static inline bool is_allowed( int pq_id ) {
  return hwpqlib_context.pq_context[pq_id].allowed;
}

static inline void evict(int pq_id) {
  if ( is_available(pq_id) ) {
    sparc64_spillpq_context_save(pq_id);
  }
  hwpqlib_context.pq_context[pq_id].allowed = false;
}

static inline void set_size(int pq_id, int size) {
  sparc64_spillpq_hwpq_set_max_size(pq_id, size);
}

static inline int check_access(int pq_id) {
  if ( !is_allowed(pq_id) ) {
    return HWPQLIB_STATUS_NOT_ALLOWED;
  }
  if ( !is_available(pq_id) ) {
    return HWPQLIB_STATUS_NOT_AVAILABLE;
  }
  if (hwpqlib_context.pq_context[pq_id].current_size > hwpqlib_context.hwpq_context.max_size * 3) {
    evict(pq_id);
    return HWPQLIB_STATUS_NOT_ALLOWED;
  }
  if ( spillpq[pq_id].stats.extracts > 0 ) {
    evict(pq_id);
    return HWPQLIB_STATUS_NOT_ALLOWED;
  }
  return HWPQLIB_STATUS_OK;
}

uint64_t hwpqlib_insert( int pq_id, uint64_t kv ) {
  int amt = 1;
  uint64_t rv = 0;
  hwpqlib_status_t status = check_access(pq_id);
  if ( status == HWPQLIB_STATUS_OK )
    HWDS_ENQUEUE(pq_id, kv_key(kv), kv_value(kv));
  else
    rv = sparc64_spillpq_insert(pq_id, kv); // FIXME: isr_disable?
  hwpqlib_context.pq_context[pq_id].current_size++;
  return rv;
}

uint64_t hwpqlib_first( int pq_id, uint64_t kv ) {
  uint64_t rv;
  hwpqlib_status_t status = check_access(pq_id);
  if ( status == HWPQLIB_STATUS_OK )
    HWDS_FIRST(pq_id, rv);
  else
    rv = sparc64_spillpq_first(pq_id, kv);
  return rv;
}

uint64_t hwpqlib_pop( int pq_id, uint64_t kv ) {
  uint64_t rv;
  int key;
  hwpqlib_status_t status = check_access(pq_id);
  // TODO: why not just one op for pop?
  if ( status == HWPQLIB_STATUS_OK ) {
    HWDS_FIRST(pq_id, rv);
    if ( rv != (uint64_t)-1 ) {
      key = kv_key(rv);
      hwpqlib_context.pq_context[pq_id].current_size--;
      HWDS_EXTRACT(pq_id, key, rv);
    }
  } else {
    rv = sparc64_spillpq_pop(pq_id, kv);
  }
  return rv;
}

uint64_t hwpqlib_search( int pq_id, uint64_t kv) {
  uint64_t rv;
  hwpqlib_status_t status = check_access(pq_id);
  if ( status == HWPQLIB_STATUS_OK )
    HWDS_SEARCH(pq_id, kv_key(kv), rv);
  else
    rv = sparc64_spillpq_search(pq_id, kv);
  return rv;
}

uint64_t hwpqlib_extract( int pq_id, uint64_t kv) {
  uint64_t rv;
  hwpqlib_status_t status = check_access(pq_id);
  if ( status == HWPQLIB_STATUS_OK ) {
    HWDS_EXTRACT(pq_id, kv_key(kv), rv);
    if ( rv == (uint64_t)-1 ) {
      HWDS_GET_PAYLOAD(pq_id, rv);
    }
  } else {
    rv = sparc64_spillpq_extract(pq_id, kv);
  }
  return rv;
}

