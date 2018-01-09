#include "sched.h"
#include "syscall.h"
#include "kheap.h"
#include "util.h"
#include "hw.h"
#include "asm_tools.h"
#include "config.h"

#include <stdint.h>

pcb_s kmain_process;
int n_process;
SCHED_POLICY sched_policy = PREEMPTIVE;

static void roundRobin_sched();
static void preemptive_sched();

static void (*scheduler_elect)();


// Initialize scheduler ________________________________________________________
void sched_init(){
  // Init heap
  kheap_init();

  if(sched_policy == PREEMPTIVE || sched_policy == FPP) {
    // Init IRQ scheduler and timer
    timer_init();
    ENABLE_IRQ();
    scheduler_elect = &preemptive_sched;
  }
  else {
    // By default we set the scheduler to round robin
    scheduler_elect = &roundRobin_sched;
  }

  current_process = &kmain_process;
  current_process->next = current_process;
  current_process->prev = current_process;
  current_process->state = ACTIVE;

  n_process = 1;
}

// Syscall : yield to __________________________________________________________
void sys_yieldto(pcb_s* dest) {
  __asm("mov r1, %0" : : "r"(dest));
  SWI(ID_YIELDTO);
}

void do_sys_yieldto(uint32_t* sp) {
  // pcb destination contained in r1
  pcb_s* dest = (pcb_s*)(*(sp+1));
  // Save cpsr user
  __asm("mrs %0, spsr" : "=r"(current_process->cpsr_user));
  // Save lr_user and sp_user
  SWITCH_TO_SYSTEM_MODE();
  __asm("mov %0, lr" : "=r"(current_process->lr_user));
  __asm("mov %0, sp" : "=r"(current_process->sp_user));
  SWITCH_TO_SVC_MODE();

  // Backup data registers
  for (int i = 0; i < N_REGISTERS; i++) {
      current_process->r[i] = sp[i];
      sp[i] = dest->r[i];
  }

  current_process->lr_svc = sp[N_REGISTERS];
  sp[N_REGISTERS] = dest->lr_svc;

  current_process = dest;

  SWITCH_TO_SYSTEM_MODE();
  // Restore lr user / sp_user
  __asm("mov lr, %0" : : "r"(current_process->lr_user));
  __asm("mov sp, %0" : : "r"(current_process->sp_user));
  SWITCH_TO_SVC_MODE();
  // Restore cpsr
  __asm("msr spsr, %0" : : "r"(current_process->cpsr_user));
}

// Elect process ______________________________________________________________
static void roundRobin_sched() {
  current_process = current_process->next;
}

static void preemptive_sched() {
  return;
}

void elect() {

  if(current_process->state == TERMINATED){
    if(current_process == current_process->next && current_process == current_process->prev) {
      terminate_kernel();
    }
    // Update pointers to remove terminated process in linked list
    current_process->prev->next = current_process->next;
    current_process->next->prev = current_process->prev;
  }
  // Round robin : new process is the next one in our linked list;
  scheduler_elect();
}


// Syscall : yield _____________________________________________________________
void sys_yield() {
  SWI(ID_YIELD);
}

void do_sys_yield(uint32_t* sp) {

  save_context_to_pcb(sp);
  elect();
  load_context_from_pcb(sp);

}

static void start_current_process() {
  current_process->entry();
  sys_exit(0);
}

pcb_s* create_process(func_t* entry) {

  // Init the pcb of the new process
  pcb_s* pcb = (pcb_s*) kAlloc(sizeof(pcb_s));
  pcb->entry = entry;
  pcb->memory_begin = (uint32_t*) kAlloc(SIZE_STACK_PROCESS);
  pcb->sp_user = (uint32_t*) ((uint32_t)pcb->memory_begin + SIZE_STACK_PROCESS + 1);
  pcb->lr_user = (uint32_t) &start_current_process;
  pcb->lr_svc = (uint32_t) &start_current_process;
  pcb->cpsr_user = 0x150;
  pcb->state = ACTIVE;
  // Add the new process at the end of the list
  pcb_s* last_process = current_process;
  while(last_process->next != &kmain_process) {
    last_process = last_process->next;
  }
  last_process->next = pcb;
  pcb->prev = last_process;
  pcb->next = &kmain_process;

  n_process++;
  return pcb;
}

void load_context_from_pcb(uint32_t* sp) {
	for(int i=0; i<N_REGISTERS; i++) {
		sp[i] = current_process->r[i];
	}
	sp[N_REGISTERS] = current_process->lr_svc;

	SWITCH_TO_SYSTEM_MODE();
	__asm("mov lr, %0" : : "r"(current_process->lr_user));
	__asm("mov sp, %0" : : "r"(current_process->sp_user));
  if(sched_policy == PREEMPTIVE || sched_policy == FPP) {
    SWITCH_TO_IRQ_MODE();
  }
  else{
    SWITCH_TO_SVC_MODE();
  }
	__asm("msr spsr, %0" : : "r"(current_process->cpsr_user));
}

void save_context_to_pcb(uint32_t* sp) {
  SWITCH_TO_SYSTEM_MODE();
	__asm("mov %0, lr" : "=r"(current_process->lr_user));
	__asm("mov %0, sp" : "=r"(current_process->sp_user));
  if(sched_policy == PREEMPTIVE || sched_policy == FPP) {
    SWITCH_TO_IRQ_MODE();
  }
  else {
    SWITCH_TO_SVC_MODE();
  }
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr_user));

	for(int i=0; i<N_REGISTERS; i++) {
		current_process->r[i] = sp[i];
	}
	current_process->lr_svc = sp[N_REGISTERS];
}

void __attribute__((naked)) irq_handler() {

  SAVE_CONTEXT();
  uint32_t* sp;
  __asm("mov %0, sp" : "=r"(sp));
	//*(sp + 13) -= 4;
	// Switch context
  save_context_to_pcb(sp);
	elect();
  load_context_from_pcb(sp);
  // Reset the time
	set_next_tick_default();
	ENABLE_TIMER_IRQ();

	RESTORE_CONTEXT();
}
