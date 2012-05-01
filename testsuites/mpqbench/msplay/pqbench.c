
#include "../shared/pqbench.h"

/* PQ implementation */
#include "msplay.h"

/* test interface */
void pq_initialize(rtems_task_argument tid, int size ) { 
  splay_initialize(tid,size);
}

void pq_insert(rtems_task_argument tid, long p ) {
  splay_insert(tid,p); 
}

long pq_first( rtems_task_argument tid ) {
  return splay_min(tid);
}

long pq_pop( rtems_task_argument tid ) {
  return splay_pop_min(tid);
}

long pq_search( rtems_task_argument tid, int key ) {
  return splay_search(tid, key);
}

long pq_extract( rtems_task_argument tid, int key ) {
  return splay_extract(tid, key);
}
