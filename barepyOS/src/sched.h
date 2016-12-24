#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "util.h"

#define N_REGISTERS 13
#define SIZE_STACK_PROCESS 10240

// Function pointer
typedef int (func_t)(void);

// Process control block
struct pcb_s {
  uint32_t r[N_REGISTERS];
  uint32_t lr_user;
  uint32_t lr_svc;
  uint32_t* memory_begin;
  uint32_t* sp_user;
  uint32_t cpsr_user;

  func_t* entry;

  int state;
  int exit_status;

  struct pcb_s* prev;
  struct pcb_s* next;

};
typedef struct pcb_s pcb_s;

// Pointer on the process currently executed
pcb_s* current_process;

// Init scheduler
void sched_init();
// Yield to known pcb
void sys_yieldto(pcb_s* dest);
void do_sys_yieldto(uint32_t* context);
// Yield
void sys_yield();
void do_sys_yield();
// Create pcb based on Function
void create_process(func_t* entry);
// Elect new process
void elect();
// helper functions to load a save context
void load_context_from_pcb(uint32_t* sp);
void save_context_to_pcb(uint32_t* sp);

#endif
