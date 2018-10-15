#ifndef VMEM_H
#define VMEM_H

#include "sched.h"

#define PAGE_SIZE 4096 // 4 KB (small page size) = 2^12 --> page offset on 12 bits

#define FIRST_LVL_TT_COUNT 4096 // n entries in table level 1
#define FIRST_LVL_TT_SIZE 16384 // (n entries * 4)
#define FIRST_LVL_TT_ALIGN 14

#define SECOND_LVL_TT_COUNT 256 // n entries in table level 2
#define SECOND_LVL_TT_SIZE 1024 // (n entries * 4)
#define SECOND_LVL_TT_ALIGN 10

#define DEVICE_START 0x20000000 // start address for the peripherals (see linker script)
#define DEVICE_END   0x20FFFFFF

// Page flags
/*
0  0  0  000 00 0 0  0  1
NG S APX TEX AP C B    XN
*/
#define TABLE_1_FLAGS 0b000000000001
/*
0  1  0  001 11 0 0  1  1
NG S APX TEX AP C B    XN
*/
#define TABLE_2_FLAGS 0b010001110010
/*
0  1  0  000 11 0 1  1  1
NG S APX TEX AP C B    XN
*/
#define DEVICE_FLAGS 0b010000110111


uint32_t init_kernel_translation_table();
void vmem_init();
uint32_t vmem_translate(uint32_t, pcb_s*, uint32_t);

uint32_t KERNEL_PAGE_TABLE_BASE;

#endif
