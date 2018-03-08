#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"


void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( Get32(UART_FR) & (1 << 5) ) { }
	Set32(UART_DR, c);
}

unsigned char uart_getc()
{
    // Wait for UART to have received something.
    while ( Get32(UART_FR) & (1 << 4) ) { }
    return Get32(UART_DR);
}

void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
}


void kmain()
{

	kernel_init();
	log_str("all cores ready");
	log_cr();

	__asm("ldr r1, =CORE1_READY");
  __asm("ldr r0, [r1]");
  __asm("mov r0, #1");
  __asm("str r0, [r1]");

	while (1)
		uart_putc(uart_getc());
}
