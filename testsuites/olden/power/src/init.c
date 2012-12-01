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
    "power.exe",
    "1",
    "1"
  };
  int argc = 3;
#else
  /* benchmark */
  char *argv[] = {
    "power.exe",
    "1",
    "1"
  };
  int argc = 3;
#endif

  printf( "\n\n*** power ***\n" );
  main(argc, argv);
  printf( "*** end of power ***\n" );
  exit( 0 );
}

