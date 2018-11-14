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
    strcpy(get_memory_ptr_for_runtime(offset, sizeof(program_name)), program_name);
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
    memcpy(get_memory_ptr_for_runtime(env_vec_offset, sizeof(env_vec)), env_vec, sizeof(env_vec));

    switch_out_of_runtime();
    wasmf___init_libc(env_vec_offset, program_name_offset);
    switch_into_runtime();
}

// Syscall stuff

// We define our own syscall numbers, because WASM uses x86_64 values even on systems that are not x86_64
#define SYS_READ 0
u32 wasm_read(i32 filedes, i32 buf_offset, i32 nbyte) {
    char* buf = get_memory_ptr_void(buf_offset, nbyte);
    i32 res = (i32) read(filedes, buf, nbyte);

    if (res == -1) {
        return -errno;
    } else {
        return res;
    }
}

#define SYS_WRITE 1
i32 wasm_write(i32 fd, i32 buf_offset, i32 buf_size) {
    char* buf = get_memory_ptr_void(buf_offset, buf_size);
    i32 res = (i32) write(fd, buf, buf_size);

    if (res == -1) {
        return -errno;
    }
    return res;
}

#define SYS_OPEN 2
i32 wasm_open(i32 path_off, i32 flags, i32 mode) {
    // TODO: Handle string being bad
    char* path = get_memory_ptr_void(path_off, 0);
    i32 res = (i32) open(path, flags, mode);

    if (res == -1) {
        return -errno;
    }
    return res;

}

#define SYS_CLOSE 3
i32 wasm_close(i32 fd) {
    i32 res = (i32) close(fd);

    if (res == -1) {
        return -errno;
    }
    return res;
}

struct wasm_stat {
	i64 st_dev;
	u64 st_ino;
	u32 st_nlink;

	u32 st_mode;
	u32 st_uid;
	u32 st_gid;
	u32 __pad0;
	u64 st_rdev;
	u64 st_size;
	i32 st_blksize;
	i64 st_blocks;

	struct { i32 tv_sec; i32 tv_nsec; } st_atim;
	struct { i32 tv_sec; i32 tv_nsec; } st_mtim;
	struct { i32 tv_sec; i32 tv_nsec; } st_ctim;
	i32 __pad1[3];
};

//     struct stat { /* when _DARWIN_FEATURE_64_BIT_INODE is NOT defined */
//         dev_t    st_dev;    /* device inode resides on */
//         ino_t    st_ino;    /* inode's number */
//         mode_t   st_mode;   /* inode protection mode */
//         nlink_t  st_nlink;  /* number of hard links to the file */
//         uid_t    st_uid;    /* user-id of owner */
//         gid_t    st_gid;    /* group-id of owner */
//         dev_t    st_rdev;   /* device type, for special file inode */
//         struct timespec st_atimespec;  /* time of last access */
//         struct timespec st_mtimespec;  /* time of last data modification */
//         struct timespec st_ctimespec;  /* time of last file status change */
//         off_t    st_size;   /* file size, in bytes */
//         quad_t   st_blocks; /* blocks allocated for file */
//         u_long   st_blksize;/* optimal file sys I/O ops blocksize */
//         u_long   st_flags;  /* user defined flags for file */
//         u_long   st_gen;    /* file generation number */
//     };


#define SYS_FSTAT 5
i32 wasm_fstat(i32 filedes, i32 stat_offset) {
    struct wasm_stat* stat_ptr = get_memory_ptr_void(stat_offset, sizeof(struct wasm_stat));

    struct stat stat;
    i32 res = fstat(filedes, &stat);

    if (res == 0) {
        *stat_ptr = (struct wasm_stat) {
            .st_dev = stat.st_dev,
            .st_ino = stat.st_ino,
            .st_nlink = stat.st_nlink,
            .st_mode = stat.st_mode,
            .st_uid = stat.st_uid,
            .st_gid = stat.st_gid,
            .st_rdev = stat.st_rdev,
            .st_size = stat.st_size,
            .st_blksize = stat.st_blksize,
            .st_blocks = stat.st_blocks,
            // TODO: Also copy access times
        };
    } else if (res == -1) {
        return -errno;
    }
    return res;
}

#define SYS_LSEEK 8
i32 wasm_lseek(i32 filedes, i32 file_offset, i32 whence) {
    i32 res = (i32) lseek(filedes, file_offset, whence);

    if (res == -1) {
        return -errno;
    }
    return res;
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
        switch_out_of_runtime();
        expand_memory();
        switch_into_runtime();
    }

    return result;
}

#define SYS_MUNMAP 11

#define SYS_BRK 12

#define SYS_RT_SIGACTION 13


#define SYS_RT_SIGPROGMASK 14
//i32 wasm_sigprocmask(i32 how, i32 set_ptr, i32 oset_ptr) {
//    return 0;
//}

#define SYS_IOCTL 16
i32 wasm_ioctl(i32 fd, i32 request, i32 data_offet) {
	/* musl libc does some ioctls to stdout, so just allow these to silently go through */
	return 0;
}

#define SYS_READV 19
struct wasm_iovec {
    i32 base_offset;
    i32 len;
};

i32 wasm_readv(i32 fd, i32 iov_offset, i32 iovcnt) {
    i32 read = 0;
    struct wasm_iovec *iov = get_memory_ptr_void(iov_offset, iovcnt * sizeof(struct wasm_iovec));
    for (int i = 0; i < iovcnt; i++) {
        read += wasm_read(fd, iov[i].base_offset, iov[i].len);
    }
    return read;
}

#define SYS_WRITEV 20
i32 wasm_writev(i32 fd, i32 iov_offset, i32 iovcnt) {
    struct wasm_iovec *iov = get_memory_ptr_void(iov_offset, iovcnt * sizeof(struct wasm_iovec));

    // If we aren't on MUSL, pass writev to printf if possible
    #if defined(__APPLE__) || defined(__GLIBC__)
    if (fd == 1) {
        int sum = 0;
        for (int i = 0; i < iovcnt; i++) {
            i32 len = iov[i].len;
            void* ptr = get_memory_ptr_void(iov[i].base_offset, len);

            printf("%.*s", len, ptr);
            sum += len;
        }
        return sum;
    }
    #endif

    struct iovec vecs[iovcnt];
    for (int i = 0; i < iovcnt; i++) {
        i32 len = iov[i].len;
        void* ptr = get_memory_ptr_void(iov[i].base_offset, len);
        vecs[i] = (struct iovec) {ptr, len};
    }

    i32 res = (i32) writev(fd, vecs, iovcnt);
    if (res == -1) {
        return -errno;
    }
    return res;
}

#define SYS_MADVISE 28

#define SYS_SET_THREAD_AREA 205

#define SYS_SET_TID_ADDRESS 218

#define SYS_GET_TIME 228
struct wasm_time_spec {
    u32 sec;
    u32 nanosec;
};

i32 wasm_get_time(i32 clock_id, i32 timespec_off) {
    clockid_t real_clock;
    switch (clock_id) {
        case 0:
            real_clock = CLOCK_REALTIME;
            break;
        case 1:
            real_clock = CLOCK_MONOTONIC;
            break;
        case 2:
            real_clock = CLOCK_PROCESS_CPUTIME_ID;
            break;
        default:
            assert(0);
    }

    struct wasm_time_spec* timespec = get_memory_ptr_void(timespec_off, sizeof(struct wasm_time_spec));

    struct timespec native_timespec = { 0, 0 };
    int res = clock_gettime(real_clock, &native_timespec);
    if (res == -1) {
        return -errno;
    }

    timespec->sec = native_timespec.tv_sec;
    timespec->nanosec = native_timespec.tv_nsec;
    return res;
}

#define SYS_EXIT_GROUP 231
i32 wasm_exit_group(i32 status) {
    exit(status);
    return 0;
}

i32 inner_syscall_handler(i32 n, i32 a, i32 b, i32 c, i32 d, i32 e, i32 f) {
    i32 res;
    switch(n) {
        case SYS_READ: return wasm_read(a, b, c);
        case SYS_WRITE: return wasm_write(a, b, c);
        case SYS_OPEN: return wasm_open(a, b, c);
        case SYS_CLOSE: return wasm_close(a);
        case SYS_FSTAT: return wasm_fstat(a, b);
        case SYS_LSEEK: return wasm_lseek(a, b, c);
        case SYS_MMAP: return wasm_mmap(a, b, c, d, e, f);
        case SYS_MUNMAP: return 0;
        case SYS_BRK: return 0;
        case SYS_RT_SIGACTION: return 0;
        case SYS_RT_SIGPROGMASK: return 0;
        case SYS_IOCTL: return wasm_ioctl(a, b, c);
        case SYS_READV: return wasm_readv(a, b, c);
        case SYS_WRITEV: return wasm_writev(a, b, c);
        case SYS_MADVISE: return 0;
        case SYS_SET_THREAD_AREA: return 0;
        case SYS_SET_TID_ADDRESS: return 0;
        case SYS_GET_TIME: return wasm_get_time(a, b);
        case SYS_EXIT_GROUP: return wasm_exit_group(a);
    }
    printf("syscall %d (%d, %d, %d, %d, %d, %d)\n", n, a, b, c, d, e, f);
    assert(0);
    return 0;
}

i32 env_syscall_handler(i32 n, i32 a, i32 b, i32 c, i32 d, i32 e, i32 f) {
    switch_into_runtime();
    i32 i  = inner_syscall_handler(n, a, b, c, d, e, f);
    switch_out_of_runtime();
    return i;
}

i32 env___syscall(i32 n, i32 a, i32 b, i32 c, i32 d, i32 e, i32 f) {
    return env_syscall_handler(n, a, b, c, d, e, f);
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


// Floating point routines
// TODO: Do a fair comparison between musl and wasm-musl
INLINE double env_sin(double d) {
    return sin(d);
}

INLINE double env_cos(double d) {
    return cos(d);
}


// gdbm code
//int gdbm_file = 0;
//GDBM_FILE gdbm_files[100];
//
//i32 env_gdbm_open(i32 name_off, i32 block_size, i32 read_write, i32 mode, i32 fatal_func) {
//    char* name = get_memory_ptr_void(name, 0);
//    GDBM_FILE f = gdbm_open(name, block_size, read_write, mode, NULL);
//    gdbm_files[gdbm_file] = f;
//    gdbm_file++;
//    return (i32) gdbm_file;
//}
//
//i32 env_gdbm_close(i32 gf) {
//
//}
//
//
//void env_gdbm_fetch(i32 a, i32 b, i32 c) {
//}
//
