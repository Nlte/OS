#include <stddef.h>
#include <stdint.h>
#include "vmem.h"
#include "kheap.h"
#include "sched.h"
#include "math.h"
#include "hw.h"

// uint32_t KERNEL_PAGE_TABLE_BASE;

int get_page_index(uint32_t addr)
{
  return divide(addr, PAGE_SIZE);
}

int get_total_page_count(uint32_t phy_addr_start, uint32_t phy_addr_end)
{
  uint32_t range = phy_addr_end - phy_addr_start;
  return divide(range, PAGE_SIZE) + 1;
}

int get_lvl1_offset(int index)
{
  return divide(index, SECOND_LVL_TT_COUNT);
}

int get_lvl2_offset(int index)
{
  return modulo(index, SECOND_LVL_TT_COUNT);
}

uint32_t get_phy_addr_from_offset(int lvl1_offset, int lvl2_offset)
{
  return PAGE_SIZE * (lvl2_offset + lvl1_offset * SECOND_LVL_TT_COUNT);
}


void init_pages(uint32_t *table1_base, uint32_t phy_addr_start, uint32_t phy_addr_end,
                uint32_t lvl1_flags, uint32_t lvl2_flags)
{
  log_str("init pages");
  log_cr();
  int start_index = get_page_index(phy_addr_start); // 0
  log_str("start index: ");
  log_int(start_index);
  log_cr();
  int total_page_count = get_total_page_count(phy_addr_start, phy_addr_end); // 4096
  int lvl1_offset = get_lvl1_offset(start_index); // 0
  int lvl2_offset = get_lvl2_offset(start_index); // 0
  log_str("tt page count: ");
  log_int(total_page_count);
  log_cr();
  log_str("lvl1_offset: ");
  log_int(lvl1_offset);
  log_cr();
  log_str("lvl2_offset: ");
  log_int(lvl2_offset);
  log_cr();
  int count = 0;
  while (count < total_page_count) {
    uint32_t *table2_base = (uint32_t*)kAlloc_aligned(SECOND_LVL_TT_SIZE, SECOND_LVL_TT_ALIGN);
    *(table1_base + lvl1_offset) = (uint32_t)table2_base | lvl1_flags;
    while ((count < total_page_count) && (lvl2_offset < SECOND_LVL_TT_COUNT)) {
      uint32_t phy_addr = get_phy_addr_from_offset(lvl1_offset, lvl2_offset);
      *(table2_base + lvl2_offset) = phy_addr | lvl2_flags;
      lvl2_offset++;
      count++;
    }
    lvl2_offset = 0;
    lvl1_offset++;
  }
  return;
}

uint32_t init_kernel_translation_table()
{
  uint32_t *page_table = (uint32_t*) kAlloc_aligned(FIRST_LVL_TT_SIZE, FIRST_LVL_TT_ALIGN);
  init_pages(page_table, 0x0, (uint32_t)kernel_heap_limit, TABLE_1_FLAGS, TABLE_2_FLAGS);
  init_pages(page_table, DEVICE_START, DEVICE_END, TABLE_1_FLAGS, DEVICE_FLAGS);
  return (uint32_t)page_table;
}


void vmem_init()
{
  KERNEL_PAGE_TABLE_BASE = init_kernel_translation_table();
}

uint32_t vmem_translate(uint32_t va, pcb_s* process, uint32_t tbase)
{
  uint32_t pa; /* The result */
  /* 1st and 2nd table addresses */
  uint32_t table_base;
  uint32_t second_level_table;
  /* Indexes */
  uint32_t first_level_index;
  uint32_t second_level_index;
  uint32_t page_index;
  /* Descriptors */
  uint32_t first_level_descriptor;
  uint32_t* first_level_descriptor_address;
  uint32_t second_level_descriptor;
  uint32_t* second_level_descriptor_address;
  if (process == NULL) {
    //__asm("mrc p15, 0, %[tb], c2, c0, 0" : [tb] "=r"(table_base));
    table_base = tbase;
  }
  else {
    table_base = (uint32_t) process->page_table;
  }
  table_base = table_base & 0xFFFFC000;
  /* Indexes*/
  first_level_index = (va >> 20);
  second_level_index = ((va << 12) >> 24);
  page_index = (va & 0x00000FFF);
  /* First level descriptor */
  first_level_descriptor_address = (uint32_t*) (table_base | (first_level_index << 2));
  first_level_descriptor = *(first_level_descriptor_address);
  /* Translation fault*/
  if (! (first_level_descriptor & 0x3)) {
    return (uint32_t) FORBIDDEN_ADDRESS;
  }
  /* Second level descriptor */
  second_level_table = first_level_descriptor & 0xFFFFFC00;
  second_level_descriptor_address = (uint32_t*) (second_level_table | (second_level_index << 2));
  second_level_descriptor = *((uint32_t*) second_level_descriptor_address);
  /* Translation fault*/
  if (! (second_level_descriptor & 0x3)) {
    return (uint32_t) FORBIDDEN_ADDRESS;
  }
  /* Physical address */
  pa = (second_level_descriptor & 0xFFFFF000) | page_index;
  return pa;
}
