#include "config.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "hw.h"
#include "asm_tools.h"
#include "kernel.h"
#include <stdio.h>

#define NB_PROCESS 3

void user_process() {
  int v = 0;
  while(v < 5) {
    v++;
    sys_yield();
  }
  sys_exit(0);
}

void kmain() {

  kernel_init();

  sys_exit(0);

  int i;
  for (i = 0; i < NB_PROCESS; i++) {
    create_process((func_t *)&user_process);
  }

  __asm("cps 0x10");

  while(1){
    sys_yield();
  }
}
