#include "kernel.h"
#include "hw.h"
#include "kheap.h"
#include "sched.h"


void kernel_init()
{
  kheap_init();
  hw_init();
  
  sched_init();

  log_str("Kernel initialized");
  log_cr();
}
