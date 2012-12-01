/*
 *  Copyright (c) 2012 Gedare Bloom.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#define CONFIGURE_INIT
#include "system.h"

rtems_task Init(
  rtems_task_argument ignored
)
{

#if defined(OLDEN_TEST_MODE)
  /* test */
  char *argv[] = {
    "tsp.exe",
    "15",
    "1"
  };
  int argc = 3;
#else
  /* benchmark */
  char *argv[] = {
    "tsp.exe",
    "100000",
    "0"
  };
  int argc = 3;
#endif

  printf( "\n\n*** tsp ***\n" );
  main(argc, argv);
  printf( "*** end of tsp ***\n" );
  exit( 0 );
}

