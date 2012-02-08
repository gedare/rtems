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
 *  $Id$
 */

#include <rtems.h>

#include <stdio.h>
#include <stdlib.h>

/* fs includes */
#include "FileSystemImage.h"
#include <rtems/untar.h>

/* functions */
rtems_task Init(
  rtems_task_argument argument
);

/* configuration information */
#include <bsp.h> /* for device driver prototypes */

/* drivers */
//#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

/* filesystem */
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 40
#define CONFIGURE_IMFS_MEMFILE_BYTES_PER_BLOCK 512

//#define CONFIGURE_STACK_CHECKER_ENABLED

/* tasks */
//#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_MAXIMUM_TASKS             1
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

/* deal with spilling: add some KB */
/* TODO: make tight
 *  need sizeof(pq_node)*MAX_SPILL/1024
 *  approx: 40*(NUM_NODES)/1024
 *  so 32 is about enough for 800 spill nodes.
 */
#define CONFIGURE_MEMORY_OVERHEAD         (32)

#include <rtems/confdefs.h>
