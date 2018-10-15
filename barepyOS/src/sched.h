#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "util.h"

/*
number of data registers on the cortex a7
*/
#define N_REGISTERS 13
/*
10 Ko: 5 locals var + 5 args + 10 words for computation ~ 80 octets for 1 stack frame
=> 100 calls ~ 10 Ko for the whole stack
*/
#define SIZE_STACK_PROCESS 10240

int nprocess;

// user process type
typedef int (func_t) (void);

// Process states
typedef enum {
  PS_RUNNING = 0x01,
  PS_IDLE = 0x02,
  PS_TERMINATED = 0x03
} PROCESS_STATE;

// Process control block
struct pcb_s {
  uint32_t r[N_REGISTERS];
  uint32_t lr_user;
  uint32_t lr_svc;
  uint32_t cpsr;
  uint32_t *mem_start;
  uint32_t *sp;
  func_t *entry;
  uint32_t *page_table;

  struct pcb_s *prev;
  struct pcb_s *next;
  uint8_t state;
  int pid;
  int exit_status;
};
typedef struct pcb_s pcb_s;

// Pointer on the process currently executed
pcb_s* current_process;

void sched_init();
pcb_s* create_process(func_t* entry);
void sys_yieldto(pcb_s* dest);
void do_sys_yieldto(uint32_t* context);
void sys_yield();
void do_sys_yield(uint32_t* context);
int sys_exit(int status);
void do_sys_exit(uint32_t* context);

#endif
