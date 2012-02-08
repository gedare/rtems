/*  Init
 *
 *  This routine is the initialization task for this test program.
 *  It is called from init_exec and has the responsibility for creating
 *  and starting the tasks that make up the test.  If the time of day
 *  clock is required for the test, it should also be set to a known
 *  value by this function.
 *
 *  Input parameters:  NONE
 *
 *  Output parameters:  NONE
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#define CONFIGURE_INIT
#include "system.h"

#include "zlib.h"

char buf_line[100];

rtems_task Init(
  rtems_task_argument ignored
)
{
  FILE *fp;
  int fd;
  gzFile fgz;
  char in_file[] = "/test.txt.gz";
  
  printf("Unpacking tar filesystem\nThis may take awhile...\n");
  if(Untar_FromMemory(0x100000000UL, FileSystemImage_size) != 0) {
    printf("Can't unpack tar filesystem\n");
    exit(1);
  }
  	
  printf( "\n\n*** gzfile test unzipping a gzip file ***\n" );

  fp = fopen(in_file, "r");
  if (!fp) {
    printf("unable to open file: %s\n", in_file);
    exit(1);
  }

  fd = fileno(fp);
  if ( fd < 0 ) {
    printf("Error converting file pointer to file descriptor\n");
    exit(1);
  }

  fgz = gzdopen(fd, "r");
  if (!fgz) {
    printf("Error opening gzfile from descriptor %d\n", fd);
    exit(1);
  }

  while ( gzgets(fgz, buf_line, 100) )
    printf("%s", buf_line);

  gzclose_r(fgz);

  printf( "*** end of gzfile ***\n" );
  exit( 0 );
}

