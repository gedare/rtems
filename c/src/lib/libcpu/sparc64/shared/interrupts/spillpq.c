#include "spillpq.h"

sparc64_spillpq_operations *spillpq_ops = NULL; 
size_t spillpq_queue_max_size[NUM_QUEUES];
hwpq_context_t hwpq_context;

int sparc64_spillpq_hwpq_context_initialize( int hwpq_id )
{
  uint64_t reg;
  HWDS_GET_SIZE_LIMIT(hwpq_id, reg);
  hwpq_context.max_size = reg;
  hwpq_context.current_size = 0;
}

int sparc64_spillpq_initialize( int queue_idx, size_t max_pq_size )
{
  return spillpq_ops->initialize(queue_idx, max_pq_size);
}

int sparc64_spillpq_insert(int queue_idx, uint64_t kv)
{
  // FIXME: update spill count
  return spillpq_ops->insert(queue_idx, kv);
}

uint64_t sparc64_spillpq_first(int queue_idx)
{
  return spillpq_ops->first(queue_idx, 0);
}

uint64_t sparc64_spillpq_pop(int queue_idx)
{
  // fixme: update spill count
  return spillpq_ops->pop(queue_idx, 0);
}

int sparc64_spillpq_handle_extract(int queue_idx, uint64_t kv)
{
  return spillpq_ops->extract(queue_idx, kv);
}

int sparc64_spillpq_handle_spill(int queue_idx)
{
  /* FIXME: make count arg more flexible */
  return spillpq_ops->spill(queue_idx, hwpq_context.max_size/2);
}

int sparc64_spillpq_handle_fill(int queue_idx)
{
  /* FIXME: make count arg more flexible */
  return spillpq_ops->fill(queue_idx, hwpq_context.max_size/2);
}

int sparc64_spillpq_drain( int queue_id )
{
  return spillpq_ops->drain(queue_id,0);
}

int sparc64_spillpq_context_switch( int queue_id)
{
  return spillpq_ops->context_switch(queue_id,0);
}

uint64_t sparc64_spillpq_null_handler(int qid, uint64_t arg)
{
  printk("%d\tNull handler called\n", qid);
  while(1);
}
