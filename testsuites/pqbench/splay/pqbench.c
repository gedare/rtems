
#include "../shared/pqbench.h"

/* PQ implementation */
#include "splay.h"

/* test interface */
void pq_initialize( int size ) { 
  splay_initialize(size);
}

void pq_insert( uint64_t p ) {
  splay_insert(p); 
}

uint64_t pq_first( void ) {
  return splay_min();
}

uint64_t pq_pop( void ) {
  return splay_pop_min();
}

