#include "util.h"
#include "syscall.h"
#include "sched.h"

pcb_s *p2;
pcb_s *p1;

void user_process1() {
  int v1 = 5;
  while(1) {
    v1++;
    sys_yieldto(p2);
  }
}

void user_process2() {
  int v2 = 12;
  while (1) {
    v2 -= 2;
    sys_yieldto(p1);
  }
}

void kmain() {
  sched_init();

  p1 = create_process((func_t *) &user_process1);
  p2 = create_process((func_t *) &user_process2);

  SWITCH_TO_USER_MODE();
  sys_yieldto(p1);

  PANIC();
}
