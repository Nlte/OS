#include "sched.h"
#include "syscall.h"
#include "kheap.h"
#include "util.h"
#include "hw.h"
#include "asm_tools.h"
#include "config.h"

#include <stdint.h>

pcb_s kmain_process;
pcb_s* current_process;

void sched_init() {
  nprocess = 1;

  current_process = &kmain_process;
}

// Syscall : yield to __________________________________________________________
void sys_yieldto(pcb_s* dest) {
  __asm("mov r1, %0" : : "r"(dest));
  SWI(SID_YIELDTO);
}

void do_sys_yieldto(uint32_t* context) {
  // destination pcb in r1
  pcb_s* dest = (pcb_s*)context[1];
  // Save cpsr user
  //__asm("mrs %0, spsr" : "=r"(current_process->cpsr_user));
  SWITCH_TO_SYSTEM_MODE();
  // Save lr_user and sp_user
  __asm("mov %0, lr" : "=r"(current_process->lr_user));
  //__asm("mov %0, sp" : "=r"(current_process->sp_user));
  SWITCH_TO_SVC_MODE();

  // Backup data registers
  for (int i = 0; i < N_REGISTERS; i++) {
      current_process->r[i] = context[i];
      context[i] = dest->r[i];
  }

  current_process->lr_svc = context[N_REGISTERS];
  context[N_REGISTERS] = dest->lr_svc;

  current_process = dest;

  SWITCH_TO_SYSTEM_MODE();
  // Restore lr user / sp_user
  __asm("mov lr, %0" : : "r"(current_process->lr_user));
  //__asm("mov sp, %0" : : "r"(current_process->sp_user));
  SWITCH_TO_SVC_MODE();
  // Restore cpsr
  //__asm("msr spsr, %0" : : "r"(current_process->cpsr_user));
}


void __attribute__ ((naked)) irq_handler() {
}
