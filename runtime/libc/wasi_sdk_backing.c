#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <printf.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/uio.h>

#include "../runtime.h"
#include "./wasi_sdk_backing.h"

/* POSIX compatibility shims */
#ifndef O_RSYNC
#define O_RSYNC O_SYNC
#endif

#ifdef __APPLE__
#undef fdatasync
#define fdatasync fsync
#endif

/* Code that actually runs the wasm code */
IMPORT void wasmf__start(void);

/* Globals */
u32 runtime_argc = 0;
char *runtime_argv_buffer = NULL;
u32 runtime_argv_buffer_len = 0;
u32 *runtime_argv_buffer_offsets = NULL;

/* Atexit callbacks */
void runtime_argv_buffer_free() {
    free(runtime_argv_buffer);
}

void runtime_argv_buffer_offsets_free() {
    free(runtime_argv_buffer_offsets);
}

void runtime_cleanup() {
    printf("mem use = %d\n", (int) memory_size);
}

/**
 * @brief Copies arguments provided to the runtime process into global data structures accessible
 * to the WASI argument syscalls
 * 
 * @param argc 
 * @param argv 
 */
void runtime_args_init(int argc, char* argv[]) {
    /* Set argc and argv to globals, these are later used by the WASI syscalls */
    runtime_argc = argc;
    runtime_argv_buffer_offsets = calloc(argc, sizeof(u32));
    if (runtime_argv_buffer_offsets == NULL) {
        fprintf(stderr, "Error allocating runtime_argv_buffer_offsets: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    atexit(runtime_argv_buffer_offsets_free);

    int i;

    /* Calculate vector of argument offsets and argument buffer length */
    for (i = 0; i < argc; i++) {
        runtime_argv_buffer_offsets[i] = runtime_argv_buffer_len;
        runtime_argv_buffer_len += (u32)(strlen(argv[i]) + 1);
    }

    /* Allocate argument buffer */
    runtime_argv_buffer = malloc(runtime_argv_buffer_len);
    if (runtime_argv_buffer == NULL) {
        fprintf(stderr, "Error allocating runtime_argv_buffer: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    atexit(runtime_argv_buffer_free);
        
    /* Copy the arguments consecutively into the buffer based on the offsets */
    for (i = 0; i < argc - 1; i++) {
        strncpy(&runtime_argv_buffer[runtime_argv_buffer_offsets[i]], argv[i], runtime_argv_buffer_offsets[i + 1] - runtime_argv_buffer_offsets[i]);
    }
    if (argc > 0) {
        strncpy(&runtime_argv_buffer[runtime_argv_buffer_offsets[i]], argv[i], runtime_argv_buffer_len - runtime_argv_buffer_offsets[i]);
    }
}

int main(int argc, char* argv[]) {
    runtime_init();
    runtime_args_init(argc, argv);

    atexit(runtime_cleanup);

    atexit(switch_into_runtime);
    switch_out_of_runtime();
    wasmf__start();
    /* WASI wrappes non zero status codes in exits, but silently returns on status code 0 */
    return 0;
}

// WASI API implementations

/**
 * @brief Converts POSIX status codes to WASI
 * 
 * @param errno_ 
 * @return wasi_errno_t 
 */
static wasi_errno_t wasi_fromerrno(int errno_) {
    switch (errno_) {
        case 0:                 return WASI_ESUCCESS;
        case E2BIG:             return WASI_E2BIG;
        case EACCES:            return WASI_EACCES;
        case EADDRINUSE:        return WASI_EADDRINUSE;
        case EADDRNOTAVAIL:     return WASI_EADDRNOTAVAIL;
        case EAFNOSUPPORT:      return WASI_EAFNOSUPPORT;
        case EAGAIN:            return WASI_EAGAIN;
        case EALREADY:          return WASI_EALREADY;
        case EBADF:             return WASI_EBADF;
        case EBADMSG:           return WASI_EBADMSG;
        case EBUSY:             return WASI_EBUSY;
        case ECANCELED:         return WASI_ECANCELED;
        case ECHILD:            return WASI_ECHILD;
        case ECONNABORTED:      return WASI_ECONNABORTED;
        case ECONNREFUSED:      return WASI_ECONNREFUSED;
        case ECONNRESET:        return WASI_ECONNRESET;
        case EDEADLK:           return WASI_EDEADLK;
        case EDESTADDRREQ:      return WASI_EDESTADDRREQ;
        case EDOM:              return WASI_EDOM;
        case EDQUOT:            return WASI_EDQUOT;
        case EEXIST:            return WASI_EEXIST;
        case EFAULT:            return WASI_EFAULT;
        case EFBIG:             return WASI_EFBIG;
        case EHOSTUNREACH:      return WASI_EHOSTUNREACH;
        case EIDRM:             return WASI_EIDRM;
        case EILSEQ:            return WASI_EILSEQ;
        case EINPROGRESS:       return WASI_EINPROGRESS;
        case EINTR:             return WASI_EINTR;
        case EINVAL:            return WASI_EINVAL;
        case EIO:               return WASI_EIO;
        case EISCONN:           return WASI_EISCONN;
        case EISDIR:            return WASI_EISDIR;
        case ELOOP:             return WASI_ELOOP;
        case EMFILE:            return WASI_EMFILE;
        case EMLINK:            return WASI_EMLINK;
        case EMSGSIZE:          return WASI_EMSGSIZE;
        case EMULTIHOP:         return WASI_EMULTIHOP;
        case ENAMETOOLONG:      return WASI_ENAMETOOLONG;
        case ENETDOWN:          return WASI_ENETDOWN;
        case ENETRESET:         return WASI_ENETRESET;
        case ENETUNREACH:       return WASI_ENETUNREACH;
        case ENFILE:            return WASI_ENFILE;
        case ENOBUFS:           return WASI_ENOBUFS;
        case ENODEV:            return WASI_ENODEV;
        case ENOENT:            return WASI_ENOENT;
        case ENOEXEC:           return WASI_ENOEXEC;
        case ENOLCK:            return WASI_ENOLCK;
        case ENOLINK:           return WASI_ENOLINK;
        case ENOMEM:            return WASI_ENOMEM;
        case ENOMSG:            return WASI_ENOMSG;
        case ENOPROTOOPT:       return WASI_ENOPROTOOPT;
        case ENOSPC:            return WASI_ENOSPC;
        case ENOSYS:            return WASI_ENOSYS;
        case ENOTCONN:          return WASI_ENOTCONN;
        case ENOTDIR:           return WASI_ENOTDIR;
        case ENOTEMPTY:         return WASI_ENOTEMPTY;
        case ENOTRECOVERABLE:   return WASI_ENOTRECOVERABLE;
        case ENOTSOCK:          return WASI_ENOTSOCK;
        case ENOTSUP:           return WASI_ENOTSUP;
        case ENOTTY:            return WASI_ENOTTY;
        case ENXIO:             return WASI_ENXIO;
        case EOVERFLOW:         return WASI_EOVERFLOW;
        case EOWNERDEAD:        return WASI_EOWNERDEAD;
        case EPERM:             return WASI_EPERM;
        case EPIPE:             return WASI_EPIPE;
        case EPROTO:            return WASI_EPROTO;
        case EPROTONOSUPPORT:   return WASI_EPROTONOSUPPORT;
        case EPROTOTYPE:        return WASI_EPROTOTYPE;
        case ERANGE:            return WASI_ERANGE;
        case EROFS:             return WASI_EROFS;
        case ESPIPE:            return WASI_ESPIPE;
        case ESRCH:             return WASI_ESRCH;
        case ESTALE:            return WASI_ESTALE;
        case ETIMEDOUT:         return WASI_ETIMEDOUT;
        case ETXTBSY:           return WASI_ETXTBSY;
        case EXDEV:             return WASI_EXDEV;
        default:
            fprintf(stderr, "wasi_fromerrno unexpectedly received: %s\n", strerror(errno_));
            fflush(stderr);
    }

    awsm_assert(0);
    return 0;
}

__attribute__((noreturn))
void wasi_unsupported_syscall(const char *syscall) {
    fprintf(stderr, "Syscall %s is not supported\n", syscall);
    exit(EXIT_FAILURE);
}

/**
 * @brief Returns arguments buffer into linear memory write an 
 * indirect to the base offset of the buffer 
 * 
 * @param argv_ptr
 * @param argv_buf_ptr
 * @return WASI_ESUCCESS
 */
wasi_errno_t wasi_snapshot_preview1_args_get(
    wasi_size_t argv_retptr, 
    wasi_size_t argv_buf_retptr
){
    wasi_size_t *argv = (wasi_size_t *)get_memory_ptr_for_runtime(argv_retptr, sizeof(wasi_size_t) * runtime_argc);
    char *argv_buf = get_memory_ptr_for_runtime(argv_buf_retptr, runtime_argv_buffer_len);
    // TODO: What is the correct behavior if a bounds check fails? WASI error code?

    /* Copy the argument buffer into the linear memory buffer */
    memcpy(argv_buf, runtime_argv_buffer, runtime_argv_buffer_len);

    // Write the vector of argument base offset
    for (u32 i = 0; i < runtime_argc; i++){
        *(argv++) = argv_buf_retptr + runtime_argv_buffer_offsets[i];
    }

    return WASI_ESUCCESS;
}

/**
 * @brief Used by a WASI module to determine the argument count and size of the requried
 * argument buffer
 * 
 * @param argc linear memory offset where we should write argc
 * @param argv_buf_len linear memory offset where we should write the length of the args buffer
 * @return WASI_ESUCCESS
 */
wasi_errno_t wasi_snapshot_preview1_args_sizes_get(
    wasi_size_t argc_retptr, 
    wasi_size_t argv_buf_len_retptr
){
    wasi_size_t *argc = (wasi_size_t *)get_memory_ptr_for_runtime(argc_retptr, sizeof(wasi_size_t));
    wasi_size_t *argv_buf_len = (wasi_size_t *)get_memory_ptr_for_runtime(argv_buf_len_retptr, sizeof(wasi_size_t));

    *argc = runtime_argc;
    *argv_buf_len = runtime_argv_buffer_len;

    return WASI_ESUCCESS;
}

/**
 * @brief Return the resolution of a clock.
 * Implementations are required to provide a non-zero value for supported clocks. For unsupported clocks,
 * return `errno::inval`.
 * Note: This is similar to `clock_getres` in POSIX.
 * 
 * @param id The clock for which to return the resolution.
 * @param res_retptr - The resolution of the clock
 */
wasi_errno_t wasi_snapshot_preview1_clock_res_get(
    wasi_clockid_t id,
    wasi_size_t res_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Return the time value of a clock.
 * Note: This is similar to `clock_gettime` in POSIX.
 * 
 * @param clock_id The clock for which to return the time.
 * @param precision The maximum lag (exclusive) that the returned time value may have, compared to its actual value.
 * @param time_retptr  The time value of the clock.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_clock_time_get(
    wasi_clockid_t clock_id, 
    wasi_timestamp_t precision, 
    wasi_size_t time_retptr
) {
    struct timespec tp;
    int rc = clock_gettime(clock_id, &tp);
    if (rc == -1) {
        return wasi_fromerrno(errno);
    }

    set_i64(time_retptr, (uint64_t)tp.tv_sec*1000000000ULL + (uint64_t)tp.tv_nsec);

    return WASI_ESUCCESS;
}

/**
 * Read environment variable data.
 * The sizes of the buffers should match that returned by `environ_sizes_get`.
 * 
 * @param environ_retptr
 * @param environ_buf_retptr
 */
wasi_errno_t wasi_snapshot_preview1_environ_get(
    wasi_size_t environ_retptr, 
    wasi_size_t environ_buf_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Returns the number of environment variable arguments and the size of the environment variable data.
 * 
 * @param environc_retptr - the offset where the resulting number of environment variable arguments should be written
 * @param environv_buf_len_retptr - the offset where the resulting size of the environment variable data should be written
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_environ_sizes_get(
    wasi_size_t environc_retptr, 
    wasi_size_t environv_buf_len_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Provide file advisory information on a file descriptor.
 * Note: This is similar to `posix_fadvise` in POSIX.
 * 
 * @param fd
 * @param offset The offset within the file to which the advisory applies.
 * @param len The length of the region to which the advisory applies.
 * @param advice
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_advise(
    wasi_fd_t fd,
    wasi_filesize_t offset,
    wasi_filesize_t len,
    wasi_advice_t advice
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Force the allocation of space in a file.
 * Note: This is similar to `posix_fallocate` in POSIX.
 * 
 * @param fd 
 * @param offset The offset at which to start the allocation.
 * @param len The length of the area that is allocated.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_allocate(
    wasi_fd_t fd,
    wasi_filesize_t offset,
    wasi_filesize_t len
){
    wasi_unsupported_syscall(__func__);
};

/**
 * Close a file descriptor.
 * Note: This is similar to `close` in POSIX.
 * 
 * @param fd
 * @return WASI_ESUCCESS, WASI_EBADF, WASI_EINTR, WASI_EIO, WASI_ENOSPC, or WASI_EDQUOT
 */
wasi_errno_t wasi_snapshot_preview1_fd_close(wasi_fd_t fd) {
    int res = (wasi_fd_t) close(fd);

    if (res == -1) {
        return wasi_fromerrno(errno);
    }
    return WASI_ESUCCESS;
}

/**
 * Synchronize the data of a file to disk.
 * Note: This is similar to `fdatasync` in POSIX.
 * 
 * @param fd
 * @return WASI_ESUCCESS, WASI_EBADF, WASI_EIO, WASI_ENOSPC, WASI_EROFS, WASI_EINVAL, WASI_ENOSPC, WASI_EDQUOT
 */
wasi_errno_t wasi_snapshot_preview1_fd_datasync(wasi_fd_t fd) {
    int res = fdatasync(fd);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    return WASI_ESUCCESS;
}

/**
 * Get the attributes of a file descriptor.
 * Note: This returns similar flags to `fsync(fd, F_GETFL)` in POSIX, as well as additional fields.
 * 
 * @param fd 
 * @param fdstat_retptr the offset where the resulting wasi_fdstat structure should be written
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EAGAIN, WASI_EBADF, WASI_EFAULT, WASI_EINVAL, WASI_ELOOP, 
 * WASI_ENAMETOOLONG, WASI_ENOTDIR, WASI_ENOENT, WASI_ENOMEM, or WASI_EOVERFLOW
 */
wasi_errno_t wasi_snapshot_preview1_fd_fdstat_get(
    wasi_fd_t fd, 
    wasi_size_t fdstat_retptr
) {
    struct wasi_fdstat* fdstat = get_memory_ptr_void(fdstat_retptr, sizeof(struct wasi_fdstat));

    struct stat stat;
    i32 res = fstat(fd, &stat);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }
    int mode = stat.st_mode;

    i32 fl = fcntl(fd, F_GETFL);
    if (fl < 0) {
        return wasi_fromerrno(errno);
    }

    fdstat->fs_filetype = (
            (S_ISBLK(mode)  ? WASI_FILETYPE_BLOCK_DEVICE     : 0) |
            (S_ISCHR(mode)  ? WASI_FILETYPE_CHARACTER_DEVICE : 0) |
            (S_ISDIR(mode)  ? WASI_FILETYPE_DIRECTORY        : 0) |
            (S_ISREG(mode)  ? WASI_FILETYPE_REGULAR_FILE     : 0) |
            (S_ISSOCK(mode) ? WASI_FILETYPE_SOCKET_STREAM    : 0) |
            (S_ISLNK(mode)  ? WASI_FILETYPE_SYMBOLIC_LINK    : 0));
    fdstat->fs_flags = (
            ((fl & O_APPEND)   ? WASI_FDFLAG_APPEND     : 0) |
            ((fl & O_DSYNC)    ? WASI_FDFLAG_DSYNC      : 0) |
            ((fl & O_NONBLOCK) ? WASI_FDFLAG_NONBLOCK   : 0) |
            ((fl & O_RSYNC)    ? WASI_FDFLAG_RSYNC      : 0) |
            ((fl & O_SYNC)     ? WASI_FDFLAG_SYNC       : 0));
    fdstat->fs_rights_base = 0; // all rights
    fdstat->fs_rights_inheriting = 0; // all rights

    return WASI_ESUCCESS;
}

/**
 * Adjust the flags associated with a file descriptor.
 * Note: This is similar to `fcntl(fd, F_SETFL, flags)` in POSIX.
 * @param fd
 * @param fdflags The desired values of the file descriptor flags.
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EAGAIN, WASI_EBADF, WASI_EFAULT, WASI_EINVAL, 
 * WASI_ENOENT, or WASI_EPERM
 */
wasi_errno_t wasi_snapshot_preview1_fd_fdstat_set_flags(
    wasi_fd_t fd, 
    wasi_fdflags_t fdflags
) {
    int flags = (
        ((flags & WASI_FDFLAG_APPEND  ) ? O_APPEND   : 0) |
        ((flags & WASI_FDFLAG_DSYNC   ) ? O_DSYNC    : 0) |
        ((flags & WASI_FDFLAG_NONBLOCK) ? O_NONBLOCK : 0) |
        ((flags & WASI_FDFLAG_RSYNC   ) ? O_RSYNC    : 0) |
        ((flags & WASI_FDFLAG_SYNC    ) ? O_SYNC     : 0));
    int err = fcntl(fd, F_SETFL, fdflags);
    if (err < 0) {
        return wasi_fromerrno(errno);
    }
    return WASI_ESUCCESS;
}

/**
 * Adjust the rights associated with a file descriptor.
 * This can only be used to remove rights, and returns `errno::notcapable` if called in a way that would attempt to add rights
 * 
 * @param fd
 * @param fs_rights_base The desired rights of the file descriptor.
 * @param fs_rights_inheriting
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_fdstat_set_rights(
    wasi_fd_t fd,
    wasi_rights_t fs_rights_base,
    wasi_rights_t fs_rights_inheriting
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Return the attributes of an open file.
 * 
 * @param fd
 * @param filestat_retptr The buffer where we should store the file's attributes
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_filestat_get(
    wasi_fd_t fd,
    wasi_size_t filestat_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Adjust the size of an open file. If this increases the file's size, the extra bytes are filled with zeros.
 * Note: This is similar to `ftruncate` in POSIX.
 * 
 * @param fd 
 * @param size The desired file size.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_filestat_set_size(
    wasi_fd_t fd,
    wasi_filesize_t size
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Adjust the timestamps of an open file or directory.
 * Note: This is similar to `futimens` in POSIX.
 * 
 * @param fd
 * @param atim The desired values of the data access timestamp.
 * @param mtim The desired values of the data modification timestamp.
 * @param fst_flags A bitmask indicating which timestamps to adjust.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_filestat_set_times(
    wasi_fd_t fd,
    wasi_timestamp_t atim,
    wasi_timestamp_t mtim,
    wasi_fstflags_t fst_flags
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Read from a file descriptor, without using and updating the file descriptor's offset.
 * Note: This is similar to `preadv` in POSIX.
 * 
 * @param fd 
 * @param iovs_ptr List of scatter/gather vectors in which to store data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param offset The offset within the file at which to read.
 * @param nbytes_retptr The number of bytes read.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_pread(
    wasi_fd_t fd,
    wasi_size_t iovs_ptr,
    size_t iovs_len,
    wasi_filesize_t offset,
    wasi_size_t nbytes_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Return a description of the given preopened file descriptor.
 * 
 * @param fd
 * @param prestat_retptr The buffer where the description is stored.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_prestat_get(
    wasi_fd_t fd,
    wasi_size_t prestat_retptr 
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Return a description of the given preopened file descriptor.
 * 
 * @param fd
 * @param path_retptr A buffer into which to write the preopened directory name.
 * @param path_len The length of the buffer at path_retptr
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_prestat_dir_name(
    wasi_fd_t fd,
    wasi_size_t path_retptr,
    wasi_size_t path_len
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Write to a file descriptor, without using and updating the file descriptor's offset.
 * Note: This is similar to `pwritev` in POSIX.
 * 
 * @param fd
 * @param iovs_ptr List of scatter/gather vectors from which to retrieve data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param offset The offset within the file at which to write.
 * @param nwritten_retptr The number of bytes written.
 * @return status code
 * 
 */
wasi_errno_t wasi_snapshot_preview1_fd_pwrite(
    wasi_fd_t fd,
    wasi_size_t iovs_ptr,
    size_t iovs_len,
    wasi_filesize_t offset,
    wasi_size_t nwritten_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Read from a file descriptor.
 * Note: This is similar to `readv` in POSIX.
 * 
 * @param fd
 * @param iovs_ptr
 * @param iovs_len
 * @param nwritten_retptr The number of bytes read.
 * @return WASI_ESUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT, WASI_EINTR, WASI_EIO, WASI_EISDIR, or others
 */
wasi_errno_t wasi_snapshot_preview1_fd_read(
    wasi_fd_t fd, 
    wasi_size_t iovs_ptr, 
    size_t iovs_len, 
    wasi_size_t nwritten_retptr
) {
    i32 sum = 0;
    const struct wasi_iovec *const iovs = get_memory_ptr_void(iovs_ptr, iovs_len * sizeof(struct wasi_iovec));

    for (int i = 0; i < iovs_len; i++) {
        void* ptr = get_memory_ptr_void(iovs[i].base_offset, iovs[i].len);
        ssize_t res = read(fd, ptr, iovs[i].len);
        if (res == -1) {
            return wasi_fromerrno(errno);
        }

        sum += res;
    }

    set_i32(nwritten_retptr, sum);
    return WASI_ESUCCESS;
}

/**
 * Read directory entries from a directory.
 * When successful, the contents of the output buffer consist of a sequence of
 * directory entries. Each directory entry consists of a `dirent` object,
 * followed by `dirent::d_namlen` bytes holding the name of the directory
 * entry.
 * This function fills the output buffer as much as possible, potentially
 * truncating the last directory entry. This allows the caller to grow its
 * read buffer size in case it's too small to fit a single large directory
 * entry, or skip the oversized directory entry.
 * 
 * @param fd
 * @param buf_ptr The buffer where directory entries are stored
 * @param buf_len
 * @param cookie The location within the directory to start reading
 * @param nwritten_retptr The number of bytes stored in the read buffer. If less than the size of the read buffer, the end of the directory has been reached.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_readdir(
    wasi_fd_t fd,
    wasi_size_t buf_ptr,
    wasi_size_t buf_len,
    wasi_dircookie_t cookie,
    wasi_size_t nwritten_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Atomically replace a file descriptor by renumbering another file descriptor.
 * Due to the strong focus on thread safety, this environment does not provide
 * a mechanism to duplicate or renumber a file descriptor to an arbitrary
 * number, like `dup2()`. This would be prone to race conditions, as an actual
 * file descriptor with the same number could be allocated by a different
 * thread at the same time.
 * This function provides a way to atomically renumber file descriptors, which
 * would disappear if `dup2()` were to be removed entirely.
 * 
 * @param fd
 * @param to the file descriptor to overwrite
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_renumber(
    wasi_fd_t fd,
    wasi_fd_t to
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Move the offset of a file descriptor.
 * Note: This is similar to `lseek` in POSIX.
 * 
 * @param fd
 * @param file_offset The number of bytes to move.
 * @param whence The base from which the offset is relative.
 * @param newoffset_retptr The new offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_seek(
    wasi_fd_t fd, 
    wasi_filedelta_t file_offset, 
    wasi_whence_t whence, 
    wasi_filesize_t newoffset_retptr
) {
    off_t res = lseek(fd, (off_t)file_offset, whence);

    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    set_i64(newoffset_retptr, res);
    return WASI_ESUCCESS;
}

/**
 * Synchronize the data and metadata of a file to disk.
 * Note: This is similar to `fsync` in POSIX.
 * 
 * @param fd
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_sync(
    wasi_fd_t fd
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Return the current offset of a file descriptor.
 * Note: This is similar to `lseek(fd, 0, SEEK_CUR)` in POSIX.
 * 
 * @param fd
 * @param fileoffset_retptr The current offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_fd_tell(
    wasi_fd_t fd,
    wasi_size_t fileoffset_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Write to a file descriptor.
 * Note: This is similar to `writev` in POSIX.
 * 
 * @param fd
 * @param iovs_ptr List of scatter/gather vectors from which to retrieve data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param nwritten_retptr
 * @return WASI_ESUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT, 
 * WASI_EFBIG, WASI_EINTR, WASI_EIO, WASI_ENOSPC, WASI_EPERM, WASI_EPIPE, or others
 */
wasi_errno_t wasi_snapshot_preview1_fd_write(
    wasi_fd_t fd, 
    wasi_size_t iovs_ptr, 
    size_t iovs_len, 
    wasi_size_t nwritten_retptr
) {
    wasi_size_t sum = 0;
    const struct wasi_iovec *const iovs = get_memory_ptr_void(iovs_ptr, iovs_len * sizeof(struct wasi_iovec));

    for (int i = 0; i < iovs_len; i++) {
        void* ptr = get_memory_ptr_void(iovs[i].base_offset, iovs[i].len);
        ssize_t res = write(fd, ptr, iovs[i].len);
        if (res == -1) {
            return wasi_fromerrno(errno);
        }
        sum += res;
    }
    set_i32(nwritten_retptr, sum);

    return WASI_ESUCCESS;
}

/**
 * Create a directory.
 * Note: This is similar to `mkdirat` in POSIX.
 * 
 * @param fd
 * @param path_ptr
 * @param path_len
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EBADF, WASI_EDQUOT, WASI_EEXIST, 
 * WASI_EFAULT, WASI_EINVAL, WASI_ELOOP, WASI_EMLINK, WASI_ENAMETOOLONG, 
 * WASI_ENOENT, WASI_ENOMEM, WASI_ENOSPC, WASI_ENOTDIR, WASI_EPERM, or WASI_EROFS
 */
wasi_errno_t wasi_snapshot_preview1_path_create_directory(
    wasi_fd_t fd, 
    u32 path_ptr, 
    u32 path_len
) {
    const char *const path = get_memory_string(path_ptr);

    int res = mkdirat(fd, path, 0777);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    return WASI_ESUCCESS;
}

/**
 * Return the attributes of a file or directory.
 * Note: This is similar to `stat` in POSIX.
 * 
 * @param fd
 * @param flags Flags determining the method of how the path is resolved.
 * @param path_ptr The path of the file or directory to inspect.
 * @param filestat_retptr The buffer where the file's attributes are stored.
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EBAD, WASI_EFAUL, WASI_EINVAL, WASI_ELOOP, 
 * WASI_ENAMETOOLON, WASI_ENOENT, WASI_ENOENT, WASI_ENOMEM, WASI_ENOTDI, or WASI_EOVERFLOW
 */
wasi_errno_t wasi_snapshot_preview1_path_filestat_get(
    wasi_fd_t fd, 
    wasi_lookupflags_t flags, 
    wasi_size_t path_ptr, 
    wasi_size_t path_len, 
    wasi_size_t filestat_retptr
) {
    // get path/filestat
    const char *const path = get_memory_string(path_ptr);
    wasi_filestat_t *const filestat = get_memory_ptr_void(filestat_retptr, sizeof(wasi_filestat_t));

    struct stat stat;
    int res = fstatat(fd, path, &stat, 0);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    filestat->dev = stat.st_dev;
    filestat->ino = stat.st_ino;
    filestat->filetype =  stat.st_mode;
    filestat->nlink = stat.st_nlink;
    filestat->size = stat.st_size;
    filestat->atim = stat.st_atime;
    filestat->mtim = stat.st_mtime;
    filestat->ctim = stat.st_ctime;

    return WASI_ESUCCESS;
}

/**
 * Adjust the timestamps of a file or directory.
 * Note: This is similar to `utimensat` in POSIX.
 * 
 * @param fd
 * @param flags Flags determining the method of how the path is resolved.
 * @param path The path of the file or directory to operate on.
 * @param atim The desired values of the data access timestamp.
 * @param mtim The desired values of the data modification timestamp.
 * @param fst_flags A bitmask indicating which timestamps to adjust.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_filestat_set_times(
    wasi_fd_t fd,
    wasi_lookupflags_t flags,
    wasi_size_t path_ptr,
    wasi_size_t path_len, 
    wasi_timestamp_t atim,
    wasi_timestamp_t mtim,
    wasi_fstflags_t fst_flags
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Create a hard link.
 * Note: This is similar to `linkat` in POSIX.
 * 
 * @param old_fd
 * @param old_flags Flags determining the method of how the path is resolved.
 * @param old_path The source path from which to link.
 * @param new_fd The working directory at which the resolution of the new path starts.
 * @param new_path  The destination path at which to create the hard link.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_link(
    wasi_fd_t old_fd,
    wasi_lookupflags_t old_flags,
    wasi_size_t old_path_ptr,
    wasi_size_t old_path_len,
    wasi_fd_t new_fd,
    wasi_size_t new_path_ptr,
    wasi_size_t new_path_len
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Open a file or directory.
 * The returned file descriptor is not guaranteed to be the lowest-numbered
 * file descriptor not currently open; it is randomized to prevent
 * applications from depending on making assumptions about indexes, since this
 * is error-prone in multi-threaded contexts. The returned file descriptor is
 * guaranteed to be less than 2**31.
 * Note: This is similar to `openat` in POSIX.
 * 
 * @param dirfd
 * @param lookupflags
 * @param path_off
 * @param path_len
 * @param oflags
 * @param fs_rights_base
 * @param fs_rights_inheriting
 * @param fdflags
 * @param fd_off The file descriptor of the file that has been opened.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_open(
        wasi_fd_t dirfd,
        wasi_lookupflags_t lookupflags,
        wasi_size_t path_off,
        wasi_size_t path_len,
        wasi_oflags_t oflags,
        wasi_rights_t fs_rights_base,
        wasi_rights_t fs_rights_inheriting,
        wasi_fdflags_t fdflags,
        wasi_fd_t fd_off) {
    // get path
    char* path = get_memory_string(path_off);

    // translate o_flags and fs_flags into flags and mode
    int flags = (
            ((oflags & WASI_O_CREAT    ) ? O_CREAT     : 0) |
            ((oflags & WASI_O_DIRECTORY) ? O_DIRECTORY : 0) |
            ((oflags & WASI_O_EXCL     ) ? O_EXCL      : 0) |
            ((oflags & WASI_O_TRUNC    ) ? O_TRUNC     : 0) |
            ((fdflags & WASI_FDFLAG_APPEND  ) ? O_APPEND   : 0) |
            ((fdflags & WASI_FDFLAG_DSYNC   ) ? O_DSYNC    : 0) |
            ((fdflags & WASI_FDFLAG_NONBLOCK) ? O_NONBLOCK : 0) |
            ((fdflags & WASI_FDFLAG_RSYNC   ) ? O_RSYNC    : 0) |
            ((fdflags & WASI_FDFLAG_SYNC    ) ? O_SYNC     : 0));
    if ((fs_rights_base & WASI_RIGHT_FD_WRITE) &&
        (fs_rights_base & WASI_RIGHT_FD_READ)) {
        flags |= O_RDWR;
    } else if (fs_rights_base & WASI_RIGHT_FD_WRITE) {
        flags |= O_WRONLY;
    } else if (fs_rights_base & WASI_RIGHT_FD_READ) {
        flags |= O_RDONLY;
    }

    int mode = 0644;
    int fd = openat(dirfd, path, flags, mode);
    if (fd < 0) {
        return wasi_fromerrno(errno);
    }

    set_i32(fd_off, fd);
    return WASI_ESUCCESS;
}

/**
 * Read the contents of a symbolic link.
 * Note: This is similar to `readlinkat` in POSIX.
 * 
 * @param fd
 * @param path_ptr The path of the symbolic link from which to read.
 * @param path_len
 * @param buf_retptr The buffer to which to write the contents of the symbolic link.
 * @param buf_len
 * @param nread_retptr The number of bytes placed in the buffer.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_readlink(
    wasi_fd_t fd,
    wasi_size_t path_ptr,
    wasi_size_t path_len,
    wasi_size_t buf_retptr,
    wasi_size_t buf_len,
    wasi_size_t nread_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Remove a directory.
 * Return `errno::notempty` if the directory is not empty.
 * Note: This is similar to `unlinkat(fd, path, AT_REMOVEDIR)` in POSIX.
 * 
 * @param fd
 * @param path The path to a directory to remove.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_remove_directory(
    wasi_fd_t fd,
    wasi_size_t path_ptr,
    wasi_size_t path_len
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Rename a file or directory.
 * Note: This is similar to `renameat` in POSIX.
 * 
 * @param fd
 * @param old_path The source path of the file or directory to rename.
 * @param new_fd The working directory at which the resolution of the new path starts.
 * @param new_path The destination path to which to rename the file or directory.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_rename(
    wasi_fd_t fd,
    wasi_size_t old_path_ptr,
    wasi_size_t old_path_len,
    wasi_fd_t new_fd,
    wasi_size_t new_path_ptr,
    wasi_size_t new_path_len
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Create a symbolic link.
 * Note: This is similar to `symlinkat` in POSIX.
 * 
 * @param old_path_ptr The contents of the symbolic link.
 * @param old_path_len
 * @param fd
 * @param new_path The destination path at which to create the symbolic link.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_symlink(
    wasi_size_t old_path_ptr,
    wasi_size_t old_path_len,
    wasi_fd_t fd,
    wasi_size_t new_path_ptr,
    wasi_size_t new_path_len
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Unlink a file.
 * Return `errno::isdir` if the path refers to a directory.
 * Note: This is similar to `unlinkat(fd, path, 0)` in POSIX.
 * 
 * @param fd
 * @param path_ptr
 * @param path_len
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_path_unlink_file(
    wasi_fd_t fd, 
    wasi_size_t path_ptr, 
    wasi_size_t path_len
) {
    // get path
    char* path = get_memory_string(path_ptr);

    int res = unlinkat(fd, path, 0);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    return WASI_ESUCCESS;
}

/**
 * Concurrently poll for the occurrence of a set of events.
 * @param in The events to which to subscribe.
 * @param out The events that have occurred.
 * @param nsubscriptions Both the number of subscriptions and events.
 * @param retptr The number of events stored.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_poll_oneoff(
    const wasi_subscription_t * in,
    wasi_event_t * out,
    wasi_size_t nsubscriptions,
    wasi_size_t *retptr0
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Terminate the process normally. An exit code of 0 indicates successful
 * termination of the program. The meanings of other values is dependent on
 * the environment.
 * 
 * @param exitcode
 */
__attribute__((noreturn))
void wasi_snapshot_preview1_proc_exit(wasi_exitcode_t exitcode) {
    exit(exitcode);
}

/**
 * Send a signal to the process of the calling thread.
 * Note: This is similar to `raise` in POSIX.
 * 
 * @param sig The signal condition to trigger.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_proc_raise(
    wasi_signal_t sig
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Write high-quality random data into a buffer.
 * This function blocks when the implementation is unable to immediately
 * provide sufficient high-quality random data.
 * This function may execute slowly, so when large mounts of random data are
 * required, it's advisable to use this function to seed a pseudo-random
 * number generator, rather than to provide the random data directly.
 * 
 * @param buf_ptr The buffer to fill with random data.
 * @param buf_len The length of the buffer
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_random_get(
    wasi_size_t buf_ptr,
    wasi_size_t buf_len
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Temporarily yield execution of the calling thread.
 * Note: This is similar to `sched_yield` in POSIX.
 * 
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_sched_yield(void) {
    // Does nothing
    return WASI_ESUCCESS;
}

/**
 * Receive a message from a socket.
 * Note: This is similar to `recv` in POSIX, though it also supports reading
 * the data into multiple buffers in the manner of `readv`.
 * 
 * @param fd
 * @param ri_data_ptr List of scatter/gather vectors to which to store data.
 * @param ri_data_len The length of the array pointed to by `ri_data`.
 * @param ri_flags Message flags.
 * @param ri_data_nbytes_retptr Number of bytes stored in ri_data flags.
 * @param message_nbytes_retptr Number of bytes stored in message flags.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_sock_recv(
    wasi_fd_t fd,
    wasi_size_t ri_data_ptr,
    wasi_size_t ri_data_len,
    wasi_riflags_t ri_flags,
    wasi_size_t ri_data_nbytes_retptr,
    wasi_size_t message_nbytes_retptr
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Send a message on a socket.
 * Note: This is similar to `send` in POSIX, though it also supports writing
 * the data from multiple buffers in the manner of `writev`.
 * 
 * @param fd
 * @param si_data_ptr List of scatter/gather vectors to which to retrieve data
 * @param si_data_len The length of the array pointed to by `si_data`.
 * @param si_flags Message flags.
 * @param nbytes_retptr Number of bytes transmitted.
 * @return status code 
 */
wasi_errno_t wasi_snapshot_preview1_sock_send(
    wasi_fd_t fd,
    wasi_size_t si_data_ptr,
    size_t si_data_len,
    wasi_siflags_t si_flags,
    wasi_size_t *retptr0
) {
    wasi_unsupported_syscall(__func__);
}

/**
 * Shut down socket send and receive channels.
 * Note: This is similar to `shutdown` in POSIX.
 * 
 * @param fd
 * @param how Which channels on the socket to shut down.
 * @return status code
 */
wasi_errno_t wasi_snapshot_preview1_sock_shutdown(
    wasi_fd_t fd,
    wasi_sdflags_t how
) {
    wasi_unsupported_syscall(__func__);
}
