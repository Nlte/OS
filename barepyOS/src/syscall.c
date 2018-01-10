#include "syscall.h"
#include "util.h"
#include "hw.h"
#include "sched.h"
#include <stdint.h>

// Syscall : reboot ____________________________________________________________
void sys_reboot() {
  // put id in r0 (#1)
  SWI(SID_REBOOT);
}

void do_sys_reboot() {
  //Reboot QEMU
  __asm("bl 0x8000");

}

// Syscall : nop _______________________________________________________________
void sys_nop() {
  SWI(SID_NOP);
}

void do_sys_nop() {
  // do nothing
  return;
}

// Syscall : set time __________________________________________________________
void sys_settime(uint64_t date_ms) {
  // Need to split the date (Registers are 32-bit blocks)
  uint32_t msb = (uint32_t)(date_ms >> 32);
  uint32_t lsb = (uint32_t)(date_ms);
  // Store date in r1 and r2
  __asm("mov r1, %0" : : "r"(msb) : "r2", "r1", "r0");
  __asm("mov r2, %0" : : "r"(lsb) : "r2", "r1", "r0");
  // swi call with id 3
  SWI(SID_SETTIME);
}

void do_sys_settime(uint32_t* context) {
  // Concat msb/lsb from r1/r2
  uint32_t msb = context[1];
  uint32_t lsb = context[2];
  uint64_t date_ms = (uint64_t) msb << 32 | lsb;
  // Actually set time
  set_date_ms(date_ms);

  return;
}

// Syscall : get time __________________________________________________________
uint64_t sys_gettime() {
  // Call the get time routine directly
  SWI(SID_GETTIME);
  // Retrieve time from r0, r1
  uint32_t lsb;
  uint32_t msb;
  __asm("mov %0, r1" : "=r"(msb) : : "r0", "r1");
  __asm("mov %0, r2" : "=r"(lsb) : : "r1", "r0");
  uint64_t date_ms = (uint64_t) msb << 32 | lsb;

  return date_ms;
}

void do_sys_gettime(uint32_t* context) {
  // Retrieve date from OS
  uint64_t date_ms = get_date_ms();
  // Split date into 2 block of 32 bits.
  uint32_t msb = (uint32_t)(date_ms >> 32);
  uint32_t lsb = (uint32_t) date_ms;
  context[1] = msb;
  context[2] = lsb;

  return;
}

// SWI handler _________________________________________________________________
void __attribute__((naked)) swi_handler() {

  // Push r0-r12 + lr to the stack
  SAVE_CONTEXT();

  // get the address of the saved registers on the stack
  uint32_t* context;
  __asm("mov %0, sp" : "=r"(context));

  int syscall_id;
  __asm("mov %0, r0" : "=r"(syscall_id));

  switch (syscall_id) {
    case SID_REBOOT: do_sys_reboot(); break;
    case SID_NOP: do_sys_nop(); break;
    case SID_SETTIME: do_sys_settime(context); break;
    case SID_GETTIME: do_sys_gettime(context); break;
    case SID_YIELDTO: do_sys_yieldto(context); break;
    default: PANIC(); break;
  }

  RESTORE_CONTEXT();
}
