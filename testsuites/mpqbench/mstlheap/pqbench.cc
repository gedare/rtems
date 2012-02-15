
#include "../shared/pqbench.h"

/* PQ implementation */
#include "mstlheap.h"

/* test interface */
void pq_initialize( int size ) { 
 the_heap = new pq_t();
}

void pq_insert( uint64_t p ) {
  the_heap->insert(p);
}

//void pq_extract( pq_node *n ) { heap_remove(n->hIndex); }

uint64_t pq_first( void ) {
  return the_heap->first();
}

uint64_t pq_pop( void ) {
  return the_heap->pop();
}

