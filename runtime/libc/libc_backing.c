#include "../runtime.h"

#define UID 0xFF
#define GID 0xFE

// Elf auxilary vector values
#define AT_NULL		0
#define AT_IGNORE	1
#define AT_EXECFD	2
#define AT_PHDR		3
#define AT_PHENT	4
#define AT_PHNUM	5
#define AT_PAGESZ	6
#define AT_BASE		7
#define AT_FLAGS	8
#define AT_ENTRY	9
#define AT_NOTELF	10
#define AT_UID		11
#define AT_EUID		12
#define AT_GID		13
#define AT_EGID		14
#define AT_CLKTCK	17
#define	AT_SECURE	23
#define AT_BASE_PLATFORM 24
#define AT_RANDOM	25


void wasmf___init_libc(i32 envp, i32 pn);

void stub_init(i32 offset) {
    char program_name[] = "wasm_program";
    i32 program_name_offset = offset;
    strcpy(get_memory_ptr_void(offset, sizeof(program_name)), program_name);
    offset += sizeof(program_name);

    // The construction of this is:
    // evn1, env2, ..., NULL, auxv_n1, auxv_1, auxv_n2, auxv_2 ..., NULL
    i32 env_vec[] = {
        // Env variables would live here, but we don't supply any
        0,
        // We supply nessesary
        AT_PAGESZ,
        WASM_PAGE_SIZE,
        AT_UID,
        UID,
        AT_EUID,
        UID,
        AT_GID,
        GID,
        AT_EGID,
        GID,
        AT_SECURE,
        0,
        AT_RANDOM,
        (i32) rand(), // It's pretty stupid to use rand here, but w/e
        0,
    };
    i32 env_vec_offset = offset;
    memcpy(get_memory_ptr_void(env_vec_offset, sizeof(env_vec)), env_vec, sizeof(env_vec));


    wasmf___init_libc(env_vec_offset, program_name_offset);
}

// Syscall stuff

// We define our own syscall numbers, because WASM uses x86_64 values even on systems that are not x86_64
#define SYS_WRITE 1
i32 wasm_write(i32 fd, i32 buf_offset, i32 buf_size) {
    // TODO: Handle errno
    char* buf = get_memory_ptr_void(buf_offset, buf_size);
    return (i32) write(fd, buf, buf_size);
}

#define SYS_MMAP 9
u32 wasm_mmap(i32 addr, i32 len, i32 prot, i32 flags, i32 fd, i32 offset) {
	if (addr != 0) {
		printf("parameter void *addr is not supported!\n");
		assert(0);
	}

	if (fd != -1) {
		printf("file mapping is not supported!\n");
		assert(0);
	}

    assert(len % WASM_PAGE_SIZE == 0);

    i32 result = memory_size;
    for (int i = 0; i < len / WASM_PAGE_SIZE; i++) {
        expand_memory();
    }

    return result;
}

#define SYS_MUNMAP 11

#define SYS_BRK 12
i32 wasm_brk(i32 addr) {
    return 0;
}

#define SYS_IOCTL 16
i32 wasm_ioctl(i32 fd, i32 request, i32 data_offet) {
	/* musl libc does some ioctls to stdout, so just allow these to silently go through */
	if (fd == 1 || fd == 2) return 0;

	printf("ioctl on fd(%d) not implemented\n", fd);
	assert(0);
	return 0;
}

#define SYS_WRITEV 20
struct wasm_iovec {
    i32 base_offset;
    i32 len;
};

i32 wasm_writev(i32 fd, i32 iov_offset, i32 iovcnt) {
    i32 written = 0;
    struct wasm_iovec *iov = get_memory_ptr_void(iov_offset, iovcnt * sizeof(struct wasm_iovec));
    for (int i = 0; i < iovcnt; i++) {
        written += wasm_write(fd, iov[i].base_offset, iov[i].len);
    }
    return written;
}

#define SYS_MADVISE 28

#define SYS_SET_THREAD_AREA 205

#define SYS_SET_TID_ADDRESS 218

#define SYS_EXIT_GROUP 231
i32 wasm_exit_group(i32 status) {
    exit(status);
    return 0;
}

i32 env_syscall_handler(i32 n, i32 a, i32 b, i32 c, i32 d, i32 e, i32 f) {
    switch(n) {
        case SYS_WRITE: return wasm_write(a, b, c);
        case SYS_MMAP: return wasm_mmap(a, b, c, d, e, f);
        case SYS_MUNMAP: return 0;
        case SYS_BRK: return wasm_brk(a);
        case SYS_IOCTL: return wasm_ioctl(a, b, c);
        case SYS_WRITEV: return wasm_writev(a, b, c);
        case SYS_MADVISE: return 0;
        case SYS_SET_THREAD_AREA: return 0;
        case SYS_SET_TID_ADDRESS: return 0;
        case SYS_EXIT_GROUP: return wasm_exit_group(a);
    }
    printf("syscall %d (%d, %d, %d, %d, %d, %d)\n", n, a, b, c, d, e, f);
    assert(0);
    return 0;
}

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

void env_a_and_64(i32 p_off, u64 v) {
    volatile uint64_t* p = get_memory_ptr_void(p_off, sizeof(volatile uint64_t));

	__asm__( "lock ; and %1, %0"
			 : "=m"(*p) : "r"(v) : "memory" );
}

void env_a_or_64(i32 p_off, i64 v) {
    assert(sizeof(i64) == sizeof(volatile uint64_t));
    volatile uint64_t* p = get_memory_ptr_void(p_off, sizeof(i64));

	__asm__( "lock ; or %1, %0"
			 : "=m"(*p) : "r"(v) : "memory" );
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
	__asm__( "mov %1, %0 ; lock ; orl $0,(%%rsp)" : "=m"(*p) : "r"(x) : "memory" );

}

void env_do_spin(i32 i) {
	__asm__ __volatile__( "pause" : : : "memory" );
}

void env_do_crash(i32 i) {
    printf("crashing: %d\n", i);
    assert(0);
}
