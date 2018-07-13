#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"
#include "util.h"
#include "sched.h"
#include "timer.h"

void kmain() {

  kernel_init();

    uint32_t val;

    log_str("CNTFRQ  : ");
    cnt_freq = read_cnt_freq();
    log_int(cnt_freq);
    log_cr();

    write_cntv_tval(cnt_freq);    // clear cntv interrupt and set next 1 sec timer.
    log_str("CNTV_TVAL: ");
    val = read_cntv_tval();
    log_int(val);
    log_cr();

    route_cntv_to_irq();
    ENABLE_CNTV();
    ENABLE_IRQ();

    while (1) {
        halt();
    }

}
