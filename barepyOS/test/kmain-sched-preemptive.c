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

#define NB_PROCESS 5


void finish_line(){
  while (1) {
  }
}

int user_process()
{
    return 0;
}

void kmain( void )
{
    uart_init();
    sched_init();
    kheap_init();
    timer_init();

    int i;
    for(i=0;i<NB_PROCESS;i++)
    {
        create_process(&user_process);
    }

    __asm("cps 0x10"); // switch CPU to USER mode
    // ******************************************

    sys_yield();
    finish_line();
}
