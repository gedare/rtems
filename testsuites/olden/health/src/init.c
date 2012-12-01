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
    "health.exe",
    "3",
    "100",
    "1"
  };
  int argc = 4;
#else
  /* benchmark */
  char *argv[] = {
    "health.exe",
    "5",
    "500",
    "1"
  };
  int argc = 4;
#endif

  printf( "\n\n*** health ***\n" );
  main(argc, argv);
  printf( "*** end of health ***\n" );
  exit( 0 );
}

