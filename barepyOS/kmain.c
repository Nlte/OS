#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"
#include "util.h"
#include "sched.h"

void user_process() {
        int v=0;
        while(v<5) {
        v++;
        log_str("Process ");
        log_int(current_process->pid);
        log_str(" v ");
        log_int(v);
        log_cr();
        sys_yield();
    }
}

void kmain() {

	kernel_init();
	log_str("Kernel initialised");
	log_cr();

	SWITCH_TO_USER_MODE();
  log_str("in user mode");
  log_cr();

  create_process((func_t*) &user_process);
  create_process((func_t*) &user_process);

  while (1){
    sys_yield();
  }

  PANIC();

}
