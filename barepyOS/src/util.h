#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

// Kernel Panic utils __________________________________________________________
#define PANIC() do { kernel_panic(__FILE__,__LINE__) ; } while(0)
#define ASSERT(exp) do { if(!(exp)) PANIC(); } while(0)

void kernel_panic(char* string, int number);


// Mode handlers
#define SWITCH_TO_USER_MODE() __asm("cps 0b10000")
#define SWITCH_TO_FIQ_MODE() __asm("cps 0b10001")
#define SWITCH_TO_IRQ_MODE() __asm("cps 0b10010")
#define SWITCH_TO_SVC_MODE() __asm("cps 0b10011")
#define SWITCH_TO_ABORT_MODE() __asm("cps 0b10111")
#define SWITCH_TO_SYSTEM_MODE() __asm("cps 0b11111")

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
