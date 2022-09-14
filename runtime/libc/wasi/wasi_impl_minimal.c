#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <printf.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/uio.h>

#include "wasi_impl.h"


/*
 * This intention of this minimal backing is to demonstrate a simple standalone
 * backing of the WASI syscalls that delegates most logic to the underlying host's
 * POSIX layer. It is currently incomplete and only has implemented the bare minimum
 * needed to run simple echo and fibonacci type programs. As such, it only passes
 * a subset of the WASI unit tests.
 */

/*
 * Essentially based off the uvwasi_t
 * TODO: Add file descriptors in support of preopens
 */
typedef struct wasi_context_s {
    __wasi_size_t argc;
    char**        argv;
    char*         argv_buf;
    __wasi_size_t argv_buf_size;
    __wasi_size_t envc;
    char**        env;
    char*         env_buf;
    __wasi_size_t env_buf_size;
} wasi_context_t;

/* Return abstract handle */
void* wasi_context_init(wasi_options_t* options) {
    /* TODO: Add default types */
    awsm_assert(options != NULL);
    awsm_assert(options->argv != NULL);

    wasi_context_t* wasi_context = malloc(sizeof(wasi_context_t));

    /* Strip path from first arg, calculating offset and length */
    size_t first_arg_offset = 0;
    size_t first_arg_len    = 0;
    for (first_arg_offset = strlen(options->argv[0]); first_arg_offset > 0; first_arg_offset--) {
        if (options->argv[0][first_arg_offset] == '/') {
            first_arg_offset++;
            break;
        }
    }
    first_arg_len = strlen(options->argv[0]) - first_arg_offset;

    /* Calculate argument buffer size */
    __wasi_size_t argv_buf_size = 0;
    __wasi_size_t argv_buffer_offsets[options->argc + 1];
    for (int i = 0; i < options->argc; i++) {
        argv_buffer_offsets[i] = argv_buf_size;
        if (i == 0) {
            argv_buf_size += first_arg_len + 1;
        } else {
            argv_buf_size += strlen(options->argv[i]) + 1;
        }
    }
    argv_buffer_offsets[options->argc] = argv_buf_size;


    /* Allocate and copy argument sizes and offsets*/
    wasi_context->argc = options->argc;
    wasi_context->argv = calloc(options->argc + 1, sizeof(char*));
    if (wasi_context->argv == NULL) {
        fprintf(stderr, "Error allocating argv: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    wasi_context->argv_buf_size = argv_buf_size;
    wasi_context->argv_buf      = calloc(argv_buf_size, sizeof(char));
    if (wasi_context->argv_buf == NULL) {
        fprintf(stderr, "Error allocating argv_buf: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Copy the binary name minux the path as the first arg */
    strncpy(wasi_context->argv_buf, &options->argv[0][first_arg_offset], first_arg_len);

    /* Copy the binary name minus the path as the first arg */
    for (int i = 1; i < options->argc; i++) {
        strncpy(&wasi_context->argv_buf[argv_buffer_offsets[i]], options->argv[i],
                argv_buffer_offsets[i + 1] - argv_buffer_offsets[i]);
    }

    /* Write argv pointers from argv_buffer_offsets */
    for (int i = 0; i < options->argc; i++) {
        wasi_context->argv[i] = &(wasi_context->argv_buf[argv_buffer_offsets[i]]);
    }

    /* Calculate env sizes */
    __wasi_size_t envc         = 0;
    __wasi_size_t env_buf_size = 0;
    for (char** environ_cursor = (char**)options->envp; *environ_cursor != NULL; environ_cursor++) {
        envc++;
        env_buf_size += strlen(*environ_cursor) + 1;
    }
    wasi_context->envc = envc;

    /* Allocate env and env_buf */
    wasi_context->env = calloc(envc, sizeof(char**));
    if (wasi_context->env == NULL) {
        fprintf(stderr, "Error allocating env: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    wasi_context->env_buf_size = env_buf_size;
    wasi_context->env_buf      = calloc(env_buf_size, sizeof(char));
    if (wasi_context->env_buf == NULL) {
        fprintf(stderr, "Error allocating env_buf: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Write env and env_buf */
    __wasi_size_t env_buf_written = 0;
    for (int i = 0; i < envc; i++) {
        wasi_context->env[i] = &(wasi_context->env_buf[env_buf_written]);
        strcpy(wasi_context->env[i], options->envp[i]);
        env_buf_written += (strlen(options->envp[i]) + 1);
    }

    /* See Random */
    srandom(time(NULL));

    /* TODO: Preopens */

    return wasi_context;
}

void wasi_context_destroy(void* wasi_context) {
    free(((wasi_context_t*)wasi_context)->argv);
    free(((wasi_context_t*)wasi_context)->argv_buf);
    free(((wasi_context_t*)wasi_context)->env);
    free(((wasi_context_t*)wasi_context)->env_buf);
    free(wasi_context);
}

__wasi_size_t wasi_context_get_argc(void* wasi_context) {
    return ((wasi_context_t*)wasi_context)->argc;
}


char** wasi_context_get_argv(void* wasi_context) {
    return ((wasi_context_t*)wasi_context)->argv;
}

__wasi_size_t wasi_context_get_argv_buf_size(void* wasi_context) {
    return ((wasi_context_t*)wasi_context)->argv_buf_size;
}

__wasi_size_t wasi_context_get_envc(void* wasi_context) {
    return ((wasi_context_t*)wasi_context)->envc;
}

__wasi_size_t wasi_context_get_env_buf_size(void* wasi_context) {
    return ((wasi_context_t*)wasi_context)->env_buf_size;
}

/* WASI API implementations */

/**
 * @brief Converts POSIX status codes to WASI
 *
 * @param errno_
 * @return wasi_errno_t
 */
static __wasi_errno_t wasi_fromerrno(int errno_) {
    switch (errno_) {
        case 0: return __WASI_ERRNO_SUCCESS;
        case E2BIG: return __WASI_ERRNO_2BIG;
        case EACCES: return __WASI_ERRNO_ACCES;
        case EADDRINUSE: return __WASI_ERRNO_ADDRINUSE;
        case EADDRNOTAVAIL: return __WASI_ERRNO_ADDRNOTAVAIL;
        case EAFNOSUPPORT: return __WASI_ERRNO_AFNOSUPPORT;
        case EAGAIN: return __WASI_ERRNO_AGAIN;
        case EALREADY: return __WASI_ERRNO_ALREADY;
        case EBADF: return __WASI_ERRNO_BADF;
        case EBADMSG: return __WASI_ERRNO_BADMSG;
        case EBUSY: return __WASI_ERRNO_BUSY;
        case ECANCELED: return __WASI_ERRNO_CANCELED;
        case ECHILD: return __WASI_ERRNO_CHILD;
        case ECONNABORTED: return __WASI_ERRNO_CONNABORTED;
        case ECONNREFUSED: return __WASI_ERRNO_CONNREFUSED;
        case ECONNRESET: return __WASI_ERRNO_CONNRESET;
        case EDEADLK: return __WASI_ERRNO_DEADLK;
        case EDESTADDRREQ: return __WASI_ERRNO_DESTADDRREQ;
        case EDOM: return __WASI_ERRNO_DOM;
        case EDQUOT: return __WASI_ERRNO_DQUOT;
        case EEXIST: return __WASI_ERRNO_EXIST;
        case EFAULT: return __WASI_ERRNO_FAULT;
        case EFBIG: return __WASI_ERRNO_FBIG;
        case EHOSTUNREACH: return __WASI_ERRNO_HOSTUNREACH;
        case EIDRM: return __WASI_ERRNO_IDRM;
        case EILSEQ: return __WASI_ERRNO_ILSEQ;
        case EINPROGRESS: return __WASI_ERRNO_INPROGRESS;
        case EINTR: return __WASI_ERRNO_INTR;
        case EINVAL: return __WASI_ERRNO_INVAL;
        case EIO: return __WASI_ERRNO_IO;
        case EISCONN: return __WASI_ERRNO_ISCONN;
        case EISDIR: return __WASI_ERRNO_ISDIR;
        case ELOOP: return __WASI_ERRNO_LOOP;
        case EMFILE: return __WASI_ERRNO_MFILE;
        case EMLINK: return __WASI_ERRNO_MLINK;
        case EMSGSIZE: return __WASI_ERRNO_MSGSIZE;
        case EMULTIHOP: return __WASI_ERRNO_MULTIHOP;
        case ENAMETOOLONG: return __WASI_ERRNO_NAMETOOLONG;
        case ENETDOWN: return __WASI_ERRNO_NETDOWN;
        case ENETRESET: return __WASI_ERRNO_NETRESET;
        case ENETUNREACH: return __WASI_ERRNO_NETUNREACH;
        case ENFILE: return __WASI_ERRNO_NFILE;
        case ENOBUFS: return __WASI_ERRNO_NOBUFS;
        case ENODEV: return __WASI_ERRNO_NODEV;
        case ENOENT: return __WASI_ERRNO_NOENT;
        case ENOEXEC: return __WASI_ERRNO_NOEXEC;
        case ENOLCK: return __WASI_ERRNO_NOLCK;
        case ENOLINK: return __WASI_ERRNO_NOLINK;
        case ENOMEM: return __WASI_ERRNO_NOMEM;
        case ENOMSG: return __WASI_ERRNO_NOMSG;
        case ENOPROTOOPT: return __WASI_ERRNO_NOPROTOOPT;
        case ENOSPC: return __WASI_ERRNO_NOSPC;
        case ENOSYS: return __WASI_ERRNO_NOSYS;
        case ENOTCONN: return __WASI_ERRNO_NOTCONN;
        case ENOTDIR: return __WASI_ERRNO_NOTDIR;
        case ENOTEMPTY: return __WASI_ERRNO_NOTEMPTY;
        case ENOTRECOVERABLE: return __WASI_ERRNO_NOTRECOVERABLE;
        case ENOTSOCK: return __WASI_ERRNO_NOTSOCK;
        case ENOTSUP: return __WASI_ERRNO_NOTSUP;
        case ENOTTY: return __WASI_ERRNO_NOTTY;
        case ENXIO: return __WASI_ERRNO_NXIO;
        case EOVERFLOW: return __WASI_ERRNO_OVERFLOW;
        case EOWNERDEAD: return __WASI_ERRNO_OWNERDEAD;
        case EPERM: return __WASI_ERRNO_PERM;
        case EPIPE: return __WASI_ERRNO_PIPE;
        case EPROTO: return __WASI_ERRNO_PROTO;
        case EPROTONOSUPPORT: return __WASI_ERRNO_PROTONOSUPPORT;
        case EPROTOTYPE: return __WASI_ERRNO_PROTOTYPE;
        case ERANGE: return __WASI_ERRNO_RANGE;
        case EROFS: return __WASI_ERRNO_ROFS;
        case ESPIPE: return __WASI_ERRNO_SPIPE;
        case ESRCH: return __WASI_ERRNO_SRCH;
        case ESTALE: return __WASI_ERRNO_STALE;
        case ETIMEDOUT: return __WASI_ERRNO_TIMEDOUT;
        case ETXTBSY: return __WASI_ERRNO_TXTBSY;
        case EXDEV: return __WASI_ERRNO_XDEV;
        default: fprintf(stderr, "wasi_fromerrno unexpectedly received: %s\n", strerror(errno_)); fflush(stderr);
    }

    awsm_assert(0);
    return 0;
}

/**
 * @brief Writes argument offsets and buffer into linear memory
 * Callers of this syscall only provide the base address of the two buffers because the WASI specification
 * assumes that the caller first called args_sizes_get and sized the buffers appropriately.
 *
 * @param argv_retptr
 * @param argv_buf_retptr
 * @return __WASI_ERRNO_SUCCESS or WASI_EINVAL
 */
__wasi_errno_t wasi_snapshot_preview1_backing_args_get(void* wasi_context_raw, char** argv, char* argv_buf) {
    if (wasi_context_raw == NULL || argv == NULL || argv_buf == NULL)
        return __WASI_ERRNO_INVAL;

    wasi_context_t* wasi_context = (wasi_context_t*)wasi_context_raw;

    memcpy(argv_buf, wasi_context->argv_buf, wasi_context->argv_buf_size);

    for (__wasi_size_t i = 0; i < wasi_context->argc; i++) {
        __wasi_size_t offset = wasi_context->argv[i] - wasi_context->argv_buf;
        argv[i]              = &argv_buf[wasi_context->argv[i] - wasi_context->argv_buf];
    }


    return __WASI_ERRNO_SUCCESS;
}

/**
 * @brief Writes the argument count and size of the requried argument buffer
 * This is called in order to size buffers that are subsequently passed to the WASI args_get syscall
 *
 * @param argc_retptr linear memory offset where we should write argc
 * @param argv_buf_len_retptr linear memory offset where we should write the length of the args buffer
 * @return __WASI_ERRNO_SUCCESS
 */
__wasi_errno_t wasi_snapshot_preview1_backing_args_sizes_get(void* wasi_context_raw, __wasi_size_t* argc_retptr,
                                                             __wasi_size_t* argv_buf_len_retptr) {
    if (wasi_context_raw == NULL || argc_retptr == NULL || argv_buf_len_retptr == NULL)
        return __WASI_ERRNO_INVAL;

    wasi_context_t* wasi_context = (wasi_context_t*)wasi_context_raw;

    *argc_retptr         = wasi_context->argc;
    *argv_buf_len_retptr = wasi_context->argv_buf_size;
    return __WASI_ERRNO_SUCCESS;
}

/**
 * @brief Return the resolution of a clock
 * Implementations are required to provide a non-zero value for supported clocks. For unsupported clocks,
 * return `errno::inval`.
 *
 * @param id The clock for which to return the resolution.
 * @param res_retptr - The resolution of the clock
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_clock_res_get(void* wasi_context, __wasi_clockid_t id, __wasi_timestamp_t* res_retptr) {
    /* similar to `clock_getres` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Return the time value of a clock
 *
 * @param clock_id The clock for which to return the time.
 * @param precision The maximum lag (exclusive) that the returned time value may have, compared to its actual value.
 * @param time_retptr  The time value of the clock.
 * @return __WASI_ERRNO_SUCCESS code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_clock_time_get(void* wasi_context_raw, __wasi_clockid_t clock_id,
                                              __wasi_timestamp_t precision, __wasi_timestamp_t* time_retptr) {
    struct timespec tp;
    int             rc = clock_gettime(clock_id, &tp);
    if (rc == -1) {
        return wasi_fromerrno(errno);
    }

    *time_retptr = (uint64_t)tp.tv_sec * 1000000000ULL + (uint64_t)tp.tv_nsec;

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Read environment variable data.
 * Callers of this syscall only provide the base address of the two buffers because the WASI specification
 * assumes that the caller first called environ_sizes_get and sized the buffers appropriately.
 *
 * @param environ_baseretptr
 * @param environ_buf_baseretptr
 * @return __WASI_ERRNO_SUCCESS or WASI_EINVAL
 */
__wasi_errno_t wasi_snapshot_preview1_backing_environ_get(void* wasi_context_raw, char** environ, char* environ_buf) {
    if (wasi_context_raw == NULL || environ == NULL || environ_buf == NULL)
        return __WASI_ERRNO_INVAL;

    wasi_context_t* wasi_context = (wasi_context_t*)wasi_context_raw;

    for (__wasi_size_t i = 0; i < wasi_context->envc; i++) {
        environ[i] = wasi_context->env_buf + (wasi_context->env[i] - wasi_context->env_buf);
    }

    memcpy(environ_buf, wasi_context->env_buf, wasi_context->env_buf_size);
    return __WASI_ERRNO_SUCCESS;
}

/**
 * Returns the environment variable count and the buffer size needed to store the environment strings in linear memory.
 * This is called in order to size buffers that are subsequently passed to the WASI environ_get syscall
 *
 * @param environ_len - the pointer where the resulting number of environment variable arguments should be written
 * @param environ_buf_len - the pointer where the resulting size of the environment variable data should be
 * written
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_environ_sizes_get(void* wasi_context_raw, __wasi_size_t* environ_len,
                                                                __wasi_size_t* environ_buf_len) {
    if (wasi_context_raw == NULL || environ_len == NULL || environ_buf_len == NULL)
        return __WASI_ERRNO_INVAL;

    wasi_context_t* wasi_context = (wasi_context_t*)wasi_context_raw;

    *environ_len     = wasi_context->envc;
    *environ_buf_len = wasi_context->env_buf_size;

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Provide file advisory information on a file descriptor.
 *
 * @param fd
 * @param offset The offset within the file to which the advisory applies.
 * @param len The length of the region to which the advisory applies.
 * @param advice
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_advise(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t offset,
                                                        __wasi_filesize_t len, __wasi_advice_t advice) {
    /* similar to `posix_fadvise` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Force the allocation of space in a file.
 *
 * @param fd
 * @param offset The offset at which to start the allocation.
 * @param len The length of the area that is allocated.
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_allocate(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t offset,
                                                          __wasi_filesize_t len) {
    /* similar to `posix_fallocate` in POSIX. */
    return wasi_unsupported_syscall(__func__);
};

/**
 * Close a file descriptor.
 *
 * @param fd
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_close(void* wasi_context, __wasi_fd_t fd) {
    /* TODO: Sandbox fd in context */
    int res = close(fd);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Synchronize the data of a file to disk.
 *
 * @param fd
 * @return __WASI_ERRNO_SUCCESS, WASI_EBADF, WASI_EIO, WASI_ENOSPC, WASI_EROFS, WASI_EINVAL, WASI_ENOSPC, WASI_EDQUOT
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_datasync(void* wasi_context, __wasi_fd_t fd) {
    /* TODO: Sandbox fd in context */
    int res = fdatasync(fd);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Get the attributes of a file descriptor.
 *
 * @param fd
 * @param fdstat_retptr the offset where the resulting wasi_fdstat structure should be written
 * @return __WASI_ERRNO_SUCCESS, WASI_EACCES, WASI_EAGAIN, WASI_EBADF, WASI_EFAULT, WASI_EINVAL, WASI_ELOOP,
 * WASI_ENAMETOOLONG, WASI_ENOTDIR, WASI_ENOENT, WASI_ENOMEM, or WASI_EOVERFLOW
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_fdstat_get(void* wasi_context, __wasi_fd_t fd, __wasi_fdstat_t* fdstat) {
    /* TODO: Sandbox fd in context */

    struct stat stat;
    int         res = fstat(fd, &stat);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }
    int mode = stat.st_mode;

    int fl = fcntl(fd, F_GETFL);
    if (fl < 0) {
        return wasi_fromerrno(errno);
    }

    fdstat->fs_filetype =
      ((S_ISBLK(mode) ? __WASI_FILETYPE_BLOCK_DEVICE : 0) | (S_ISCHR(mode) ? __WASI_FILETYPE_CHARACTER_DEVICE : 0)
       | (S_ISDIR(mode) ? __WASI_FILETYPE_DIRECTORY : 0) | (S_ISREG(mode) ? __WASI_FILETYPE_REGULAR_FILE : 0)
       | (S_ISSOCK(mode) ? __WASI_FILETYPE_SOCKET_STREAM : 0) | (S_ISLNK(mode) ? __WASI_FILETYPE_SYMBOLIC_LINK : 0));
    fdstat->fs_flags = (((fl & O_APPEND) ? __WASI_FDFLAGS_APPEND : 0) | ((fl & O_DSYNC) ? __WASI_FDFLAGS_DSYNC : 0)
                        | ((fl & O_NONBLOCK) ? __WASI_FDFLAGS_NONBLOCK : 0)
                        | ((fl & O_RSYNC) ? __WASI_FDFLAGS_RSYNC : 0) | ((fl & O_SYNC) ? __WASI_FDFLAGS_SYNC : 0));
    fdstat->fs_rights_base       = 0; /* all rights */
    fdstat->fs_rights_inheriting = 0; /* all rights */

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Adjust the flags associated with a file descriptor
 *
 * @param fd
 * @param fdflags The desired values of the file descriptor flags.
 * @return __WASI_ERRNO_SUCCESS, WASI_EACCES, WASI_EAGAIN, WASI_EBADF, WASI_EFAULT, WASI_EINVAL, WASI_ENOENT, or
 * WASI_EPERM
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_fdstat_set_flags(void* wasi_context, __wasi_fd_t fd, __wasi_fdflags_t fdflags) {
    /* TODO: Sandbox fd in context */

    int flags = (((flags & __WASI_FDFLAGS_APPEND) ? O_APPEND : 0) | ((flags & __WASI_FDFLAGS_DSYNC) ? O_DSYNC : 0)
                 | ((flags & __WASI_FDFLAGS_NONBLOCK) ? O_NONBLOCK : 0) | ((flags & __WASI_FDFLAGS_RSYNC) ? O_RSYNC : 0)
                 | ((flags & __WASI_FDFLAGS_SYNC) ? O_SYNC : 0));
    int err   = fcntl(fd, F_SETFL, fdflags);
    if (err < 0) {
        return wasi_fromerrno(errno);
    }
    return __WASI_ERRNO_SUCCESS;
}

/**
 * Adjust the rights associated with a file descriptor.
 * This can only be used to remove rights, and returns `errno::notcapable` if called in a way that would attempt to add
 * rights
 *
 * @param fd
 * @param fs_rights_base The desired rights of the file descriptor.
 * @param fs_rights_inheriting
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_fdstat_set_rights(void* wasi_context, __wasi_fd_t fd, __wasi_rights_t fs_rights_base,
                                                    __wasi_rights_t fs_rights_inheriting) {
    return wasi_unsupported_syscall(__func__);
}

/**
 * Return the attributes of an open file.
 *
 * @param fd
 * @param filestat_retptr The buffer where we should store the file's attributes
 * @return status code
 */

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_filestat_get(void* wasi_context, __wasi_fd_t fd, __wasi_filestat_t* filestat) {
    return wasi_unsupported_syscall(__func__);
}

/**
 * Adjust the size of an open file, zeroing extra bytes on increase
 *
 * @param fd
 * @param size The desired file size.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_filestat_set_size(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t size) {
    /* similar to `ftruncate` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Adjust the timestamps of an open file or directory
 *
 * @param fd
 * @param atim The desired values of the data access timestamp.
 * @param mtim The desired values of the data modification timestamp.
 * @param fst_flags A bitmask indicating which timestamps to adjust.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_filestat_set_times(void* wasi_context, __wasi_fd_t fd, __wasi_timestamp_t atim,
                                                     __wasi_timestamp_t mtim, __wasi_fstflags_t fst_flags) {
    /* similar to `futimens` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Read from a file descriptor without updating the descriptor's offset
 *
 * @param fd
 * @param iovs_baseptr List of scatter/gather vectors in which to store data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param offset The offset within the file at which to read.
 * @param nread_retptr The number of bytes read.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_pread(void* wasi_context, __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len,
                                        __wasi_filesize_t offset, __wasi_size_t* nread_retptr) {
    /* similar to `preadv` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Return a description of the given preopened file descriptor.
 *
 * @param fd
 * @param prestat_retptr The buffer where the description is stored.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_prestat_get(void* wasi_context, __wasi_fd_t fd, __wasi_prestat_t* prestat_retptr) {
    return __WASI_ERRNO_BADF;
}

/**
 * Return a description of the given preopened file descriptor.
 *
 * @param fd
 * @param path_retptr A buffer into which to write the preopened directory name.
 * @param path_len The length of the buffer at path_retptr
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_prestat_dir_name(void* wasi_context, __wasi_fd_t fd, char* path,
                                                                  __wasi_size_t path_len) {
    return wasi_unsupported_syscall(__func__);
}

/**
 * Write to a file descriptor without updating the descriptor's offset
 *
 * @param fd
 * @param iovs_baseptr List of scatter/gather vectors from which to retrieve data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param offset The offset within the file at which to write.
 * @param nwritten_retptr The number of bytes written.
 * @return status code
 *
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_pwrite(void* wasi_context, __wasi_fd_t fd, const __wasi_ciovec_t* iovs,
                                         size_t iovs_len, __wasi_filesize_t offset, __wasi_size_t* nwritten_retptr) {
    /* similar to `pwritev` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Read from a file descriptor
 *
 * @param fd
 * @param iovs_baseptr
 * @param iovs_len
 * @param nwritten_retptr The number of bytes read.
 * @return __WASI_ERRNO_SUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT, WASI_EINTR, WASI_EIO,
 * WASI_EISDIR, or others
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_read(void* wasi_context, __wasi_fd_t fd, const __wasi_iovec_t* iovs,
                                                      size_t iovs_len, __wasi_size_t* nwritten_retptr) {
    /* TODO: Should this use readv instead of read? */
    __wasi_size_t sum = 0;

    for (int i = 0; i < iovs_len; i++) {
        ssize_t res = read(fd, iovs[i].buf, iovs[i].buf_len);
        if (res == -1) {
            return wasi_fromerrno(errno);
        }

        sum += res;
    }

    *nwritten_retptr = sum;
    return __WASI_ERRNO_SUCCESS;
}

/**
 * Read directory entries from a directory.
 * When successful, the contents of the output buffer consist of a sequence of
 * directory entries. Each directory entry consists of a `dirent` object,
 * followed by `dirent::d_namlen` bytes holding the name of the directory entry.
 * This function fills the output buffer as much as possible, potentially
 * truncating the last directory entry. This allows the caller to grow its
 * read buffer size in case it's too small to fit a single large directory
 * entry, or skip the oversized directory entry.
 *
 * @param fd
 * @param buf_baseptr The buffer where directory entries are stored
 * @param buf_len
 * @param cookie The location within the directory to start reading
 * @param nwritten_retptr The number of bytes stored in the read buffer. If less than the size of the read buffer, the
 * end of the directory has been reached.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_readdir(void* wasi_context, __wasi_fd_t fd, uint8_t* buf, __wasi_size_t buf_len,
                                          __wasi_dircookie_t cookie, __wasi_size_t* nwritten_retptr) {
    return wasi_unsupported_syscall(__func__);
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
__wasi_errno_t wasi_snapshot_preview1_backing_fd_renumber(void* wasi_context, __wasi_fd_t fd, __wasi_fd_t to) {
    return wasi_unsupported_syscall(__func__);
}

/**
 * Move the offset of a file descriptor
 *
 * @param fds
 * @param file_offset The number of bytes to move.
 * @param whence The base from which the offset is relative.
 * @param newoffset_retptr The new offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_seek(void* wasi_context, __wasi_fd_t fd, __wasi_filedelta_t file_offset,
                                       __wasi_whence_t whence, __wasi_filesize_t* newoffset_retptr) {
    /* TODO: Does whence differ from POSIX? */
    off_t res = lseek(fd, (off_t)file_offset, whence);

    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    *newoffset_retptr = res;
    return __WASI_ERRNO_SUCCESS;
}

/**
 * Synchronize the data and metadata of a file to disk
 *
 * @param fd
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_sync(void* wasi_context, __wasi_fd_t fd) {
    /* similar to `fsync` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Return the current offset of a file descriptor
 *
 * @param fd
 * @param fileoffset_retptr The current offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_fd_tell(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t* fileoffset_retptr) {
    /* similar to `lseek(fd, 0, SEEK_CUR)` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Write to a file descriptor
 *
 * @param fd
 * @param iovs_baseptr List of scatter/gather vectors from which to retrieve data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param nwritten_retptr
 * @return __WASI_ERRNO_SUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT,
 * WASI_EFBIG, WASI_EINTR, WASI_EIO, WASI_ENOSPC, WASI_EPERM, WASI_EPIPE, or others
 */
__wasi_errno_t wasi_snapshot_preview1_backing_fd_write(void* wasi_context, __wasi_fd_t fd, const __wasi_ciovec_t* iovs,
                                                       size_t iovs_len, __wasi_size_t* nwritten_retptr) {
    /* TODO: Should this use writev instead of write? */
    __wasi_size_t sum = 0;

    for (int i = 0; i < iovs_len; i++) {
        ssize_t res = write(fd, iovs[i].buf, iovs[i].buf_len);
        if (res == -1) {
            return wasi_fromerrno(errno);
        }
        sum += res;
    }
    *nwritten_retptr = sum;

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Create a directory
 *
 * @param fd
 * @param path_baseptr
 * @param path_len
 * @return __WASI_ERRNO_SUCCESS, WASI_EACCES, WASI_EBADF, WASI_EDQUOT, WASI_EEXIST,
 * WASI_EFAULT, WASI_EINVAL, WASI_ELOOP, WASI_EMLINK, WASI_ENAMETOOLONG,
 * WASI_ENOENT, WASI_ENOMEM, WASI_ENOSPC, WASI_ENOTDIR, WASI_EPERM, or WASI_EROFS
 */
__wasi_errno_t wasi_snapshot_preview1_backing_path_create_directory(void* wasi_context, __wasi_fd_t fd,
                                                                    const char* path, __wasi_size_t path_len) {
    /* TODO: Can we 100% guarantee that path is null terminated here? Likely not! */
    int res = mkdirat(fd, path, 0777);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Return the attributes of a file or directory
 *
 * @param fd
 * @param flags Flags determining the method of how the path is resolved.
 * @param path_baseptr The path of the file or directory to inspect.
 * @param filestat_retptr The buffer where the file's attributes are stored.
 * @return __WASI_ERRNO_SUCCESS, WASI_EACCES, WASI_EBAD, WASI_EFAUL, WASI_EINVAL, WASI_ELOOP,
 * WASI_ENAMETOOLON, WASI_ENOENT, WASI_ENOENT, WASI_ENOMEM, WASI_ENOTDI, or WASI_EOVERFLOW
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_path_filestat_get(void* wasi_context, __wasi_fd_t fd, __wasi_lookupflags_t flags,
                                                 const char* path, __wasi_size_t path_len,
                                                 __wasi_filestat_t* const filestat) {
    /* TODO: Can we 100% guarantee that path is null terminated here? Likely not! */
    struct stat stat;
    int         res = fstatat(fd, path, &stat, 0);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    filestat->dev      = stat.st_dev;
    filestat->ino      = stat.st_ino;
    filestat->filetype = stat.st_mode;
    filestat->nlink    = stat.st_nlink;
    filestat->size     = stat.st_size;
    filestat->atim     = stat.st_atime;
    filestat->mtim     = stat.st_mtime;
    filestat->ctim     = stat.st_ctime;

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Adjust the timestamps of a file or directory
 *
 * @param fd
 * @param flags Flags determining the method of how the path is resolved.
 * @param path_baseptr The path of the file or directory to operate on.
 * @param path_len
 * @param atim The desired values of the data access timestamp.
 * @param mtim The desired values of the data modification timestamp.
 * @param fst_flags A bitmask indicating which timestamps to adjust.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_path_filestat_set_times(void* wasi_context, __wasi_fd_t fd, __wasi_lookupflags_t flags,
                                                       const char* path, __wasi_size_t path_len,
                                                       __wasi_timestamp_t atim, __wasi_timestamp_t mtim,
                                                       __wasi_fstflags_t fst_flags) {
    /* similar to `utimensat` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Create a hard link
 *
 * @param old_fd
 * @param old_flags Flags determining the method of how the path is resolved.
 * @param old_path_baseptr The source path from which to link.
 * @param old_path_len
 * @param new_fd The working directory at which the resolution of the new path starts.
 * @param new_path_baseptr The destination path at which to create the hard link.
 * @param new_path_len
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_path_link(void* wasi_context, __wasi_fd_t old_fd, __wasi_lookupflags_t old_flags,
                                         const char* old_path, __wasi_size_t old_path_len, __wasi_fd_t new_fd,
                                         const char* new_path, __wasi_size_t new_path_len) {
    /* similar to `linkat` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Open a file or directory
 * The returned file descriptor is not guaranteed to be the lowest-numbered
 * file descriptor not currently open; it is randomized to prevent
 * applications from depending on making assumptions about indexes, since this
 * is error-prone in multi-threaded contexts. The returned file descriptor is
 * guaranteed to be less than 2**31.
 *
 * The initial rights of the newly created file descriptor. The
 * implementation is allowed to return a file descriptor with fewer rights
 * than specified, if and only if those rights do not apply to the type of
 * file being opened.
 * The *base* rights are rights that will apply to operations using the file
 * descriptor itself, while the *inheriting* rights are rights that apply to
 * file descriptors derived from it.
 *
 * @param dirfd
 * @param lookupflags Flags determining the method of how the path is resolved.
 * @param path_baseptr path of the file or directory to open relative to the fd directory.
 * @param path_len
 * @param oflags The method by which to open the file.
 *

 *
 * @param fs_rights_base
 * @param fs_rights_inheriting
 * @param fdflags
 * @param fd_off The file descriptor of the file that has been opened.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_path_open(void* wasi_context, __wasi_fd_t dirfd, __wasi_lookupflags_t dirflags,
                                         const char* path, __wasi_size_t path_len, __wasi_oflags_t oflags,

                                         __wasi_rights_t fs_rights_base, __wasi_rights_t fs_rights_inheriting,
                                         __wasi_fdflags_t fdflags, __wasi_fd_t* fd) {
    /* translate o_flags and fs_flags into flags and mode */
    int flags = (((oflags & __WASI_OFLAGS_CREAT) ? O_CREAT : 0) | ((oflags & __WASI_OFLAGS_DIRECTORY) ? O_DIRECTORY : 0)
                 | ((oflags & __WASI_OFLAGS_EXCL) ? O_EXCL : 0) | ((oflags & __WASI_OFLAGS_TRUNC) ? O_TRUNC : 0)
                 | ((fdflags & __WASI_FDFLAGS_APPEND) ? O_APPEND : 0) | ((fdflags & __WASI_FDFLAGS_DSYNC) ? O_DSYNC : 0)
                 | ((fdflags & __WASI_FDFLAGS_NONBLOCK) ? O_NONBLOCK : 0)
                 | ((fdflags & __WASI_FDFLAGS_RSYNC) ? O_RSYNC : 0) | ((fdflags & __WASI_FDFLAGS_SYNC) ? O_SYNC : 0));
    if ((fs_rights_base & __WASI_RIGHTS_FD_WRITE) && (fs_rights_base & __WASI_RIGHTS_FD_READ)) {
        flags |= O_RDWR;
    } else if (fs_rights_base & __WASI_RIGHTS_FD_WRITE) {
        flags |= O_WRONLY;
    } else if (fs_rights_base & __WASI_RIGHTS_FD_READ) {
        flags |= O_RDONLY;
    }

    int mode = 0644;
    int rc   = openat(dirfd, path, flags, mode);
    if (rc < 0) {
        return wasi_fromerrno(errno);
    }

    *fd = rc;
    return __WASI_ERRNO_SUCCESS;
}

/**
 * Read the contents of a symbolic link
 *
 * @param fd
 * @param path_baseptr The path of the symbolic link from which to read.
 * @param path_len
 * @param buf_baseretptr The buffer to which to write the contents of the symbolic link.
 * @param buf_len
 * @param nread_retptr The number of bytes placed in the buffer.
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_path_readlink(void* wasi_context, __wasi_fd_t fd, const char* path,
                                                            __wasi_size_t path_len, uint8_t* buf, __wasi_size_t buf_len,
                                                            __wasi_size_t* nread_retptr) {
    /* similar to `readlinkat` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Remove a directory
 * Return `errno::notempty` if the directory is not empty.
 *
 * @param fd
 * @param path_baseptr The path to a directory to remove.
 * @param path_len
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_path_remove_directory(void* wasi_context, __wasi_fd_t fd,
                                                                    const char* path, __wasi_size_t path_len) {
    /* similar to `unlinkat(fd, path, AT_REMOVEDIR)` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Rename a file or directory
 *
 * @param fd
 * @param old_path The source path of the file or directory to rename.
 * @param new_fd The working directory at which the resolution of the new path starts.
 * @param new_path The destination path to which to rename the file or directory.
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_path_rename(void* wasi_context, __wasi_fd_t fd, const char* old_path,
                                                          __wasi_size_t old_path_len, __wasi_fd_t new_fd,
                                                          const char* new_path, __wasi_size_t new_path_len) {
    /* similar to `renameat` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Create a symbolic link
 *
 * @param old_path_baseptr The contents of the symbolic link.
 * @param old_path_len
 * @param fd
 * @param new_path_baseptr The path where we want the symbolic link.
 * @param new_path_len
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_path_symlink(void* wasi_context, const char* old_path, __wasi_size_t old_path_len,
                                            __wasi_fd_t fd, const char* new_path, __wasi_size_t new_path_len) {
    /* similar to `symlinkat` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Unlink a file
 * Return `errno::isdir` if the path refers to a directory.
 *
 * @param fd
 * @param path_baseptr
 * @param path_len
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_path_unlink_file(void* wasi_context, __wasi_fd_t fd, const char* path,
                                                               __wasi_size_t path_len) {
    /* TODO: Path likely is not null terminated */
    int res = unlinkat(fd, path, 0);
    if (res == -1) {
        return wasi_fromerrno(errno);
    }

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Concurrently poll for the occurrence of a set of events.
 *
 * @param in The events to which to subscribe.
 * @param out The events that have occurred.
 * @param nsubscriptions Both the number of subscriptions and events.
 * @param retptr The number of events stored.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_poll_oneoff(void* wasi_context, const __wasi_subscription_t* in, __wasi_event_t* out,
                                           __wasi_size_t nsubscriptions, __wasi_size_t* retptr0) {
    return wasi_unsupported_syscall(__func__);
}

/**
 * Terminate the process normally. An exit code of 0 indicates successful
 * termination of the program. The meanings of other values is dependent on
 * the environment.
 *
 * @param exitcode
 */
__attribute__((noreturn)) void
wasi_snapshot_preview1_backing_proc_exit(void* wasi_context, __wasi_exitcode_t exitcode) {
    exit(exitcode);
}

/**
 * Send a signal to the process of the calling thread.
 *
 * @param sig The signal condition to trigger.
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_proc_raise(void* wasi_context, __wasi_signal_t sig) {
    /* similar to `raise` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}

/**
 * Write high-quality random data into a buffer.
 * This function blocks when the implementation is unable to immediately
 * provide sufficient high-quality random data.
 * This function may execute slowly, so when large mounts of random data are
 * required, it's advisable to use this function to seed a pseudo-random
 * number generator, rather than to provide the random data directly.
 *
 * @param buf_baseretptr The buffer to fill with random data.
 * @param buf_len The length of the buffer
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_random_get(void* wasi_context, uint8_t* buf, __wasi_size_t buf_len) {
    static bool has_udev = true;
    static bool did_seed = false;

    if (!has_udev)
        goto NO_UDEV;

    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        has_udev = false;
        goto NO_UDEV;
    }

    ssize_t nread = 0;
    while (nread < buf_len) {
        size_t rc = read(urandom_fd, buf, buf_len);
        if (rc < 0)
            goto ERR_READ;
        nread += rc;
    }

    close(urandom_fd);
    return __WASI_ERRNO_SUCCESS;

ERR_READ:
    close(urandom_fd);
    return wasi_fromerrno(errno);
NO_UDEV:
    do {
        __wasi_size_t buf_cursor = 0;
        for (; (buf_len - buf_cursor) >= 4; buf_cursor += 4) {
            *((int*)(buf + buf_cursor)) = random();
        }
        for (; buf_cursor < buf_len; buf_cursor += 1) {
            *(buf + buf_cursor) = random() % UINT8_MAX;
        }
    } while (0);

    return __WASI_ERRNO_SUCCESS;
}

/**
 * Temporarily yield execution of the calling thread similar to `sched_yield` in POSIX.
 * This implementation ignores client calls and silently returns RC 0
 *
 * @return __WASI_ERRNO_SUCCESS
 */
__wasi_errno_t wasi_snapshot_preview1_backing_sched_yield(void* wasi_context) {
    return __WASI_ERRNO_SUCCESS;
}

/**
 * Receive a message from a socket.
 * Note: This is similar to `recv` in POSIX, though it also supports reading
 * the data into multiple buffers in the manner of `readv`.
 *
 * @param fd
 * @param ri_data_baseretptr List of scatter/gather vectors to which to store data.
 * @param ri_data_len The length of the array pointed to by `ri_data`.
 * @param ri_flags Message flags.
 * @param ri_data_nbytes_retptr Number of bytes stored in ri_data flags.
 * @param message_nbytes_retptr Number of bytes stored in message flags.
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_sock_recv(void* wasi_context, __wasi_fd_t fd,
                                                        const __wasi_iovec_t* ri_data, size_t ri_data_len,
                                                        __wasi_riflags_t ri_flags, __wasi_size_t* ri_data_nbytes_retptr,
                                                        __wasi_roflags_t* message_nbytes_retptr) {
    return wasi_unsupported_syscall(__func__);
}

/**
 * Send a message on a socket.
 * Note: This is similar to `send` in POSIX, though it also supports writing
 * the data from multiple buffers in the manner of `writev`.
 *
 * @param fd
 * @param si_data_baseptr List of scatter/gather vectors to which to retrieve data
 * @param si_data_len The length of the array pointed to by `si_data`.
 * @param si_flags Message flags.
 * @param nsent_retptr Number of bytes transmitted.
 * @return status code
 */
__wasi_errno_t
wasi_snapshot_preview1_backing_sock_send(void* wasi_context, __wasi_fd_t fd, const __wasi_ciovec_t* si_data,
                                         size_t si_data_len, __wasi_siflags_t si_flags, __wasi_size_t* nsent_retptr) {
    return wasi_unsupported_syscall(__func__);
}

/**
 * Shut down socket send and receive channels.
 *
 * @param fd
 * @param how Which channels on the socket to shut down.
 * @return status code
 */
__wasi_errno_t wasi_snapshot_preview1_backing_sock_shutdown(void* wasi_context, __wasi_fd_t fd, __wasi_sdflags_t how) {
    /* similar to `shutdown` in POSIX. */
    return wasi_unsupported_syscall(__func__);
}
