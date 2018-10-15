#include <stddef.h>
#include "hw.h"
#include "vmem.h"
#include "kheap.h"

void kmain()
{
  kheap_init();
  vmem_init();
  uint32_t res = vmem_translate(0x123456, NULL, KERNEL_PAGE_TABLE_BASE);
  // res = 1193046
  log_int(res);
  log_cr();
}
