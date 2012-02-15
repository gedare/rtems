
#include "../shared/pqbench.h"

/* PQ implementation */
#include "iheap.h"

/* test interface */
void pq_initialize( int size ) { 
  heap_initialize(size);
}

void pq_insert( uint64_t p ) {
  heap_insert(p); 
}

//void pq_extract( pq_node *n ) { heap_remove(n->hIndex); }

uint64_t pq_first( void ) {
  return heap_min();
}

uint64_t pq_pop( void ) {
  return heap_pop_min();
}

