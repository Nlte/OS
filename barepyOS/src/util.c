#include "util.h"
#include "hw.h"

void
kernel_panic(char* string, int number)
{
    for(;;)
    {
        log_str("Kernel panic");
        log_cr();
        /* do nothing */
    }
}
