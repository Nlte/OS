#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "uart.h"
#include "sched.h"

void kernel_init()
{

  uart_init();

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

}

void __attribute__ ((naked)) reset_handler() {
  __asm("bl kmain");
}
