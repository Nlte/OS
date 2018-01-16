#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "util.h"

#define N_REGISTERS 13
#define SIZE_STACK_PROCESS 10240 // 10 Ko

// Function pointer
typedef int (func_t)(void);

// Process control block
struct pcb_s {
  uint32_t r[N_REGISTERS];
  uint32_t lr_user;
  uint32_t lr_svc;
  uint32_t* memory_begin;
  uint32_t* sp;
  uint32_t cpsr;

  struct pcb_s* next;
  struct pcb_s* prev;
};
typedef struct pcb_s pcb_s;

// Pointer on the process currently executed
pcb_s* current_process;

// Init scheduler
void sched_init();
// Yield to known pcb
void sys_yieldto(pcb_s* dest);
void do_sys_yieldto(uint32_t* context);
// Create new process
pcb_s* create_process(func_t* entry);
// Yield to next process
void sys_yield();
void do_sys_yield(uint32_t* context);
void context_to_pcb(uint32_t* context);
void pcb_to_context(uint32_t* context);

#endif
