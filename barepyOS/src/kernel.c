#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "uart.h"
#include "sched.h"
#include "kheap.h"
#include "timer.h"
#include "vmem.h"

void kernel_init()
{

  uart_init();
  log_str("uart init");
  log_cr();

  log_str("Core 0: ");
  log_int((int) Get32(CORE0_READY));
  log_cr();
  log_str("Core 1: ");
  log_int((int) Get32(CORE1_READY));
  log_cr();
  log_str("Core 2: ");
  log_int((int) Get32(CORE2_READY));
  log_cr();
  log_str("Core 3: ");
  log_int((int) Get32(CORE3_READY));
  log_cr();

  sched_init();
  log_str("scheduler initialised");
  log_cr();
  kheap_init();
  log_str("kheap initialised");
  log_cr();
  /*
  Enable IRQ: timer + cps
  timer_init();
  log_str("irq enabled");
  log_cr();
  */
  #if VMEM
    vmem_init();
    log_str("vmem initialised");
    log_cr();
  #endif

}

void __attribute__ ((naked)) reset_handler() {
  __asm("bl kmain");
}
