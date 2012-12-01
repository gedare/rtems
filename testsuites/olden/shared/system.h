/*  system.h
 *
 *  This include file contains information that is included in every
 *  function in the test set.
 *
 */

/*
 * Copyright (c) 2012 Gedare Bloom
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#include <rtems.h>
#include <inttypes.h>
#include "tmacros.h"

/* functions */
rtems_task Init(
  rtems_task_argument argument
);

/* global variables */

/* configuration information */
#include <bsp.h> /* for device driver prototypes */

/* drivers */
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

/* tasks */
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

#include <rtems/confdefs.h>

/* end of include file */
