#ifndef TIMER_H
#define TIME_H

#define CORE0_TIMER_IRQ_CNTL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060


#define ENABLE_CNTV() \
  __asm volatile("mcr p15, 0, %0, c14, c3, 1" :: "r"(1));
#define DISABLE_CNTV() \
  __asm volatile("mcr p15, 0, %0, c14, c3, 1" :: "r"(0));


static uint32_t cnt_freq = 0;

void route_cntv_to_irq(void);

uint32_t read_core0timer_src(void);

uint64_t read_cntvct(void);

uint64_t read_cntv_offset(void);

uint32_t read_cntv_tval(void);

void write_cntv_tval(uint32_t date);

uint32_t read_cnt_freq(void);


#endif
