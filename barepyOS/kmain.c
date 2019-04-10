#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"
#include "util.h"
#include "sched.h"
#include "timer.h"
#include "syscall.h"
#include "sched.h"
#include "kernel.h"
#include "kheap.h"
#include "vmem.h"

#define NB_PROCESS 5


void finish_line(){
    while (1) {
    }
}

int user_process()
{
    log_str("in user process");
    log_cr();
    return 0;
}

void kmain( void )
{
    kernel_init();
    log_str("Kernel initialised");
    log_cr();
    SWITCH_TO_USER_MODE();
    create_process(user_process); 
    while (1) {}
}
