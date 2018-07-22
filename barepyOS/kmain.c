#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"
#include "util.h"
#include "sched.h"
#include "timer.h"


void c_halt(){
  return;
}

void kmain() {

  kernel_init();

    uint32_t val;

    write_cntv_tval(DEFAULT_CNTV_VAL);    // clear cntv interrupt and set next 1 sec timer.
    log_str("CNTV_TVAL: ");
    val = read_cntv_tval();
    log_int(val);
    log_cr();

    route_cntv_to_irq();
    ENABLE_CNTV();
    ENABLE_IRQ();

    while (1) {
        c_halt();
    }

}
