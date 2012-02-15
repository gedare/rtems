/*  Init
 *
 *  Input parameters:  NONE
 *
 *  Output parameters:  NONE
 *
 *  $Id$
 */

#define CONFIGURE_INIT
#include "system.h"

#include "workload.h"

rtems_task Init(
  rtems_task_argument ignored
)
{

  /* initialize structures */
  initialize();

  /* start measurement */
  asm volatile("break_start_opal:");

  /* reach PQ steady state */
  warmup();MAGIC(1);

  /* workload */
  work();

  /* stop measurement */
  MAGIC_BREAKPOINT;

  /* unreached when magic is enabled */
  exit( 0 );
}

