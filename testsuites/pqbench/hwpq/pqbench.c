
#include "../shared/pqbench.h"

/* PQ implementation */
#include <libcpu/hwpqlib.h> /* bad */

/* test interface */
void pq_initialize( int size ) {
  hwpqlib_initialize( HWPQLIB_SPILLPQ_UNITEDLIST, 4, size ); // FIXME: PQ number
}

void pq_insert( uint64_t p ) {
  hwpqlib_insert(4, kv_key(p), kv_value(p));// FIXME: PQ number
}

uint64_t pq_first( void ) {
  return hwpqlib_first( 4 ); // FIXME: PQ number
}

uint64_t pq_pop( void ) {
  return hwpqlib_pop( 4 );  // FIXME: PQ number
}

