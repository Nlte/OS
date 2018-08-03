#include "sched.h"
#include "syscall.h"
#include "kheap.h"
#include "util.h"
#include "hw.h"
#include "asm_tools.h"
#include "config.h"
#include "kheap.h"
#include "timer.h"

#include <stdint.h>

pcb_s kmain_process;

void sched_init() {
  nprocess = 0;
  current_process = &kmain_process;
  current_process->prev = current_process;
  current_process->next = current_process;
  current_process->state = PS_RUNNING;
  current_process->pid = nprocess;
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
  __asm("mrs %0, spsr" : "=r"(current_process->cpsr));
  SWITCH_TO_SYSTEM_MODE();
  // Save lr_user and sp_user
  __asm("mov %0, lr" : "=r"(current_process->lr_user));
  __asm("mov %0, sp" : "=r"(current_process->sp));
  SWITCH_TO_SVC_MODE();

  // data registers
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
  __asm("mov sp, %0" : : "r"(current_process->sp));
  SWITCH_TO_SVC_MODE();
  // Restore cpsr
  __asm("msr spsr, %0" : : "r"(current_process->cpsr));
}

void context_to_pcb_svc(uint32_t* context)
{
  // save context in current pcb
  __asm("mrs %0, spsr" : "=r"(current_process->cpsr));
  SWITCH_TO_SYSTEM_MODE();
  __asm("mov %0, lr" : "=r"(current_process->lr_user));
  __asm("mov %0, sp" : "=r"(current_process->sp));
  SWITCH_TO_SVC_MODE();
  for (int i = 0; i < N_REGISTERS; i++) {
    current_process->r[i] = context[N_REGISTERS];
  }
  current_process->lr_svc = context[N_REGISTERS];
}

void pcb_to_context_svc(uint32_t* context)
{
  // Update context for the next process
  for (int i = 0; i < N_REGISTERS; i++) {
    context[i] = current_process->r[i];
  }
  context[N_REGISTERS] = current_process->lr_svc;
  SWITCH_TO_SYSTEM_MODE();
  __asm("mov lr, %0" : : "r"(current_process->lr_user));
  __asm("mov sp, %0" : : "r"(current_process->sp));
  SWITCH_TO_SVC_MODE();
  __asm("msr spsr, %0" : : "r"(current_process->cpsr));
}

void elect() {
  if (current_process->state == PS_TERMINATED) {
    current_process->prev->next = current_process->next;
    current_process->next->prev = current_process->prev;
    kFree((uint8_t*) current_process, sizeof(pcb_s));
  }
  current_process = current_process->next;
  current_process->state = PS_RUNNING;
}

void sys_yield() {
  SWI(SID_YIELD);
}

void do_sys_yield(uint32_t *context) {
  context_to_pcb_svc(context);
  elect();
  pcb_to_context_svc(context);
}

int sys_exit(int status) {
  __asm("mov r1, %0" : : "r"(status));
  SWI(SID_EXIT);
  return status;
}

void do_sys_exit(uint32_t *context) {
  current_process->state = PS_TERMINATED;
  current_process->exit_status = context[1];
  elect();
  pcb_to_context_svc(context);
}

void start_current_process() {
  current_process->entry();
  sys_exit(0);
}


pcb_s* create_process(func_t* entry) {
  nprocess++;
  pcb_s* pcb = (pcb_s*) kAlloc(sizeof(pcb_s));
  pcb->mem_start = (uint32_t*) kAlloc(SIZE_STACK_PROCESS);
  pcb->sp = (uint32_t*)((uint32_t)pcb->mem_start + SIZE_STACK_PROCESS + 1);
  pcb->entry = entry;
  pcb->lr_user = (uint32_t) &start_current_process;
  pcb->lr_svc = (uint32_t) &start_current_process;
  pcb->cpsr = 0b10000;

  pcb_s* last = current_process;
  while (last->next != &kmain_process) {
    last = last->next;
  }
  pcb->next = &kmain_process;
  pcb->prev = last;
  last->next = pcb;
  pcb->state = PS_IDLE;
  pcb->pid = nprocess;

  log_str("New process pid: ");
  log_int(nprocess);
  log_cr();

  return pcb;
}

void context_to_pcb_irq(uint32_t* context)
{
  // save context in current pcb
  __asm("mrs %0, spsr" : "=r"(current_process->cpsr));
  SWITCH_TO_SYSTEM_MODE();
  __asm("mov %0, lr" : "=r"(current_process->lr_user));
  __asm("mov %0, sp" : "=r"(current_process->sp));
  SWITCH_TO_IRQ_MODE();
  for (int i = 0; i < N_REGISTERS; i++) {
    current_process->r[i] = context[N_REGISTERS];
  }
  current_process->lr_svc = context[N_REGISTERS];
}

void pcb_to_context_irq(uint32_t* context)
{
  // Update context for the next process
  for (int i = 0; i < N_REGISTERS; i++) {
    context[i] = current_process->r[i];
  }
  context[N_REGISTERS] = current_process->lr_svc;
  SWITCH_TO_SYSTEM_MODE();
  __asm("mov lr, %0" : : "r"(current_process->lr_user));
  __asm("mov sp, %0" : : "r"(current_process->sp));
  SWITCH_TO_IRQ_MODE();
  __asm("msr spsr, %0" : : "r"(current_process->cpsr));
}

void __attribute__((naked)) irq_handler() {
  // decrease LR of 4 and push it on stack first otherwise it will
  // be lost
  //--------------------------------------------
  __asm("sub lr, #4");
  SAVE_CONTEXT();
  //--------------------------------------------
  log_str("in irq handler");
  log_cr();
  uint32_t *context;
  __asm("mov %0, sp" : "=r"(context));
  context_to_pcb_irq(context);
  elect();
  pcb_to_context_irq(context);
  write_cntv_tval(DEFAULT_CNTV_VAL);
  // rearm timer
  RESTORE_CONTEXT();
}
