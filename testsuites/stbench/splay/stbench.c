
#include "../shared/pqbench.h"

/* ST implementation */
#include "splay.h"

/* test interface */
void st_initialize(rtems_task_argument tid, int size ) { 
  splay_initialize(tid,size);
}

void st_insert(rtems_task_argument tid, uint64_t p ) {
  splay_insert(tid,p); 
}

uint64_t st_first( rtems_task_argument tid ) {
  return splay_min(tid);
}

uint64_t st_pop( rtems_task_argument tid ) {
  return splay_pop_min(tid);
}

