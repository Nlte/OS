#ifndef SYSCALL_H
#define SYSCALL_H
#include <stdint.h>

// Reboot qemu
void sys_reboot();
// Do nothing
void sys_nop();
// Set time
void sys_settime(uint64_t date_ms);
// Get time
uint64_t sys_gettime();
// Terminate process
void sys_exit(int status);


#endif
