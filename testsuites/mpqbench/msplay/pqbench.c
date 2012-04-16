
#include "../shared/pqbench.h"

/* PQ implementation */
#include "msplay.h"

/* test interface */
void pq_initialize(rtems_task_argument tid, int size ) { 
  splay_initialize(tid,size);
}

void pq_insert(rtems_task_argument tid, uint64_t p ) {
  splay_insert(tid,p); 
}

uint64_t pq_first( rtems_task_argument tid ) {
  return splay_min(tid);
}

uint64_t pq_pop( rtems_task_argument tid ) {
  return splay_pop_min(tid);
}

uint64_t pq_search( rtems_task_argument tid, int key ) {
  return splay_search(tid, key);
}

