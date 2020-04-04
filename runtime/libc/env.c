/* https://github.com/gwsystems/silverfish/blob/master/runtime/libc/libc_backing.c */
#include "../runtime.h"

i32
env_a_ctz_64(u64 x)
{
	return __builtin_ctzll(x);
}

INLINE void
env_a_and_64(i32 p_off, u64 v)
{
	uint64_t *p = worker_thread_get_memory_ptr_void(p_off, sizeof(uint64_t));
	*p &= v;
}

INLINE void
env_a_or_64(i32 p_off, i64 v)
{
	assert(sizeof(i64) == sizeof(uint64_t));
	uint64_t *p = worker_thread_get_memory_ptr_void(p_off, sizeof(i64));
	*p |= v;
}

i32
env_a_cas(i32 p_off, i32 t, i32 s)
{
	assert(sizeof(i32) == sizeof(volatile int));
	volatile int *p = worker_thread_get_memory_ptr_void(p_off, sizeof(i32));

	return __sync_val_compare_and_swap(p, t, s);
}

void
env_a_or(i32 p_off, i32 v)
{
        assert(sizeof(i32) == sizeof(volatile int));
        volatile int *p = worker_thread_get_memory_ptr_void(p_off, sizeof(i32));
        //__asm__("lock ; or %1, %0" : "=m"(*p) : "r"(v) : "memory");
	*p |= v;
}

i32
env_a_swap(i32 x_off, i32 v)
{
	assert(sizeof(i32) == sizeof(volatile int));
	volatile int *x = worker_thread_get_memory_ptr_void(x_off, sizeof(i32));

	//__asm__("xchg %0, %1" : "=r"(v), "=m"(*x) : "0"(v) : "memory");
	int t = *x;
	*x = v;
	v = t;

	return v;
}

i32
env_a_fetch_add(i32 x_off, i32 v)
{
	assert(sizeof(i32) == sizeof(volatile int));
	volatile int *x = worker_thread_get_memory_ptr_void(x_off, sizeof(i32));

	//__asm__("lock ; xadd %0, %1" : "=r"(v), "=m"(*x) : "0"(v) : "memory");
	return __sync_fetch_and_add(x, v);
}

void
env_a_inc(i32 x_off)
{
	assert(sizeof(i32) == sizeof(volatile int));
	volatile int *x = worker_thread_get_memory_ptr_void(x_off, sizeof(i32));
	(*x)++;

	//__asm__("lock ; incl %0" : "=m"(*x) : "m"(*x) : "memory");
}

void
env_a_dec(i32 x_off)
{
	assert(sizeof(i32) == sizeof(volatile int));
	volatile int *x = worker_thread_get_memory_ptr_void(x_off, sizeof(i32));
	(*x)--;

	//__asm__("lock ; decl %0" : "=m"(*x) : "m"(*x) : "memory");
}

void
env_a_store(i32 p_off, i32 x)
{
	assert(sizeof(i32) == sizeof(volatile int));
	volatile int *p = worker_thread_get_memory_ptr_void(p_off, sizeof(i32));
	*p = x;
	//__asm__ __volatile__("mov %1, %0 ; lock ; orl $0,(%%esp)" : "=m"(*p) : "r"(x) : "memory");
}

int
env_a_ctz_32(i32 x)
{
	return __builtin_ctz(x);
}

void
env_do_spin(i32 i)
{
	printf("nope! not happening: %d\n", i);
	assert(0);
}

void
env_do_crash(i32 i)
{
	printf("crashing: %d\n", i);
	assert(0);
}

void
env_do_barrier(i32 x)
{
	__sync_synchronize();
}

INLINE unsigned long long
env_getcycles(void)
{
	return __getcycles();
}

