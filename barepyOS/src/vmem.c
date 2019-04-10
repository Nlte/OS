#include <stddef.h>
#include <stdint.h>
#include "vmem.h"
#include "kheap.h"
#include "sched.h"
#include "math.h"
#include "hw.h"
#include "asm_tools.h"
#include "util.h"

// uint32_t KERNEL_PAGE_TABLE_BASE;
static uint8_t* frame_table;
unsigned int translation_table_base;


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

int get_frame_count_from_size_octet(int size_octet)
{   
    return divide_ceil(size_octet, FRAME_SIZE);
}

uint32_t get_phy_addr_from_offset(int lvl1_offset, int lvl2_offset)
{
    return PAGE_SIZE * (lvl2_offset + lvl1_offset * SECOND_LVL_TT_COUNT);
}

uint32_t get_phy_addr_from_frame_index(int frame_index)
{   
    return frame_index * FRAME_SIZE;
}

int find_next_free_frame(void)
{
    int f;
    for(f = 0; f < FRAME_TABLE_SIZE; ++f){
        if(frame_table[f] == FRAME_FREE){
            return f;
        }
    }
    return -1;
}


void start_mmu_C()
{
    log_str("starting mmu");
    __asm("mcr p15, 0, r1, c1, c0, 0");
    __asm("orr r1, #0x1");
    __asm("mcr p15, 0, r1, c1, c0, 0");
}


void start_mmu_C_v6()
{
    log_str("starting mmu");
    register unsigned int control;
    __asm("mcr p15, 0, %[zero], c1, c0, 0" : : [zero] "r"(0)); //Disable cache
    __asm("mcr p15, 0, r0, c7, c7, 0"); //Invalidate cache (data and instructions) */
    __asm("mcr p15, 0, r0, c8, c7, 0"); //Invalidate TLB entries
    /* Enable ARMv6 MMU features (disable sub-page AP) */
    control = (1<<23) | (1 << 15) | (1 << 4) | 1;
    /* Invalidate the translation lookaside buffer (TLB) */
    __asm volatile("mcr p15, 0, %[data], c8, c7, 0" : : [data] "r" (0));
    /* Write control register */
    __asm volatile("mcr p15, 0, %[control], c1, c0, 0" : : [control] "r" (control));
}

void configure_mmu_C(uint32_t addr_translation_base)
{
    register unsigned int pt_addr = addr_translation_base;
    /* Translation table 0 */
    __asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pt_addr));
    /* Translation table 1 */
    __asm volatile("mcr p15, 0, %[addr], c2, c0, 1" : : [addr] "r" (pt_addr));
    /* Use translation table 0 for everything */
    __asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (0));
    /* Set Domain 0 ACL to "Manager", not enforcing memory permissions
     * Every mapped section/page is in domain 0
     */
    __asm volatile("mcr p15, 0, %[r], c3, c0, 0" : : [r] "r" (0x3));  
}

uint8_t* init_occupation_table(void)
{
    uint8_t *frame_table = (uint8_t*)kAlloc(FRAME_TABLE_SIZE);
    unsigned int i;
    unsigned int frame_kernel_heap_end = divide(__kernel_heap_end__, FRAME_SIZE);
    unsigned int frame_device_start = divide(DEVICE_START, FRAME_SIZE);
    unsigned int frame_device_end = divide(DEVICE_END, FRAME_SIZE);
    for (i = 0; i <= frame_kernel_heap_end; ++i){
        frame_table[i] = FRAME_OCCUPIED;
    }
    for (i = frame_kernel_heap_end; i < frame_device_start; ++i){
        frame_table[i] = FRAME_FREE;
    }
    for (i = frame_device_start; i <= frame_device_end; ++i){
        frame_table[i] = FRAME_OCCUPIED;
    }
    return frame_table;
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

uint32_t init_translation_table()
{
    uint32_t *page_table = (uint32_t*) kAlloc_aligned(FIRST_LVL_TT_SIZE, FIRST_LVL_TT_ALIGN);
    // kernel pages
    init_pages(page_table, 0x0, (uint32_t)kernel_heap_limit, TABLE_1_FLAGS, TABLE_2_FLAGS);
    // device pages
    init_pages(page_table, DEVICE_START, DEVICE_END, TABLE_1_FLAGS, DEVICE_FLAGS);
    return (uint32_t)page_table;
}


void vmem_init()
{
    // kheap needs to be initialised already
    translation_table_base = (unsigned int) init_translation_table();
    log_str("translation base done");
    log_cr();
    frame_table = init_occupation_table();
    log_str("frame table done");
    log_cr();
    configure_mmu_C(translation_table_base);
    log_str("mmu configured");
    log_cr();
    //ENABLE_AB();
    log_str("data abort enabled");
    log_cr();
    start_mmu_C();
    log_str("mmu started");
    log_cr();
}

uint8_t* vmem_alloc_for_userland(pcb_s* process, unsigned int size)
{
    uint32_t * table_1_it = process->page_table;
    int lvl_1_oft_ref = 0;
    int lvl_2_oft_ref = 0;
    int ram_overflow_flag = 0;
    int found_flag = 0;
    for (lvl_1_oft_ref = 0; !found_flag && lvl_1_oft_ref < FIRST_LVL_TT_COUNT; ++lvl_1_oft_ref, ++table_1_it)
    {   
        if(!((*table_1_it) & 0x03)){
            found_flag = 1;
            lvl_2_oft_ref = 0;
            break;
        }
        else if(get_phy_addr_from_offset(lvl_1_oft_ref, lvl_2_oft_ref) >= DEVICE_START){
            ram_overflow_flag = 1;
            break;
        }
        else{   
            uint32_t * table_2_it = (uint32_t*)((*table_1_it) & 0xFFFFFC00);
            for (lvl_2_oft_ref = 0; lvl_2_oft_ref < SECOND_LVL_TT_COUNT; ++lvl_2_oft_ref, ++table_2_it){
                if(!((*table_2_it) & 0x03)){
                    found_flag = 1;
                    break;
                }
            }
        }
    }
    if(ram_overflow_flag){   
        return NULL;
    }
    int nb_frame_to_alloc = get_frame_count_from_size_octet(size);
    uint32_t lvl_1_flags = TABLE_1_FLAGS;
    uint32_t lvl_2_flags = TABLE_2_FLAGS;
    int fta;
    int lvl_2_oft, lvl_1_oft;
    uint32_t firstPhysicalAddress = 0;
    for (fta=0; fta < nb_frame_to_alloc; ++fta){
        lvl_2_oft = modulo(lvl_2_oft_ref + fta, SECOND_LVL_TT_COUNT);
        lvl_1_oft = lvl_1_oft_ref + divide(lvl_2_oft_ref + fta, SECOND_LVL_TT_COUNT);
        int ff_ind = find_next_free_frame();
        if(ff_ind == -1){
            return NULL;
        }
        uint32_t f_phy_addr = get_phy_addr_from_frame_index(ff_ind);
        if(firstPhysicalAddress == 0){
            firstPhysicalAddress = f_phy_addr;
        }
        uint32_t * entry_1 = process->page_table + lvl_1_oft;
        uint32_t * entry_2;
        if(!(*entry_1 & 0x03)){
            entry_2 = (uint32_t*)kAlloc_aligned(SECOND_LVL_TT_SIZE, SECOND_LVL_TT_ALIGN); 
            (*entry_1) = (uint32_t)entry_2 | lvl_1_flags;
        }
        else{
            entry_2 = (uint32_t*)((*entry_1) & 0xFFFFFC00);
        }
        *(entry_2 + lvl_2_oft) = f_phy_addr | lvl_2_flags;
        frame_table[ff_ind] = FRAME_OCCUPIED;
    }
    uint32_t modifiedVirtualAddress = 0x0;
    modifiedVirtualAddress |= (lvl_1_oft_ref << 20);
    modifiedVirtualAddress |= (lvl_2_oft_ref << 12);
    uint32_t pageIndex = firstPhysicalAddress & 0xC;
    modifiedVirtualAddress |= pageIndex;
    return (uint8_t*)modifiedVirtualAddress;
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

void __attribute__((naked)) data_handler(void)
{
    uint32_t addr;
    int error;
    // on recupere l'adresse qui a genere l'erreur
    __asm("mrc p15, 0, %0, c6, c0, 0" : "=r"(addr));
    log_str("Data error at addr=");
    log_int((int)addr);
    log_cr();
    // on recupere le code de l'erreur
    __asm("mrc p15, 0, %0, c5, c0, 0" : "=r"(error));
    // on masque sur les 4 bits de poids faible
    error = error & 0b1111; 
    // on identifie l'erreur
    switch(error)
    {
        case TRANSLATION_FAULT: 
            log_str("ERR: Translation fault"); 
            log_cr();
            break;
        case ACCESS_FAULT:      
            log_str("ERR: Access fault"); 
            log_cr();
            break;        
        case PRIVILEGES_FAULT:  
            log_str("ERR: Privileges fault"); 
            log_cr();
            break;
        default:
            log_str("ERR: Unhandled error");
            log_cr();
    }
    PANIC();
}
