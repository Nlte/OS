#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"

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

	uint64_t t = sys_gettime();
	log_int((int) t);
	log_cr();


}
