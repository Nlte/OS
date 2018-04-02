#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"
#include "util.h"
#include "sched.h"

pcb_s pcb1, pcb2;

pcb_s *p1, *p2;

void user_process_1()
{
    int v1=5;
    while(1)
    {
        log_str("process 1");
        log_cr();
        v1++;
        sys_yieldto(p2);
    }
}

void user_process_2()
{
    int v2=-12;
    while(1)
    {
        log_str("process 2");
        log_cr();
        v2-=2;
        sys_yieldto(p1);
    }
}

void kmain() {

  p1 = &pcb1;
  p2 = &pcb2;

  p1->lr_user = (uint32_t) &user_process_1;
  p2->lr_user = (uint32_t) &user_process_2;
  p1->lr_svc = (uint32_t) &user_process_1;
  p2->lr_svc = (uint32_t) &user_process_2;
  __asm("mrs %0, cpsr" : "=r"(p1->cpsr_user));
  //p1->cpsr_user &= 0b1111111111111111111111101111111; // enable interruptions
  __asm("mrs %0, cpsr" : "=r"(p2->cpsr_user));
  //p2->cpsr_user &= 0b1111111111111111111111101111111; // enable interruptions

	kernel_init();
	log_str("Kernel initialised");
	log_cr();

  sched_init();

	SWITCH_TO_USER_MODE();
  log_str("in user mode");
  log_cr();


	sys_yieldto(p1);
  //uint64_t ktime = sys_gettime();

  PANIC();

}
