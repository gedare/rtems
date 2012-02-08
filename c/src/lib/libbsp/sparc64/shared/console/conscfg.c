/*
 *  COPYRIGHT (c) 2010 Eugen Leontie.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#include <bsp.h>

#include <libchip/serial.h>

extern void ofw_write(const char *,const int );
extern void ofw_read(void *,int );

extern uint64_t* real_trap_table;

static void call_ofw_write(const char * buf, const int len) {
   uint64_t curr_tba = 0;
  uintptr_t orig_tba = real_trap_table;
  uint64_t curr_pil = 0;
  uint64_t mask_pil = 0xf;

  /* first mask the pil so we don't miss timer ticks */
  sparc64_get_pil(curr_pil);
  sparc64_set_pil(mask_pil);

  /* now set the trap table (tba) to the firmware */
  sparc64_get_tba(curr_tba);
  sparc64_set_tba(orig_tba);

  /* enter firmware */
	ofw_write(buf, len);

  /* reset tba and pil */
  sparc64_set_tba(curr_tba);
  sparc64_set_pil(curr_pil);
}

int sun4v_console_device_first_open(int major, int minor, void *arg)
{
  return 0;
}

static ssize_t sun4v_console_poll_write(int minor, const char *buf, size_t n)
{
  call_ofw_write(buf, n);
  return 0;
}

void sun4v_console_deviceInitialize (int minor)
{
  
}

int sun4v_console_poll_read(int minor){
  int a;
  ofw_read(&a,1);
  if(a!=0){
    return a>>24;
  }
  return -1;
}

bool sun4v_console_deviceProbe (int minor){
  return true;
}

/*
 *  Polled mode functions
 */
console_fns pooled_functions={
  sun4v_console_deviceProbe,       /* deviceProbe */
  sun4v_console_device_first_open, /* deviceFirstOpen */
  NULL,                            /* deviceLastClose */
  sun4v_console_poll_read,         /* deviceRead */
  sun4v_console_poll_write,        /* deviceWrite */
  sun4v_console_deviceInitialize,  /* deviceInitialize */
  NULL,                            /* deviceWritePolled */
  NULL,                            /* deviceSetAttributes */
  NULL                             /* deviceOutputUsesInterrupts */
};

console_flow sun4v_console_console_flow = {
  NULL, /* deviceStopRemoteTx */
  NULL  /* deviceStartRemoteTx */
};

console_tbl     Console_Configuration_Ports[] = {
  {
    "/dev/ttyS0",                 /* sDeviceName */
    SERIAL_CUSTOM,                /* deviceType */
    &pooled_functions,            /* pDeviceFns */
    NULL,                         /* deviceProbe, assume it is there */
    &sun4v_console_console_flow,  /* pDeviceFlow */
    0,                            /* ulMargin */
    0,                            /* ulHysteresis */
    (void *) NULL,                /* pDeviceParams */
    0,                            /* ulCtrlPort1 */
    0,                            /* ulCtrlPort2 */
    1,                            /* ulDataPort */
    NULL,                         /* getRegister */
    NULL,                         /* setRegister */
    NULL, /* unused */            /* getData */
    NULL, /* unused */            /* setData */
    0,                            /* ulClock */
    0                             /* ulIntVector -- base for port */
  },
};

/*
 *  Declare some information used by the console driver
 */

#define NUM_CONSOLE_PORTS 1

unsigned long  Console_Configuration_Count = NUM_CONSOLE_PORTS;

/* putchar/getchar for printk */

static void bsp_out_char (char c)
{
  call_ofw_write(&c, 1);
}

BSP_output_char_function_type BSP_output_char = bsp_out_char;

static int bsp_in_char( void ){
  int tmp;
  ofw_read( &tmp, 1 ); /* blocks */
  if( tmp != 0 ) {
    return tmp>>24;
  }
  return -1;
}

BSP_polling_getchar_function_type BSP_poll_char = bsp_in_char;

