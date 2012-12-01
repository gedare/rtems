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
    "bisort.exe",
    "16",
    "1"
  };
  int argc = 3;
#else
  /* benchmark */
  char *argv[] = {
    "bisort.exe",
    "250000"
  };
  int argc = 2;
#endif

  printf( "\n\n*** bisort ***\n" );
  main(argc, argv);
  printf( "*** end of bisort ***\n" );
  exit( 0 );
}

