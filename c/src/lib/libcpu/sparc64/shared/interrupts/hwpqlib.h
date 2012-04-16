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

typedef struct {
  int current_size;
  /* threads? */
} hwpqlib_pq_context_t;

typedef struct {
  hwpqlib_pq_context_t *pq_context; // array of pqs
  int num_pqs; // size of pq_context array.

  hwpq_context_t hwpq_context;
} hwpqlib_context_t;

extern hwpqlib_context_t hwpqlib_context;

// initialize the hwpqlib
void hwpqlib_initialize( int hwpq_id, int num_pqs );

// Initialize spillpq callouts
void hwpqlib_pq_initialize( hwpqlib_spillpq_t type, int qid, int size );

// Interposition functions for PQ operations
void hwpqlib_insert( int pq_id, int key, int value );
uint64_t hwpqlib_first( int pq_id );
uint64_t hwpqlib_pop( int pq_id );
uint64_t hwpqlib_search( int pq_id, int key);

