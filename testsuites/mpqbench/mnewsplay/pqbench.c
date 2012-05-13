
#include "../shared/pqbench.h"

/* PQ implementation */
#include "mnewsplay.h"

/* test interface */
void pq_initialize(rtems_task_argument tid, int size ) { 
  newsplay_initialize(tid,size);
}

void pq_insert(rtems_task_argument tid, long p ) {
  newsplay_insert(tid,p); 
}

long pq_first( rtems_task_argument tid ) {
  return newsplay_min(tid);
}

long pq_pop( rtems_task_argument tid ) {
  return newsplay_pop_min(tid);
}

long pq_search( rtems_task_argument tid, int key ) {
  return newsplay_search(tid, key);
}

long pq_extract( rtems_task_argument tid, int key ) {
  return newsplay_extract(tid, key);
}
