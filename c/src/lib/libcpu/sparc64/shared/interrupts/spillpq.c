#include "spillpq.h"

sparc64_spillpq_operations *spillpq_ops = NULL; 
size_t spillpq_queue_max_size[NUM_QUEUES];

int sparc64_spillpq_initialize( int queue_idx, size_t max_pq_size )
{
  return spillpq_ops->initialize(queue_idx, max_pq_size);
}

int sparc64_spillpq_handle_spill(int queue_idx, int count)
{
  return spillpq_ops->spill(queue_idx, count);
}

int sparc64_spillpq_handle_fill(int queue_idx, int count)
{
  return spillpq_ops->fill(queue_idx, count);
}

int sparc64_spillpq_handle_extract(int queue_idx, uint64_t kv)
{
  return spillpq_ops->extract(queue_idx, kv);
}

int sparc64_spillpq_drain( int queue_id )
{
  return spillpq_ops->drain(queue_id,0);
}

int sparc64_spillpq_context_switch( int queue_id)
{
  return spillpq_ops->context_switch(queue_id,0);
}

