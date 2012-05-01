
#include "../shared/pqbench.h"

/* PQ implementation */
#include "mrbtree.h"

/* test interface */
void pq_initialize( rtems_task_argument tid, int size ) { 
  rbtree_initialize(tid, size);
}

void pq_insert( rtems_task_argument tid, long p ) {
  rbtree_insert(tid, p);
}

long pq_first( rtems_task_argument tid ) {
  return rbtree_min(tid);
}

long pq_pop( rtems_task_argument tid ) {
  return rbtree_pop_min(tid);
}

long pq_search( rtems_task_argument tid, int key ) {
  return rbtree_search(tid, key);
}

long pq_extract( rtems_task_argument tid, int key ) {
  return rbtree_extract(tid, key);
}
