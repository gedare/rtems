
#include "../shared/pqbench.h"

/* PQ implementation */
#include "mskiplist.h"

/* test interface */
void pq_initialize(rtems_task_argument tid, int size ) { 
  skiplist_initialize(tid,size);
}

void pq_insert( rtems_task_argument tid, long p ) {
  skiplist_insert(tid,p); 
}

long pq_first( rtems_task_argument tid ) {
  return skiplist_min(tid);
}

long pq_pop( rtems_task_argument tid  ) {
  return skiplist_pop_min(tid);
}

long pq_search( rtems_task_argument tid, int key ) {
  return skiplist_search(tid, key);
}

long pq_extract( rtems_task_argument tid, int key ) {
  return skiplist_extract(tid, key);
}
