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
  uint32_t cpsr_user;
  uint32_t *sp_user;
};
typedef struct pcb_s pcb_s;

// Pointer on the process currently executed
pcb_s* current_process;

// Yield to known pcb
void sys_yieldto(pcb_s* dest);
void do_sys_yieldto(uint32_t* context);

// initialise scheduler
void sched_init();

#endif
