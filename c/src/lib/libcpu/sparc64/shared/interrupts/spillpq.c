#include "spillpq.h"
#include "gabdebug.h"

spillpq_context_t spillpq[NUM_QUEUES];
uint64_t spillpq_cs_payload[NUM_QUEUES];
uint64_t spillpq_cs_trap_payload[NUM_QUEUES];

#define STAT_ZERO(id, stat) spillpq[id].stats.stat = 0
#define STAT_INC(id, stat) spillpq[id].stats.stat++

hwpq_context_t *hwpq_context = NULL;

int sparc64_spillpq_hwpq_context_initialize( int hwpq_id, hwpq_context_t *ctx )
{
  int i;
  if ( hwpq_context ) {
    *ctx = *hwpq_context; // does this make sense??
  } else {
    uint64_t reg;
    HWDS_GET_SIZE_LIMIT(hwpq_id, reg);
    ctx->max_size = reg;
  }
  hwpq_context = ctx;

  for ( i = 0; i < NUM_QUEUES; i++ ) {
    spillpq[i].cs_count = 0;
    STAT_ZERO(i,spills);
    STAT_ZERO(i,fills);
    STAT_ZERO(i,switches);
    STAT_ZERO(i,firsts);
    STAT_ZERO(i,inserts);
    STAT_ZERO(i,extracts);
    STAT_ZERO(i,searches);
    STAT_ZERO(i,pops);
    spillpq_cs_payload[i] = 0;
    spillpq_cs_trap_payload[i] = (uint64_t)-2;
  }
}


int sparc64_spillpq_hwpq_set_max_size(int queue_idx, int size)
{
  HWDS_SET_SIZE_LIMIT(queue_idx, size); // FIXME: error checking, hwpq ctxt..
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
  STAT_INC(queue_idx, firsts);
  rv = spillpq[queue_idx].ops->first(queue_idx, 0);
  return rv;
}

uint64_t sparc64_spillpq_insert(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  int amt = 1;
  STAT_INC(queue_idx, inserts);
  rv = spillpq[queue_idx].ops->insert(queue_idx, kv);
  HWDS_ADJUST_SPILL_COUNT(queue_idx, amt);
  return rv;
}

uint64_t sparc64_spillpq_extract(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  int amt = -1;
  STAT_INC(queue_idx, extracts);
  rv = spillpq[queue_idx].ops->extract(queue_idx, kv);
  if (rv != (uint64_t)-1)
    HWDS_ADJUST_SPILL_COUNT(queue_idx, amt); // adjust spill count.
  return rv;
}

uint64_t sparc64_spillpq_search(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  STAT_INC(queue_idx, searches);
  rv = spillpq[queue_idx].ops->search(queue_idx, kv);
  return rv;
}

uint64_t sparc64_spillpq_pop(int queue_idx, uint64_t kv)
{
  uint64_t rv;
  int amt = -1;
  STAT_INC(queue_idx, firsts);
  rv = spillpq[queue_idx].ops->pop(queue_idx, 0);
  HWDS_ADJUST_SPILL_COUNT(queue_idx, amt); // adjust spill count.
  return rv;
}

int sparc64_spillpq_handle_failover(int queue_idx, uint32_t trap_context)
{
  uint32_t trap_operation;
  uint64_t kv;
  int rv = 0;
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
  STAT_INC(queue_idx, spills);

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
  STAT_INC(queue_idx, fills);

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

int sparc64_spillpq_context_save(int queue_idx) {
  uint64_t kv;
  int rv;
  int size = 0;
  if ( queue_idx >= 0 && queue_idx < NUM_QUEUES && spillpq[queue_idx].ops ) {
    if ( spillpq[queue_idx].policy.pinned ) {
      return -1;
    }
  } else {
    return 0;
  }
  STAT_INC(queue_idx, switches);
  HWDS_GET_CURRENT_SIZE(queue_idx, size);
  HWDS_GET_PAYLOAD(queue_idx, kv); // SAVE PAYLOAD
  spillpq_cs_payload[queue_idx] = kv;
  HWDS_GET_TRAP_PAYLOAD(queue_idx, kv); // SAVE PAYLOAD
  spillpq_cs_trap_payload[queue_idx] = kv;

  // spill all of queue_idx
  rv = spillpq[queue_idx].ops->spill(queue_idx, size);
  if ( rv != size ) {
    printk("failed to spill whole queue!\n");
  }
  spillpq[queue_idx].cs_count = rv;
  HWDS_SET_CURRENT_ID(-1);
  hwpq_context->current_qid = -1;

  return rv;
}

int sparc64_spillpq_context_restore(int queue_idx) {
  uint64_t kv;
  // Policy point: choose how much to fill
  // fill up to cs_count[queue_idx]; for SPILLPQ_POLICY_RT fill all
  // otherwise filling nothing
  if ( queue_idx >= 0 && queue_idx < NUM_QUEUES && spillpq[queue_idx].ops ) {
  } else {
    return -2;
  }
  if ( spillpq[queue_idx].policy.evicted ) {
    return -1;
  }
  HWDS_SET_CURRENT_ID(queue_idx);
  hwpq_context->current_qid = queue_idx;
  if ( spillpq[queue_idx].policy.realtime ) {
    spillpq[queue_idx].ops->fill(queue_idx, spillpq[queue_idx].cs_count);
  } else {
    ; // do nothing
  }
  kv = spillpq_cs_payload[queue_idx]; // RESTORE PAYLOAD
  HWDS_SET_PAYLOAD(queue_idx, kv);
  kv = spillpq_cs_trap_payload[queue_idx];
  HWDS_SET_TRAP_PAYLOAD(queue_idx, kv);
  return 0;
}

int sparc64_spillpq_context_switch( int from_idx, int to_idx)
{
  int rv = 0;
  int cq = hwpq_context->current_qid;
  if ( from_idx == to_idx )
    return rv;

  if ( cq == to_idx )
    return rv;

  if ( cq >= 0 && cq < NUM_QUEUES && cq != from_idx )
    return -1;

  if ( spillpq[to_idx].policy.evicted ) {
    return -1;
  }

  rv = sparc64_spillpq_context_save(from_idx);
  if ( rv >= 0 )
    rv = sparc64_spillpq_context_restore(to_idx);
  return rv;
}

uint64_t sparc64_spillpq_null_handler(int qid, uint64_t arg)
{
  printk("%d\tNull handler called\n", qid);
  while(1);
}
