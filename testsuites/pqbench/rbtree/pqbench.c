
#include "../shared/pqbench.h"

/* PQ implementation */
#include "rbtree.h"

/* test interface */
void pq_initialize( int size ) { 
  rbtree_initialize(size);
}

void pq_insert( uint64_t p ) {
  rbtree_insert(p); 
}

uint64_t pq_first( void ) {
  return rbtree_min();
}

uint64_t pq_pop( void ) {
  return rbtree_pop_min();
}

