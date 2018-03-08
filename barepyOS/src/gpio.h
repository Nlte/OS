#ifndef __HEADER_GPIO
#define __HEADER_GPIO

#include "hw.h"
// Registres du GPIO (seulement ceux qui nous intéressent)
#define GPIO_FSEL1		0x3F200004u
#define GPIO_SET0		0x3F20001Cu
#define GPIO_CLR0		0x3F200028u
#define GPIO_PUD		(GPIO_BASE + 0x94)
#define GPIO_PUDCLK0	(GPIO_BASE + 0x98)


#endif
