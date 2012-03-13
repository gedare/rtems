/*
 *  Some helper functions for setting up the HWPQ exception handling.
 *  Optional interposition functions for PQ operations.
 */
#include "spillpq.h"
#include "unitedlistpq.h"
#include "splitheappq.h"

// The kinds of hwpq spill structures that are supported.
typedef enum {
  HWPQLIB_SPILLPQ_UNITEDLIST,
  HWPQLIB_SPILLPQ_SPLITHEAP,
  HWPQLIB_SPILLPQ_NONE
} hwpqlib_spillpq_t;

// initialize the hwpqlib
void hwpqlib_initialize( int hwpq_id );

// Initialize spillpq callouts
void hwpqlib_pq_initialize( hwpqlib_spillpq_t type, int pq_id, int size );

// Interposition functions for PQ operations
void hwpqlib_insert( int pq_id, int key, int value );
uint64_t hwpqlib_first( int pq_id );
uint64_t hwpqlib_pop( int pq_id );


