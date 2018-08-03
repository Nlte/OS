/*

ARM generic timer
It can schedule events and trigger interrupts based on an incrementing counter value.
The Cortex-A7 MPCore processor provides a set of four timers for each processor in the cluster.

http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0464e/BABIDBIJ.html
- Physical Timer for use in Secure and Non-secure PL1 modes. = CNTP
- Virtual Timer for use in Non-secure PL1 modes. = CNTV
- Physical Timer for use in Hyp mode. = CNTH

*/

#include <stdint.h>
#include "timer.h"
#include "asm_tools.h"
#include "hw.h"

void route_cntv_to_irq(void)
{
    // QA7_rev3.4 p13 bit 3 = 1 to enable CNTV IRQ
    Set32(CORE0_TIMER_IRQ_CNTL, 0x08);
}

uint32_t read_core0timer_src(void)
{
    uint32_t tmp;
    tmp = Get32(CORE0_IRQ_SOURCE);
    return tmp;
}

uint64_t read_cntvct(void)
{
	uint64_t tval;
	__asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (tval));
	return tval;
}

uint32_t read_cntv_tval(void)
{
  uint32_t tval;
  __asm volatile("mrc p15, 0, %0, c14, c3, 0" : "=r"(tval));
  return tval;
}

void write_cntv_tval(uint32_t tval)
{
  __asm volatile("mcr p15, 0, %0, c14, c3, 0" :: "r"(tval));
  return;
}

uint32_t read_cnt_freq(void)
{
  uint32_t freq;
	__asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(freq));
  return freq;
}

uint64_t read_cntv_offset(void)
{
	uint64_t offset;
  __asm volatile("mrrc p15, 4, %Q0, %R0, c14" : "=r" (offset));
	return offset;
}

void timer_init() {
  uint32_t val;
  write_cntv_tval(DEFAULT_CNTV_VAL);
  val = read_cntv_tval();
  log_str("CNTV_TVAL: ");
  log_int(val);
  log_cr();
  route_cntv_to_irq();
  ENABLE_CNTV();
  ENABLE_IRQ();
}
