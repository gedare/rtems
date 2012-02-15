
#include "../shared/pqbench.h"

/* PQ implementation */
#include "miheap.h"

/* test interface */
void pq_initialize(rtems_task_argument tid, int size ) { 
  heap_initialize(tid,size);
}

void pq_insert( rtems_task_argument tid, uint64_t p ) {
  heap_insert(tid,p); 
}

//void pq_extract( pq_node *n ) { heap_remove(n->hIndex); }

uint64_t pq_first( rtems_task_argument tid ) {
  return heap_min(tid);
}

uint64_t pq_pop( rtems_task_argument tid  ) {
  return heap_pop_min(tid);
}
