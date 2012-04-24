
#include "../shared/pqbench.h"

/* PQ implementation */
#include "mskiplist.h"

/* test interface */
void pq_initialize(rtems_task_argument tid, int size ) { 
  skiplist_initialize(tid,size);
}

void pq_insert( rtems_task_argument tid, uint64_t p ) {
  skiplist_insert(tid,p); 
}

uint64_t pq_first( rtems_task_argument tid ) {
  return skiplist_min(tid);
}

uint64_t pq_pop( rtems_task_argument tid  ) {
  return skiplist_pop_min(tid);
}

uint64_t pq_search( rtems_task_argument tid, int key ) {
  return skiplist_search(tid, key);
}

uint64_t pq_extract( rtems_task_argument tid, int key ) {
  return skiplist_extract(tid, key);
}
