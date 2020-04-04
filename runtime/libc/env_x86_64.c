#if defined(X86_64) || defined(x86_64)

#include "../runtime.h"

unsigned long long int
__getcycles(void)
{
        unsigned long long int cpu_time_in_cycles = 0;
        unsigned int           cycles_lo;
        unsigned int           cycles_hi;
        __asm__ volatile("rdtsc" : "=a"(cycles_lo), "=d"(cycles_hi));
        cpu_time_in_cycles = (unsigned long long int)cycles_hi << 32 | cycles_lo;

        return cpu_time_in_cycles;
}

#endif
