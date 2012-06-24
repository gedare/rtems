#include "spillpq.h"
#include "gabdebug.h"


spillpq_context_t spillpq[NUM_QUEUES];
uint64_t spillpq_cs_payload[NUM_QUEUES];
uint64_t spillpq_cs_trap_payload[NUM_QUEUES];

hwpq_context_t *hwpq_context = NULL;

int sparc64_spillpq_hwpq_context_initialize( int hwpq_id, hwpq_context_t *ctx )
{
  int i;
  if ( hwpq_context ) {
    *ctx = *hwpq_context;
  } else {
    uint64_t reg;
    HWDS_GET_SIZE_LIMIT(hwpq_id, reg);
    ctx->max_size = reg;
  }
  hwpq_context = ctx;

  for ( i = 0; i < NUM_QUEUES; i++ ) {
    spillpq[i].cs_count = 0;
    spillpq_cs_payload[i] = 0;
    spillpq_cs_trap_payload[i] = (uint64_t)-2;
  }
}

int sparc64_spillpq_initialize(
    int queue_idx,
    spillpq_policy_t *policy,
    sparc64_spillpq_operations* ops,
    size_t max_pq_size
)
{
  int rv;
  spillpq[queue_idx].ops = ops;
  spillpq[queue_idx].policy = *policy;
  rv = spillpq[queue_idx].ops->initialize(queue_idx, max_pq_size);
  return rv;
}

uint64_t sparc64_spillpq_first(int queue_idx, uint64_t ignored)
{
  uint64_t rv;
  rv = spillpq[queue_idx].ops->first(queue_idx, 0);
  return rv;
}

uint64_t sparc64_spillpq_insert(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  rv = spillpq[queue_idx].ops->insert(queue_idx, kv);
  HWDS_ADJUST_SPILL_COUNT(queue_idx, (int32_t)1);
  return rv;
}

uint64_t sparc64_spillpq_extract(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  rv = spillpq[queue_idx].ops->extract(queue_idx, kv);
  if (rv != (uint64_t)-1)
    HWDS_ADJUST_SPILL_COUNT(queue_idx, (int32_t)-1); // adjust spill count.
  return rv;
}

uint64_t sparc64_spillpq_search(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  rv = spillpq[queue_idx].ops->search(queue_idx, kv);
  return rv;
}

uint64_t sparc64_spillpq_pop(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  rv = spillpq[queue_idx].ops->pop(queue_idx, 0);
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
  trap_operation = (trap_context)&~(~0 << (16 + 1)); // what is the op?
  
  HWDS_GET_PAYLOAD(queue_idx, kv);

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

    case 17:
      rv = sparc64_spillpq_search(queue_idx, kv);
      break;

    default:
      printk("Unknown operation to emulate: %d\n", trap_operation);
      break;
  }
  HWDS_SET_TRAP_PAYLOAD(queue_idx, rv);
  return rv;
}

int sparc64_spillpq_handle_spill(int queue_idx)
{
  int rv;
  int amt = spillpq[queue_idx].policy.spill_max;

  if ( amt == 0 )
    rv = spillpq[queue_idx].ops->spill(queue_idx, hwpq_context->max_size/2);
  else
    rv = spillpq[queue_idx].ops->spill(queue_idx, amt);

  return rv;
}

int sparc64_spillpq_handle_fill(int queue_idx)
{
  int rv;
  int amt = spillpq[queue_idx].policy.fill_max;

  if ( amt == 0 )
    rv = spillpq[queue_idx].ops->fill(queue_idx, hwpq_context->max_size/2);
  else
    rv = spillpq[queue_idx].ops->fill(queue_idx, amt);

  return rv;
}

int sparc64_spillpq_drain( int queue_idx )
{
  int rv;
  rv = spillpq[queue_idx].ops->drain(queue_idx, 0);
  return rv;
}

int sparc64_spillpq_context_switch( int from_idx, uint32_t trap_context)
{
  uint32_t trap_operation;
  uint32_t trap_idx;
  Chain_Control *spill_pq;
  Chain_Node *iter;
  int rv = 0;
  int size = 0;
  uint64_t kv;
  
  trap_idx = ((trap_context)&(~0))>>20;
  trap_operation = (trap_context)&~(~0 << (16 + 1));
  
  DPRINTK("context switch\tfrom: %d\tto: %d\tduring: %d\n",
      from_idx, trap_idx, trap_operation);
  
  if ( from_idx < NUM_QUEUES && spillpq[from_idx].ops ) {
    // Policy point: Pinning
    if ( spillpq[from_idx].policy.pinned ) {
      return sparc64_spillpq_handle_failover(trap_idx, trap_context);
    }

    HWDS_GET_CURRENT_SIZE(from_idx, size);
    HWDS_GET_PAYLOAD(from_idx, kv); // SAVE PAYLOAD
    spillpq_cs_payload[from_idx] = kv;
    HWDS_GET_TRAP_PAYLOAD(from_idx, kv); // SAVE PAYLOAD
    spillpq_cs_trap_payload[from_idx] = kv;

    // spill all of from_idx
    rv = spillpq[from_idx].ops->spill(from_idx, size);
    if ( rv != size ) {
      printk("failed to spill whole queue!\n");
    }
    spillpq[from_idx].cs_count = rv;
  }
  if ( trap_idx < NUM_QUEUES && spillpq[trap_idx].ops ) {
    // Policy point: choose how much to fill
    // fill up to cs_count[trap_idx]; for SPILLPQ_POLICY_RT fill all
    // otherwise filling nothing
    HWDS_SET_CURRENT_ID(trap_idx);
    hwpq_context->current_qid = trap_idx;
    if ( spillpq[trap_idx].policy.realtime ) {
      spillpq[trap_idx].ops->fill(trap_idx, spillpq[trap_idx].cs_count);
    } else {
      ; // do nothing
    }
    kv = spillpq_cs_payload[trap_idx]; // RESTORE PAYLOAD
    HWDS_SET_PAYLOAD(trap_idx, kv);
    kv = spillpq_cs_trap_payload[trap_idx];
    HWDS_SET_TRAP_PAYLOAD(trap_idx, kv);
  }
  return rv;
}

uint64_t sparc64_spillpq_null_handler(int qid, uint64_t arg)
{
  printk("%d\tNull handler called\n", qid);
  while(1);
}
