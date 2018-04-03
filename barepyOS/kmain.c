#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"
#include "util.h"
#include "sched.h"

pcb_s *p1, *p2;

void user_process_1()
{
    int v1=5;
    while(1)
    {
        log_str("process 1 ");
        log_int(v1);
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
        log_int(v2);
        log_cr();
        v2-=2;
        sys_yieldto(p1);
    }
}

void kmain() {

	kernel_init();
	log_str("Kernel initialised");
	log_cr();

	SWITCH_TO_USER_MODE();
  log_str("in user mode");
  log_cr();

  p1 = create_process((func_t*) &user_process_1);
  p2 = create_process((func_t*) &user_process_2);

	sys_yieldto(p1);
  //uint64_t ktime = sys_gettime();

  PANIC();

}
