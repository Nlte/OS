#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "util.h"

#define N_REGISTERS 13
#define SIZE_STACK_PROCESS 10240 // 10 Ko

int nprocess;

// Function pointer
typedef int (func_t)(void);

// Process state
typedef enum {
  PS_RUNNING = 0x01,
  PS_STANDBY = 0x02,
  PS_TERMINATED = 0x03
} PROCESS_STATE;

// Process control block
struct pcb_s {
  uint32_t r[N_REGISTERS];
  uint32_t lr_user;
  uint32_t lr_svc;
  uint32_t* memory_begin;
  uint32_t* sp;
  uint32_t cpsr;

  func_t* entry;

  int pid;
  int exit_status;
  uint8_t state;

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
// Terminate process
int sys_exit(int status);
void do_sys_exit(uint32_t* context);
void free_pcb(pcb_s* pcb);

#endif
