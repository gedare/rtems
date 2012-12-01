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
    "em3d.exe",
    "5",
    "3",
    "30"
  };
  int argc = 4;
#else
  /* benchmark */
  char *argv[] = {
    "em3d.exe",
    "2000",
    "3",
    "100",
    "100"
  };
  int argc = 5;
#endif

  printf( "\n\n*** em3d ***\n" );
  main(argc, argv);
  printf( "*** end of em3d ***\n" );
  exit( 0 );
}

