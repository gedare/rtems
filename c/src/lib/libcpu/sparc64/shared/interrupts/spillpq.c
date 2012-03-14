#include "spillpq.h"

sparc64_spillpq_operations *spillpq_ops = NULL; 
size_t spillpq_queue_max_size[NUM_QUEUES];
hwpq_context_t *hwpq_context = NULL;

int sparc64_spillpq_hwpq_context_initialize( int hwpq_id, hwpq_context_t *ctx )
{
  if ( hwpq_context ) {
    *ctx = *hwpq_context;
  } else {
    uint64_t reg;
    HWDS_GET_SIZE_LIMIT(hwpq_id, reg);
    ctx->max_size = reg;
  }
  hwpq_context = ctx;
}

int sparc64_spillpq_initialize( int queue_idx, size_t max_pq_size )
{
  int rv;
  rv = spillpq_ops->initialize(queue_idx, max_pq_size);
  return rv;
}

uint64_t sparc64_spillpq_first(int queue_idx, uint64_t ignored)
{
  uint64_t rv;
  rv = spillpq_ops->first(queue_idx, 0);
  return rv;
}

uint64_t sparc64_spillpq_insert(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  rv = spillpq_ops->insert(queue_idx, kv);
  HWDS_ADJUST_SPILL_COUNT(queue_idx, (int32_t)1);
  return rv;
}

uint64_t sparc64_spillpq_extract(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  rv = spillpq_ops->extract(queue_idx, kv);
  if (!rv)
    HWDS_ADJUST_SPILL_COUNT(queue_idx, (int32_t)-1); // adjust spill count.
  return rv;
}

uint64_t sparc64_spillpq_pop(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  rv = spillpq_ops->pop(queue_idx, 0);
  HWDS_ADJUST_SPILL_COUNT(queue_idx, (int32_t)-1); // adjust spill count.
  return rv;
}

int sparc64_spillpq_handle_failover(int queue_idx, uint32_t trap_context)
{
  uint32_t trap_operation;
  uint32_t trap_idx;
  uint64_t kv;
  int rv = 0;
  trap_idx = ((trap_context)&(~0))>>20; // what is trying to be used?
  trap_operation = (trap_context)&~(~0 << (3 + 1)); // what is the op?
  
  HWDS_GET_PAYLOAD(kv);

  switch (trap_operation) {
    case 1:
      rv = sparc64_spillpq_first(queue_idx, kv);
      break;

    case 2:
      rv = sparc64_spillpq_insert(queue_idx, kv);
      break;

    case 3:
      rv = sparc64_spillpq_extract(queue_idx, kv);
      break;

    default:
      printk("Unknown operation to emulate: %d\n", trap_operation);
      break;
  }
  return rv;
}

int sparc64_spillpq_handle_spill(int queue_idx)
{
  /* FIXME: make count arg more flexible */
  int rv;
  rv = spillpq_ops->spill(queue_idx, hwpq_context->max_size/2);
  return rv;
}

int sparc64_spillpq_handle_fill(int queue_idx)
{
  /* FIXME: make count arg more flexible */
  int rv;
  rv = spillpq_ops->fill(queue_idx, hwpq_context->max_size/2);
  return rv;
}

int sparc64_spillpq_drain( int queue_id )
{
  int rv;
  rv = spillpq_ops->drain(queue_id,0);
  return rv;
}

int sparc64_spillpq_context_switch( int from_idx, uint32_t trap_context)
{
  uint32_t trap_operation;
  uint32_t trap_idx;
  int rv;
  
  trap_idx = ((trap_context)&(~0))>>20;
  trap_operation = (trap_context)&~(~0 << (3 + 1));
  
  // FIXME: choose whether or not to context switch.
  rv = spillpq_ops->context_switch(from_idx, 0);
  HWDS_SET_CURRENT_ID(trap_idx);
  hwpq_context->current_qid = trap_idx;
  return rv;
}

uint64_t sparc64_spillpq_null_handler(int qid, uint64_t arg)
{
  printk("%d\tNull handler called\n", qid);
  while(1);
}
