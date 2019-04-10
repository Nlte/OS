#include <stddef.h>
#include "hw.h"
#include "vmem.h"
#include "kheap.h"

void kmain()
{
    unsigned int table_base;
    kheap_init();
    table_base = (unsigned int) init_translation_table();
    uint32_t res = vmem_translate(0x123456, NULL, table_base);
    // res = 1193046
    log_int(res);
    log_cr();
}
