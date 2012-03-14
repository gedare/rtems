
#include "../shared/pqbench.h"

/* PQ implementation */

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/splitheappq.h> /* bad */

/* test interface */
void pq_initialize( int size ) {
  SPARC64_SET_SPLITHEAPPQ_OPERATIONS(4); // FIXME
  sparc64_spillpq_initialize(4, size); // FIXME: PQ number
}

void pq_insert( uint64_t p ) {
  HWDS_ENQUEUE(4, kv_key(p), kv_value(p)); // FIXME: PQ number
}

uint64_t pq_first( void ) {
  uint64_t kv;
  HWDS_FIRST(4, kv); // FIXME: PQ number
  return kv;
}

uint64_t pq_pop( void ) {
  uint64_t kv;
  HWDS_FIRST(4, kv);  // FIXME: PQ number
  if ( kv != (uint64_t)-1 )
    HWDS_EXTRACT(4, kv);  // TODO: why not just one op for pop?
  return kv;
}

