#if defined(x86_64) || defined(X86_64)

#include "../runtime.h"

// Atomic functions, with definitions stolen from musl

i32 env_a_ctz_64(u64 x) {
	__asm__( "bsf %1,%0" : "=r"(x) : "r"(x) );
	return x;
}

//static inline int a_ctz_l(unsigned long x)
//{
//	__asm__( "bsf %1,%0" : "=r"(x) : "r"(x) );
//	return x;
//}

INLINE void env_a_and_64(i32 p_off, u64 v) {
    uint64_t* p = get_memory_ptr_void(p_off, sizeof(uint64_t));
    *p &= v;

//	__asm__( "lock ; and %1, %0"
//			 : "=m"(*p) : "r"(v) : "memory" );
}

INLINE void env_a_or_64(i32 p_off, i64 v) {
    assert(sizeof(i64) == sizeof(uint64_t));
    uint64_t* p = get_memory_ptr_void(p_off, sizeof(i64));
    *p |= v;

//	__asm__( "lock ; or %1, %0"
//			 : "=m"(*p) : "r"(v) : "memory" );
}

//static inline void a_or_l(volatile void *p, long v)
//{
//	__asm__( "lock ; or %1, %0"
//		: "=m"(*(long *)p) : "r"(v) : "memory" );
//}
//
//static inline void *a_cas_p(volatile void *p, void *t, void *s)
//{
//	__asm__( "lock ; cmpxchg %3, %1"
//		: "=a"(t), "=m"(*(long *)p) : "a"(t), "r"(s) : "memory" );
//	return t;
//}
//
i32 env_a_cas(i32 p_off, i32 t, i32 s) {
    assert(sizeof(i32) == sizeof(volatile int));
    volatile int* p = get_memory_ptr_void(p_off, sizeof(i32));

	__asm__( "lock ; cmpxchg %3, %1"
		: "=a"(t), "=m"(*p) : "a"(t), "r"(s) : "memory" );
	return t;
}

void env_a_or(i32 p_off, i32 v) {
    assert(sizeof(i32) == sizeof(volatile int));
    volatile int* p = get_memory_ptr_void(p_off, sizeof(i32));
	__asm__( "lock ; or %1, %0"
		: "=m"(*p) : "r"(v) : "memory" );
}

//static inline void a_and(volatile int *p, int v)
//{
//	__asm__( "lock ; and %1, %0"
//		: "=m"(*p) : "r"(v) : "memory" );
//}

i32 env_a_swap(i32 x_off, i32 v)
{
    assert(sizeof(i32) == sizeof(volatile int));
    volatile int* x = get_memory_ptr_void(x_off, sizeof(i32));

	__asm__( "xchg %0, %1" : "=r"(v), "=m"(*x) : "0"(v) : "memory" );
	return v;
}

i32 env_a_fetch_add(i32 x_off, i32 v)
{
    assert(sizeof(i32) == sizeof(volatile int));
    volatile int* x = get_memory_ptr_void(x_off, sizeof(i32));

	__asm__( "lock ; xadd %0, %1" : "=r"(v), "=m"(*x) : "0"(v) : "memory" );
	return v;
}

void env_a_inc(i32 x_off) {
    assert(sizeof(i32) == sizeof(volatile int));
    volatile int* x = get_memory_ptr_void(x_off, sizeof(i32));

	__asm__( "lock ; incl %0" : "=m"(*x) : "m"(*x) : "memory" );
}

void env_a_dec(i32 x_off) {
    assert(sizeof(i32) == sizeof(volatile int));
    volatile int* x = get_memory_ptr_void(x_off, sizeof(i32));

	__asm__( "lock ; decl %0" : "=m"(*x) : "m"(*x) : "memory" );
}

void env_a_store(i32 p_off, i32 x) {
    assert(sizeof(i32) == sizeof(volatile int));
    volatile int* p = get_memory_ptr_void(p_off, sizeof(i32));
	__asm__ __volatile__(
		"mov %1, %0 ; lock ; orl $0,(%%esp)" : "=m"(*p) : "r"(x) : "memory" );
}

void env_do_spin(i32 i) {
	__asm__ __volatile__( "pause" : : : "memory" );
}

void env_do_crash(i32 i) {
    printf("crashing: %d\n", i);
    assert(0);
}

int env_a_ctz_32(i32 x) {
	__asm__( "bsf %1,%0" : "=r"(x) : "r"(x) );
	return x;
}

#endif
