
#include "../shared/pqbench.h"

/* PQ implementation */
#include "mbptree.h"

/* test interface */
void pq_initialize(rtems_task_argument tid, int size ) { 
  bptree_initialize(tid,size);
}

void pq_insert( rtems_task_argument tid, long p ) {
  bptree_insert(tid,p); 
}

long pq_first( rtems_task_argument tid ) {
  return bptree_min(tid);
}

long pq_pop( rtems_task_argument tid  ) {
  return bptree_pop_min(tid);
}

long pq_search( rtems_task_argument tid, int key ) {
  return bptree_search(tid, key);
}

long pq_extract( rtems_task_argument tid, int key ) {
  return bptree_extract(tid, key);
}
