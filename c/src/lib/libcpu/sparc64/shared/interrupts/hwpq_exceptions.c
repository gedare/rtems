
#include "hwpq_exceptions.h"
#include "spillpq.h"
#include <rtems/bspIo.h>

hwpq_context_t hwpq_context_default;

void sparc64_hwpq_initialize()
{
  proc_ptr old;

  _CPU_ISR_install_vector(
    SPARC_ASYNCHRONOUS_TRAP(0x41),
    sparc64_hwpq_exception_handler,
    &old
   );

  _CPU_ISR_install_vector(
    SPARC_ASYNCHRONOUS_TRAP(0x42),
    sparc64_hwpq_exception_handler,
    &old
   );

  _CPU_ISR_install_vector(
    SPARC_ASYNCHRONOUS_TRAP(0x43), /* Failover--emulate */
    sparc64_hwpq_exception_handler,
    &old
   );

  _CPU_ISR_install_vector(
    SPARC_ASYNCHRONOUS_TRAP(0x44),
    sparc64_hwpq_exception_handler,
    &old
   );

  sparc64_spillpq_hwpq_context_initialize(0, &hwpq_context_default);
}

void sparc64_hwpq_drain_queue( int qid ) {
  sparc64_spillpq_drain(qid);
}

void sparc64_hwpq_exception_handler(
    uint64_t vector,
    CPU_Interrupt_frame *istate
)
{
  uint32_t level;
  uint64_t context;
  uint32_t queue_idx;
  uint32_t trap_context;
  int softint_bit = ((vector&~SPARC_SYNCHRONOUS_TRAP_BIT_MASK) - 0x40);
  int mask = (1<<softint_bit);
//  int synchronous = vector & SPARC_SYNCHRONOUS_TRAP_BIT_MASK;

  level = sparc_disable_interrupts(); // necessary?

  // acknowledge the interrupt (allows isr to schedule another at this level)
  sparc64_clear_interrupt_bits_reg(mask);

  // get the interrupted state
  HWDS_GET_CONTEXT(context); // get context first!

  HWDS_GET_CURRENT_ID(queue_idx); // what is loaded in hw?

  trap_context = (uint32_t)context;

  /* handle the context switch first */

  switch (softint_bit) {
    case 1:
      sparc64_spillpq_handle_spill(queue_idx);
      break;
    case 2:
      sparc64_spillpq_handle_fill(queue_idx);
      break;
    case 3:
      sparc64_spillpq_handle_failover(queue_idx, trap_context);
      break;
    case 4:
      sparc64_spillpq_context_switch(queue_idx, trap_context);
      break;

    default:
      printk("Invalid softint hwpq exception %d\n", softint_bit);
      break;
  }

  sparc_enable_interrupts(level);
}
