/*  system.h
 *
 *  This include file contains information that is included in every
 *  function in the test set.
 *
 *  COPYRIGHT (c) 1989-2009.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 */

/* Extra workspace for spill structure in KB
 * Spill structure:
 *  pq_node: 2 ptrs and 2 uint32: 192b = 24B
 *  Max nodes: ST_MAX_SIZE - QUEUE_SIZE
 *
 * Upper bound on space: ST_MAX_SIZE * sizeof(pq_node)
 */
#include "../shared/params.h"
#define CONFIGURE_MEMORY_OVERHEAD (16+(NUM_APERIODIC_TASKS*ST_MAX_SIZE * 24)/1000)

#include <rtems/confdefs.h>
