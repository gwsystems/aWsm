#if defined(AARCH64) || defined(aarch64)

#include "../runtime.h"

unsigned long long int
__getcycles(void)
{
	unsigned long long virtual_timer_value;
	asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
	return virtual_timer_value;
}

#endif
