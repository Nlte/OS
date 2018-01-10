#ifndef SYSCALL_H
#define SYSCALL_H
#include <stdint.h>

// SWI macros __________________________________________________________
#define SWI(syscall_id) __asm("mov r0, %0\n" "SWI #0" : : "r"(syscall_id))
#define SAVE_CONTEXT() __asm("stmfd sp!, {r0-r12, lr}")
#define RESTORE_CONTEXT() __asm("ldmfd sp!, {r0-r12, pc}^")

typedef enum {
  SID_REBOOT = 0x01,
  SID_NOP = 0x02,
  SID_SETTIME = 0x03,
  SID_GETTIME = 0x04,
  SID_YIELDTO = 0x05,
} SYSCALL_ID;

// Reboot qemu
void sys_reboot();
// Do nothing
void sys_nop();
// Set time
void sys_settime(uint64_t date_ms);
// Get time
uint64_t sys_gettime();


#endif
