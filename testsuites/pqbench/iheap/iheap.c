
#include "iheap.h"

#include <stdlib.h>
#include <tmacros.h>

static node* the_heap[NUM_NODES+1];
static int heap_current_size;

#if defined(USE_NEW_FREELIST)

#define MAX_FREELISTS (40)
static rtems_id freelist_id[MAX_FREELISTS];
static int freelist_count;
static int bump_count;
static int node_size;

void free_list_bump(int index)
{
  rtems_status_code status;
  void *space = malloc(bump_count*node_size);
  if ( !space ) {
    printk("Failed to malloc during free_list_bump\n");
  }

  status = rtems_partition_create(
      rtems_build_name('f','r','e','e'),
      space,
      bump_count*node_size,
      node_size,
      RTEMS_DEFAULT_ATTRIBUTES,
      &freelist_id[index]
  );
  directive_failed(status,"partition create");
  freelist_count++;
}

// TODO: make a more generic freelist implementation
static node *alloc_node() {
  node * n;
  rtems_status_code status;
  int i = 0;

  do {
    status = rtems_partition_get_buffer(freelist_id[i], &n);
    i++;
  } while (status != RTEMS_SUCCESSFUL && i < freelist_count);

  if ( status != RTEMS_SUCCESSFUL ) {
    free_list_bump(i);
    status = rtems_partition_get_buffer(freelist_id[i], &n);
    i++;
    directive_failed(status,"partition_get_buffer");
  }
  n->part_id = freelist_id[i-1];

  return n;
}
static void free_node(node *n) {
  rtems_status_code status = rtems_partition_return_buffer(n->part_id, n);
  directive_failed(status,"partition_return_buffer");
}

#else

static node* the_nodes;
rtems_chain_control freelist;
static node *alloc_node() {
  node *n = rtems_chain_get_unprotected( &freelist );
  return n;
}
static void free_node(node *n) {
  rtems_chain_append_unprotected( &freelist, n );
}

#endif

static inline void swap_entries(int a, int b) {
  node *tmp = the_heap[a];
  int tmpIndex = the_heap[a]->hIndex;
  the_heap[a]->hIndex = the_heap[b]->hIndex;
  the_heap[b]->hIndex = tmpIndex;
  the_heap[a] = the_heap[b];
  the_heap[b] = tmp;
}

static void bubble_up( int i )
{
  while ( i > 1 && the_heap[i]->key < the_heap[HEAP_PARENT(i)]->key ) {
    swap_entries (i, HEAP_PARENT(i));
    i = HEAP_PARENT(i);
  }
}

static void bubble_down( int i ) {
  int j = 0;

  do {
    j = i;
    if ( HEAP_LEFT(j) <= heap_current_size ) {
      if (the_heap[HEAP_LEFT(j)]->key < the_heap[i]->key)
        i = HEAP_LEFT(j);
    }
    if ( HEAP_RIGHT(j) <= heap_current_size ) {
      if (the_heap[HEAP_RIGHT(j)]->key < the_heap[i]->key) 
        i = HEAP_RIGHT(j);
    }
    swap_entries(i,j);
  } while (i != j);
}

void heap_initialize( int size ) {
  int i;
  heap_current_size = 0;

#if defined(USE_NEW_FREELIST)
  bump_count = size;
  node_size = sizeof(node);
  free_list_bump(0);
#else
  the_nodes = (node*)malloc(sizeof(node)*size);
  if ( ! the_nodes ) {
    printk("failed to alloc nodes\n");
    while(1);
  }

  rtems_chain_initialize_empty ( &freelist );
  for ( i = 0; i < size; i++ ) {
    rtems_chain_append(&freelist, &the_nodes[i].link);
  }
#endif
}

void heap_insert( uint64_t kv ) {
  node *n = alloc_node();
  n->key = kv_key(kv);
  n->val = kv_value(kv);
  ++heap_current_size;
  the_heap[heap_current_size] = n;
  n->hIndex = heap_current_size;
  bubble_up(heap_current_size);
}

void heap_remove( int i ) {
  swap_entries(i, heap_current_size);
  free_node(the_heap[heap_current_size]);
  --heap_current_size;
  bubble_down(i);
}

void heap_change_key( int i, int k ) {
  if (the_heap[i]->key < k) {
    heap_increase_key(i,k);
  } else if (the_heap[i]->key > k) {
    heap_decrease_key(i,k);
  }
}

void heap_decrease_key( int i, int k ) {
  the_heap[i]->key = k;
  bubble_up(i);
}

void heap_increase_key( int i, int k ) {
  the_heap[i]->key = k;
  bubble_down(i);
}

uint64_t heap_min( ) {
  if (heap_current_size) {
    return (HEAP_NODE_TO_KV(the_heap[1]));
  }
  return (uint64_t)-1; // FIXME: error handling
}

uint64_t heap_pop_min( ) {
  uint64_t kv;
  kv = heap_min();
  if ( kv != (uint64_t)-1 )
    heap_remove(1);
  return kv;
}
