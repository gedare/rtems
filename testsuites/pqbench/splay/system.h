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

/* tasks */
//#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_MAXIMUM_TASKS             1
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
//#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MAXIMUM_PARTITIONS (40)

#include <rtems/confdefs.h>
