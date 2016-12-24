#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

// Kernel Panic utils __________________________________________________________
#define PANIC() do { kernel_panic(__FILE__,__LINE__) ; } while(0)
#define ASSERT(exp) do { if(!(exp)) PANIC(); } while(0)

void kernel_panic(char* string, int number);


// SWI utils __________________________________________________________
#define SWI(syscall_id) __asm("mov r0, %0\n" "SWI #0" : : "r"(syscall_id))
#define SAVE_CONTEXT() __asm("stmfd sp!, {r0-r12, lr}")
#define RESTORE_CONTEXT() __asm("ldmfd sp!, {r0-r12, pc}^")
// Mode handlers
#define SWITCH_TO_USER_MODE() __asm("cps 0b10000")
#define SWITCH_TO_FIQ_MODE() __asm("cps 0b10001")
#define SWITCH_TO_IRQ_MODE() __asm("cps 0b10010")
#define SWITCH_TO_SVC_MODE() __asm("cps 0b10011")
#define SWITCH_TO_ABORT_MODE() __asm("cps 0b10111")
#define SWITCH_TO_SYSTEM_MODE() __asm("cps 0b11111")

typedef enum {
  ID_REBOOT = 0x01,
  ID_NOP = 0x02,
  ID_SETTIME = 0x03,
  ID_GETTIME = 0x04,
  ID_YIELDTO = 0x05,
  ID_YIELD = 0x06,
  ID_EXIT = 0x07
} SYSCALL_ID;

typedef enum {
  TERMINATED = 0x00,
  RUNNING = 0x01,
  ACTIVE = 0x02
} PROCESS_STATE;

typedef enum {
  ROUND_ROBIN = 0x00,
  PREEMPTIVE = 0x01,
  FPP = 0x02
} SCHED_POLICY;

#endif
