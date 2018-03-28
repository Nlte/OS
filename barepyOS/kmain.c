#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hw.h"
#include "syscall.h"
#include "util.h"

uint32_t function_with_locals() {
	uint32_t l1 = 5;
	uint32_t l2 = 3;

	return l1 - l2;
}

void kmain()
{
	SWITCH_TO_SYSTEM_MODE();

	uint64_t clock_value = function_with_locals();

}
