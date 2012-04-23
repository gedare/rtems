/*
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <rtems/io.h>

#include "devfs.h"

int devFS_close(
  rtems_libio_t *iop
)
{
  rtems_libio_open_close_args_t  args;
  rtems_status_code              status;
  const devFS_node *np = iop->pathinfo.node_access;

  args.iop   = iop;
  args.flags = 0;
  args.mode  = 0;

  status = rtems_io_close(
    np->major,
    np->minor,
    (void *) &args
  );

  return rtems_deviceio_errno(status);
}


