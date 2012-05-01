
#include "miheap.h"

#include <stdlib.h>
#include <tmacros.h>

#include "../shared/params.h"

static node* the_heap[NUM_NODES+1][NUM_APERIODIC_TASKS];
static int heap_current_size[NUM_APERIODIC_TASKS];

static node* the_nodes[NUM_APERIODIC_TASKS];
rtems_chain_control freelist[NUM_APERIODIC_TASKS];

static node *alloc_node(rtems_task_argument tid) {
  node *n = rtems_chain_get_unprotected( &freelist[tid] );
  return n;
}
static void free_node(rtems_task_argument tid, node *n) {
  rtems_chain_append_unprotected( &freelist[tid], n );
}

static inline void swap_entries(rtems_task_argument tid, int a, int b) {
  node *tmp = the_heap[a][tid];
  int tmpIndex = the_heap[a][tid]->hIndex;
  the_heap[a][tid]->hIndex = the_heap[b][tid]->hIndex;
  the_heap[b][tid]->hIndex = tmpIndex;
  the_heap[a][tid] = the_heap[b][tid];
  the_heap[b][tid] = tmp;
}

static void bubble_up( rtems_task_argument tid, int i )
{
  while (i > 1 && the_heap[i][tid]->key < the_heap[HEAP_PARENT(i)][tid]->key) {
    swap_entries (tid, i, HEAP_PARENT(i));
    i = HEAP_PARENT(i);
  }
}

static void bubble_down( rtems_task_argument tid, int i ) {
  int j = 0;

  do {
    j = i;
    if ( HEAP_LEFT(j) <= heap_current_size[tid] ) {
      if (the_heap[HEAP_LEFT(j)][tid]->key < the_heap[i][tid]->key)
        i = HEAP_LEFT(j);
    }
    if ( HEAP_RIGHT(j) <= heap_current_size[tid] ) {
      if (the_heap[HEAP_RIGHT(j)][tid]->key < the_heap[i][tid]->key) 
        i = HEAP_RIGHT(j);
    }
    swap_entries(tid, i,j);
  } while (i != j);
}

void heap_initialize( rtems_task_argument tid, int size ) {
  int i;
  heap_current_size[tid] = 0;

  the_nodes[tid] = (node*)malloc(sizeof(node)*size);
  if ( ! the_nodes[tid] ) {
    printk("failed to alloc nodes\n");
    while(1);
  }

  rtems_chain_initialize_empty ( &freelist[tid] );
  for ( i = 0; i < size; i++ ) {
    rtems_chain_append(&freelist[tid], &the_nodes[tid][i].link);
  }
}

void heap_insert(rtems_task_argument tid, long kv ) {
  node *n = alloc_node(tid);
  n->key = kv_key(kv);
  n->val = kv_value(kv);
  ++heap_current_size[tid];
  the_heap[heap_current_size[tid]][tid] = n;
  n->hIndex = heap_current_size[tid];
  bubble_up(tid, heap_current_size[tid]);
}

void heap_remove( rtems_task_argument tid, int i ) {
  swap_entries(tid, i, heap_current_size[tid]);
  free_node(tid, the_heap[heap_current_size[tid]][tid]);
  --heap_current_size[tid];
  bubble_down(tid, i);
}

void heap_change_key( rtems_task_argument tid, int i, int k ) {
  if (the_heap[i][tid]->key < k) {
    heap_increase_key(tid,i,k);
  } else if (the_heap[i][tid]->key > k) {
    heap_decrease_key(tid,i,k);
  }
}

void heap_decrease_key( rtems_task_argument tid, int i, int k ) {
  the_heap[i][tid]->key = k;
  bubble_up(tid,i);
}

void heap_increase_key( rtems_task_argument tid, int i, int k ) {
  the_heap[i][tid]->key = k;
  bubble_down(tid,i);
}

long heap_min( rtems_task_argument tid ) {
  if (heap_current_size[tid]) {
    return (HEAP_NODE_TO_KV(the_heap[1][tid]));
  }
  return (long)-1; // FIXME: error handling
}

long heap_pop_min( rtems_task_argument tid ) {
  long kv;
  kv = heap_min(tid);
  if ( kv != (long)-1 )
    heap_remove(tid,1);
  return kv;
}

long heap_search( rtems_task_argument tid, int k ) {
  int i;
  for ( i = 1; i <= heap_current_size[tid]; i++ ) {
    if ( the_heap[i][tid]->key == k ) {
      return (HEAP_NODE_TO_KV(the_heap[i][tid]));
    }
  }
  return (long)-1;
}

/* this can be a lot more efficient if the heap node is known/passed */
long heap_extract( rtems_task_argument tid, int k ) {
  int i;
  long kv;
  for ( i = 1; i <= heap_current_size[tid]; i++ ) {
    if ( the_heap[i][tid]->key == k ) {
      kv = HEAP_NODE_TO_KV(the_heap[i][tid]);
      heap_remove( tid, i );
      return kv;
    }
  }
  return (long)-1;
}

