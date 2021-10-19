#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wasi_impl.h"
#include "wasi_serdes.h"

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef CURRENT_MEMORY_BASE
#error CURRENT_MEMORY_BASE not defined
#endif

#ifndef CURRENT_MEMORY_SIZE
#error CURRENT_MEMORY_SIZE not defined
#endif

#ifndef CURRENT_WASI_CONTEXT
#error CURRENT_WASI_CONTEXT symbol not defined
#endif

/**
 * @brief Writes argument offsets and buffer into linear memory write
 *
 * @param argv_retptr
 * @param argv_buf_retptr
 * @return status code
 */
uint32_t wasi_snapshot_preview1_args_get(__wasi_size_t argv_retptr, __wasi_size_t argv_buf_retptr) {
    awsm_assert(CURRENT_WASI_CONTEXT != NULL);
    awsm_assert(CURRENT_MEMORY_BASE != NULL);

    char**         argv_ptrs    = NULL;
    __wasi_size_t* argv_offsets = NULL;
    __wasi_size_t  rc           = 0;

    /* Unpack WASI config values */
    const __wasi_size_t argc          = wasi_context_get_argc(CURRENT_WASI_CONTEXT);
    const __wasi_size_t argv_buf_size = wasi_context_get_argv_buf_size(CURRENT_WASI_CONTEXT);

    if (unlikely(argc == 0)) {
        goto done;
    }

    /* Check Bounds */
    wasi_serdes_check_array_bounds(argv_retptr, CURRENT_MEMORY_SIZE, sizeof(__wasi_size_t), argc);
    wasi_serdes_check_bounds(argv_buf_retptr, CURRENT_MEMORY_SIZE, argv_buf_size);

    /* args_get backings return a vector of host pointers. We need a host buffer to store this
     * temporarily so we don't leak host addresses into the linear memory */
    argv_ptrs = calloc(sizeof(char*), argc);

    /* Convert offsets to pointers */
    argv_offsets   = &CURRENT_MEMORY_BASE[argv_retptr];
    char* argv_buf = &CURRENT_MEMORY_BASE[argv_buf_retptr];

    /* Write the argv buffer to to linear memory and the argv vector to our temporary buffer*/
    rc = wasi_snapshot_preview1_backing_args_get(CURRENT_WASI_CONTEXT, argv_ptrs, argv_buf);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Convery pointers in our temporary argv buffer into offsets and write to linear memory */
    for (int i = 0; i < argc; i++) {
        argv_offsets[i] = argv_buf_retptr + (uint32_t)(argv_ptrs[i] - argv_ptrs[0]);
    }

done:
    if (likely(argv_ptrs != NULL)) {
        free(argv_ptrs);
        argv_ptrs = NULL;
    }

    return (uint32_t)rc;
}

/**
 * @brief Used by a WASI module to determine the argument count and size of the requried
 * argument buffer
 *
 * @param argc linear memory offset where we should write argc
 * @param argv_buf_len linear memory offset where we should write the length of the args buffer
 * @return status code
 */
uint32_t wasi_snapshot_preview1_args_sizes_get(__wasi_size_t argc_retptr, __wasi_size_t argv_buf_len_retptr) {
    __wasi_size_t* argc         = NULL;
    __wasi_size_t* argv_buf_len = NULL;
    __wasi_errno_t rc           = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(argc_retptr, CURRENT_MEMORY_SIZE, sizeof(uint32_t));
    wasi_serdes_check_bounds(argv_buf_len_retptr, CURRENT_MEMORY_SIZE, sizeof(uint32_t));

    /* Convert offsets to pointers */
    argc         = &CURRENT_MEMORY_BASE[argc_retptr];
    argv_buf_len = &CURRENT_MEMORY_BASE[argv_buf_len_retptr];

    /* Get sizes */
    rc = wasi_snapshot_preview1_backing_args_sizes_get(CURRENT_WASI_CONTEXT, argc, argv_buf_len);

done:
    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_clock_res_get(__wasi_clockid_t id, __wasi_size_t res_retptr) {
    __wasi_errno_t      rc  = 0;
    __wasi_timestamp_t* res = NULL;

    /* Check Bounds */
    wasi_serdes_check_bounds(res_retptr, CURRENT_MEMORY_SIZE, sizeof(__wasi_timestamp_t));

    /* Convert offsets to pointers */
    res = &CURRENT_MEMORY_BASE[res_retptr];

    /* Read resolution into buffer */
    rc = wasi_snapshot_preview1_backing_clock_res_get(CURRENT_WASI_CONTEXT, id, res);

done:
    return (uint32_t)rc;
}

/**
 * Return the time value of a clock
 *
 * @param clock_id The clock for which to return the time.
 * @param precision The maximum lag (exclusive) that the returned time value may have, compared to its actual value.
 * @param time_retptr  The time value of the clock.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_clock_time_get(__wasi_clockid_t clock_id, __wasi_timestamp_t precision,
                                               __wasi_size_t time_retptr) {
    __wasi_errno_t      rc   = 0;
    __wasi_timestamp_t* time = NULL;

    /* Check Bounds */
    wasi_serdes_check_bounds(time_retptr, CURRENT_MEMORY_SIZE, sizeof(__wasi_timestamp_t));

    /* Convert offsets to pointers */
    time = &CURRENT_MEMORY_BASE[time_retptr];

    /* Make WASI call */
    rc = wasi_snapshot_preview1_backing_clock_time_get(CURRENT_WASI_CONTEXT, clock_id, precision, time);

done:
    return (uint32_t)rc;
}

/**
 * Read environment variable data.
 * The sizes of the buffers should match that returned by `environ_sizes_get`.
 *
 * @param environ_retptr
 * @param environ_buf_retptr
 * @return status code
 */
uint32_t wasi_snapshot_preview1_environ_get(__wasi_size_t environ_retptr, __wasi_size_t environ_buf_retptr) {
    __wasi_errno_t rc              = 0;
    char*          environ_buf     = NULL;
    __wasi_size_t* environ_offsets = NULL;
    char**         environ_ptrs    = NULL;

    const __wasi_size_t envc = wasi_context_get_envc(CURRENT_WASI_CONTEXT);
    if (envc == 0) {
        goto done;
    }

    const __wasi_size_t env_buf_size = wasi_context_get_env_buf_size(CURRENT_WASI_CONTEXT);
    awsm_assert(env_buf_size > envc);

    /* Check Bounds */
    wasi_serdes_check_array_bounds(environ_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t, envc);
    wasi_serdes_check_bounds(environ_buf_retptr, CURRENT_MEMORY_SIZE, env_buf_size);

    /* environ_get backings return a vector of host pointers. We need a host buffer to store this
     * temporarily so we don't leak host addresses into the linear memory */
    environ_ptrs = calloc(sizeof(char*), envc);

    /* Convert offsets to pointers */
    environ_offsets = &CURRENT_MEMORY_BASE[environ_retptr];
    environ_buf     = &CURRENT_MEMORY_BASE[environ_buf_retptr];

    /* Write environment to temporary buffer and environment_buf directly to linear memory */
    rc = wasi_snapshot_preview1_backing_environ_get(CURRENT_WASI_CONTEXT, environ_ptrs, environ_buf);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Convert pointers in our temporary buffer into offsets and write to linear memory */
    for (int i = 0; i < envc; i++) {
        environ_offsets[i] = environ_buf_retptr + (uint32_t)(environ_ptrs[i] - environ_ptrs[0]);
    }

done:
    if (likely(environ_ptrs != NULL)) {
        free(environ_ptrs);
        environ_ptrs = NULL;
    }

    return (uint32_t)rc;
}

/**
 * Returns the number of environment variable arguments and the size of the environment variable data.
 *
 * @param environc_retptr - the offset where the resulting number of environment variable arguments should be written
 * @param environv_buf_len_retptr - the offset where the resulting size of the environment variable data should be
 * written
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_environ_sizes_get(__wasi_size_t environc_retptr, __wasi_size_t environv_buf_len_retptr) {
    __wasi_size_t* envc             = NULL;
    __wasi_size_t* environv_buf_len = NULL;
    __wasi_errno_t rc               = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(environc_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);
    wasi_serdes_check_bounds(environv_buf_len_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Convert offsets to host pointers */
    envc             = &CURRENT_MEMORY_BASE[environc_retptr];
    environv_buf_len = &CURRENT_MEMORY_BASE[environv_buf_len_retptr];

    /* Make WASI Call */
    rc = wasi_snapshot_preview1_backing_environ_sizes_get(CURRENT_WASI_CONTEXT, envc, environv_buf_len);

done:
    return (uint32_t)rc;
}

/**
 * Provide file advisory information on a file descriptor.
 * Note: similar to `posix_fadvise` in POSIX
 *
 * @param fd
 * @param offset The offset within the file to which the advisory applies.
 * @param len The length of the region to which the advisory applies.
 * @param advice
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_fd_advise(__wasi_fd_t fd, __wasi_filesize_t offset, __wasi_filesize_t len, uint32_t advice) {
    __wasi_errno_t rc = 0;

    if (unlikely(advice > UINT8_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_fd_advise(CURRENT_WASI_CONTEXT, fd, offset, len, (__wasi_advice_t)advice);

done:
    return (uint32_t)rc;
}

/**
 * Force the allocation of space in a file.
 *
 * @param fd
 * @param offset The offset at which to start the allocation.
 * @param len The length of the area that is allocated.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_allocate(__wasi_fd_t fd, __wasi_filesize_t offset, __wasi_filesize_t len) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_fd_allocate(CURRENT_WASI_CONTEXT, fd, offset, len);

done:
    return (uint32_t)rc;
};

/**
 * Close a file descriptor.
 *
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_close(__wasi_fd_t fd) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_fd_close(CURRENT_WASI_CONTEXT, fd);

done:
    return (uint32_t)rc;
}

/**
 * Synchronize the data of a file to disk.
 *
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_datasync(__wasi_fd_t fd) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_fd_datasync(CURRENT_WASI_CONTEXT, fd);

done:
    return (uint32_t)rc;
}

/**
 * Get the attributes of a file descriptor.
 *
 * @param fd
 * @param fdstat_retptr the offset where the resulting wasi_fdstat structure should be written
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_fdstat_get(__wasi_fd_t fd, __wasi_size_t fdstat_retptr) {
    __wasi_errno_t   rc    = 0;
    __wasi_fdstat_t* stats = NULL;

    /* Check Bounds */
    wasi_serdes_check_bounds(fdstat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_fdstat_t);

    /* Convert offset to host pointer */
    stats = &CURRENT_MEMORY_BASE[fdstat_retptr];

    /* Make WASI Call */
    rc = wasi_snapshot_preview1_backing_fd_fdstat_get(CURRENT_WASI_CONTEXT, fd, stats);

done:
    return (uint32_t)rc;
}

/**
 * Adjust the flags associated with a file descriptor
 *
 * @param fd
 * @param fdflags The desired values of the file descriptor flags.
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EAGAIN, WASI_EBADF, WASI_EFAULT, WASI_EINVAL, WASI_ENOENT, or WASI_EPERM
 */
uint32_t wasi_snapshot_preview1_fd_fdstat_set_flags(__wasi_fd_t fd, uint32_t fdflags) {
    __wasi_errno_t rc = 0;

    /* fdflags should be a zero-extended uint16_t */
    if (unlikely(fdflags > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_fdstat_set_flags(CURRENT_WASI_CONTEXT, fd, (__wasi_fdflags_t)fdflags);

done:
    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_fd_fdstat_set_rights(__wasi_fd_t fd, __wasi_rights_t fs_rights_base,
                                                     __wasi_rights_t fs_rights_inheriting) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_fdstat_set_rights(CURRENT_WASI_CONTEXT, fd, fs_rights_base,
                                                          fs_rights_inheriting);

done:
    return (uint32_t)rc;
}

/**
 * Return the attributes of an open file.
 *
 * @param fd
 * @param filestat_retptr The buffer where we should store the file's attributes
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_filestat_get(__wasi_fd_t fd, __wasi_size_t filestat_retptr) {
    __wasi_errno_t     rc    = 0;
    __wasi_filestat_t* stats = NULL;

    /* Check Bounds */
    wasi_serdes_check_bounds(filestat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filestat_t);

    /* Convert offset to host pointer */
    stats = &CURRENT_MEMORY_BASE[filestat_retptr];

    /* Make WASI Call */
    rc = wasi_snapshot_preview1_backing_fd_filestat_get(CURRENT_WASI_CONTEXT, fd, stats);

done:
    return (uint32_t)rc;
}

/**
 * Adjust the size of an open file, zeroing extra bytes on increase
 *
 * @param fd
 * @param size The desired file size.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_filestat_set_size(__wasi_fd_t fd, __wasi_filesize_t size) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_fd_filestat_set_size(CURRENT_WASI_CONTEXT, fd, size);

done:
    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_fd_filestat_set_times(__wasi_fd_t fd, __wasi_timestamp_t atim, __wasi_timestamp_t mtim,
                                                      uint32_t fst_flags) {
    __wasi_errno_t rc = 0;

    if (unlikely(fst_flags > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_filestat_set_times(CURRENT_WASI_CONTEXT, fd, atim, mtim,
                                                           (__wasi_fstflags_t)fst_flags);

done:
    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_fd_pread(__wasi_fd_t fd, __wasi_size_t iovs_baseptr, __wasi_size_t iovs_len,
                                         __wasi_filesize_t offset, __wasi_size_t nread_retptr) {
    __wasi_iovec_t* iovs  = NULL;
    __wasi_size_t*  nread = NULL;
    __wasi_errno_t  rc    = 0;

    /* Check Bounds */
    wasi_serdes_check_array_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_iovec_t, iovs_len);
    wasi_serdes_check_bounds(nread_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Convert offsets to host pointers */
    nread = &CURRENT_MEMORY_BASE[nread_retptr];

    /* Read iovec from linear memory, converting offsets to host pointers */
    iovs = calloc(iovs_len, sizeof(__wasi_iovec_t));
    if (unlikely(iovs == NULL)) {
        goto done;
    }
    rc = wasi_serdes_readv_iovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Execute WASI call */
    rc = wasi_snapshot_preview1_backing_fd_pread(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, offset, nread);

done:
    if (likely(iovs != NULL)) {
        free(iovs);
        iovs = NULL;
    }
    return (uint32_t)rc;
}

/**
 * Return a description of the given preopened file descriptor.
 *
 * @param fd
 * @param prestat_retptr The buffer where the description is stored.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_prestat_get(__wasi_fd_t fd, __wasi_size_t prestat_retptr) {
    __wasi_prestat_t* prestat = NULL;
    __wasi_errno_t    rc      = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(prestat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_prestat_t);

    /* Convert Offset to Host Pointer */
    prestat = &CURRENT_MEMORY_BASE[prestat_retptr];

    /* Execute WASI call */
    rc = wasi_snapshot_preview1_backing_fd_prestat_get(CURRENT_WASI_CONTEXT, fd, prestat);

done:
    return (uint32_t)rc;
}

/**
 * Return a description of the given preopened file descriptor.
 *
 * @param fd
 * @param path_retptr A buffer into which to write the preopened directory name.
 * @param path_len The length of the buffer at path_retptr
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_prestat_dir_name(__wasi_fd_t fd, __wasi_size_t path_retptr, __wasi_size_t path_len) {
    char*          path = NULL;
    __wasi_errno_t rc   = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(path_retptr, CURRENT_MEMORY_SIZE, path_len);

    /* Convert Offset to Host Pointer */
    path = (char*)&CURRENT_MEMORY_BASE[path_retptr];

    /* Execute WASI call, writing results directly to linear memory */
    rc = wasi_snapshot_preview1_backing_fd_prestat_dir_name(CURRENT_WASI_CONTEXT, fd, path, path_len);

done:
    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_fd_pwrite(__wasi_fd_t fd, __wasi_size_t iovs_baseptr, __wasi_size_t iovs_len,
                                          __wasi_filesize_t offset, __wasi_size_t nwritten_retptr) {
    __wasi_ciovec_t* iovs     = NULL;
    __wasi_size_t*   nwritten = NULL;
    __wasi_errno_t   rc       = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, iovs_len * WASI_SERDES_SIZE_ciovec_t);
    wasi_serdes_check_bounds(nwritten_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Convert Offset to Host Pointer */
    nwritten = &CURRENT_MEMORY_BASE[nwritten_retptr];

    /* Copy iovs into memory */
    iovs = calloc(iovs_len, sizeof(__wasi_ciovec_t));
    if (unlikely(iovs == NULL)) {
        goto done;
    }
    rc = wasi_serdes_readv_ciovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Execute WASI call */
    rc = wasi_snapshot_preview1_backing_fd_pwrite(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, offset, nwritten);

done:
    if (likely(iovs != NULL)) {
        free(iovs);
        iovs = NULL;
    }

    return (uint32_t)rc;
}

/**
 * Read from a file descriptor
 *
 * @param fd
 * @param iovs_baseptr
 * @param iovs_len
 * @param nwritten_retptr The number of bytes read.
 * @return WASI_ESUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT, WASI_EINTR, WASI_EIO, WASI_EISDIR, or
 * others
 */
uint32_t wasi_snapshot_preview1_fd_read(__wasi_fd_t fd, __wasi_size_t iovs_baseptr, __wasi_size_t iovs_len,
                                        __wasi_size_t nread_retptr) {
    __wasi_iovec_t* iovs  = NULL;
    __wasi_size_t*  nread = NULL;
    __wasi_errno_t  rc    = 0;

    /* Check Bounds */
    wasi_serdes_check_array_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_iovec_t, iovs_len);
    wasi_serdes_check_bounds(nread_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Convert offsets to host pointers */
    nread = &CURRENT_MEMORY_BASE[nread_retptr];

    /* Copy iovs into memory, converting offsets into host pointers */
    iovs = calloc(iovs_len, sizeof(__wasi_iovec_t));
    if (unlikely(iovs == NULL)) {
        goto done;
    }
    rc = wasi_serdes_readv_iovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Execute WASI call */
    rc = wasi_snapshot_preview1_backing_fd_read(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, nread);

done:
    return (uint32_t)rc;
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
 * @param nread_retptr The number of bytes stored in the read buffer. If less than the size of the read buffer, the end
 * of the directory has been reached.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_readdir(__wasi_fd_t fd, __wasi_size_t buf_baseptr, __wasi_size_t buf_len,
                                           __wasi_dircookie_t cookie, __wasi_size_t nread_retptr) {
    uint8_t*       buf   = NULL;
    __wasi_size_t* nread = NULL;
    __wasi_errno_t rc    = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(buf_baseptr, CURRENT_MEMORY_SIZE, buf_len);
    wasi_serdes_check_bounds(nread_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Convert offsets to host pointers */
    buf   = &CURRENT_MEMORY_BASE[buf_baseptr];
    nread = &CURRENT_MEMORY_BASE[nread_retptr];

    /* Execute WASI call */
    rc = wasi_snapshot_preview1_backing_fd_readdir(CURRENT_WASI_CONTEXT, fd, buf, buf_len, cookie, nread);

done:
    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_fd_renumber(__wasi_fd_t fd, __wasi_fd_t to) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_fd_renumber(CURRENT_WASI_CONTEXT, fd, to);

done:
    return (uint32_t)rc;
}

/**
 * Move the offset of a file descriptor
 *
 * @param fd
 * @param file_offset The number of bytes to move.
 * @param whence The base from which the offset is relative.
 * @param newoffset_retptr The new offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_seek(__wasi_fd_t fd, __wasi_filedelta_t file_offset, uint32_t whence,
                                        __wasi_size_t newoffset_retptr) {
    __wasi_filesize_t* newoffset = NULL;
    __wasi_errno_t     rc        = 0;

    /* Validate types zero-extended by wasm32 ABI */
    if (unlikely(whence > UINT8_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    /* Check Bounds */
    wasi_serdes_check_bounds(newoffset_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filesize_t);

    /* Convert Offsets to Host Pointers */
    newoffset = &CURRENT_MEMORY_BASE[newoffset_retptr];

    /* Execute WASI syscall */
    rc = wasi_snapshot_preview1_backing_fd_seek(CURRENT_WASI_CONTEXT, fd, file_offset, (__wasi_whence_t)whence,
                                                newoffset);

done:
    return (uint32_t)rc;
}

/**
 * Synchronize the data and metadata of a file to disk
 *
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_sync(__wasi_fd_t fd) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_fd_sync(CURRENT_WASI_CONTEXT, fd);

done:
    return (uint32_t)rc;
}

/**
 * Return the current offset of a file descriptor
 *
 * @param fd
 * @param fileoffset_retptr The current offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_tell(__wasi_fd_t fd, __wasi_size_t fileoffset_retptr) {
    __wasi_filesize_t* fileoffset = NULL;
    __wasi_errno_t     rc         = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(fileoffset_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filesize_t);

    /* Convert Offsets to Host Pointers */
    fileoffset = &CURRENT_MEMORY_BASE[fileoffset_retptr];

    /* Execute WASI Call */
    rc = wasi_snapshot_preview1_backing_fd_tell(CURRENT_WASI_CONTEXT, fd, fileoffset);

done:
    return (uint32_t)rc;
}

/**
 * Write to a file descriptor
 *
 * @param fd
 * @param iovs_baseptr List of scatter/gather vectors from which to retrieve data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param nwritten_retptr
 * @return WASI_ESUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT,
 * WASI_EFBIG, WASI_EINTR, WASI_EIO, WASI_ENOSPC, WASI_EPERM, WASI_EPIPE, or others
 */
uint32_t wasi_snapshot_preview1_fd_write(__wasi_fd_t fd, __wasi_size_t iovs_baseptr, __wasi_size_t iovs_len,
                                         __wasi_size_t nwritten_retptr) {
    __wasi_ciovec_t* iovs     = NULL;
    __wasi_size_t*   nwritten = NULL;
    __wasi_errno_t   rc       = 0;

    /* Check bounds */
    wasi_serdes_check_array_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_ciovec_t, iovs_len);
    wasi_serdes_check_bounds(nwritten_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Convert Offsets to Host Pointers */
    nwritten = &CURRENT_MEMORY_BASE[nwritten_retptr];

    /* Read iovec into memory */
    iovs = calloc(iovs_len, sizeof(__wasi_ciovec_t));
    rc   = wasi_serdes_readv_ciovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Read iovec to target fd */
    rc = wasi_snapshot_preview1_backing_fd_write(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, nwritten);

done:
    return (uint32_t)rc;
}

/**
 * Create a directory
 *
 * @param fd
 * @param path_baseptr
 * @param path_len
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EBADF, WASI_EDQUOT, WASI_EEXIST,
 * WASI_EFAULT, WASI_EINVAL, WASI_ELOOP, WASI_EMLINK, WASI_ENAMETOOLONG,
 * WASI_ENOENT, WASI_ENOMEM, WASI_ENOSPC, WASI_ENOTDIR, WASI_EPERM, or WASI_EROFS
 */
uint32_t
wasi_snapshot_preview1_path_create_directory(__wasi_fd_t fd, __wasi_size_t path_baseptr, __wasi_size_t path_len) {
    const char*    path = NULL;
    __wasi_errno_t rc   = 0;

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    path = &CURRENT_MEMORY_BASE[path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_create_directory(CURRENT_WASI_CONTEXT, fd, path, path_len);

done:
    return (uint32_t)rc;
}

/**
 * Return the attributes of a file or directory
 *
 * @param fd
 * @param flags Flags determining the method of how the path is resolved.
 * @param path_baseptr The path of the file or directory to inspect.
 * @param filestat_retptr The buffer where the file's attributes are stored.
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EBAD, WASI_EFAUL, WASI_EINVAL, WASI_ELOOP,
 * WASI_ENAMETOOLON, WASI_ENOENT, WASI_ENOENT, WASI_ENOMEM, WASI_ENOTDI, or WASI_EOVERFLOW
 */
uint32_t
wasi_snapshot_preview1_path_filestat_get(__wasi_fd_t fd, __wasi_lookupflags_t flags, __wasi_size_t path_baseptr,
                                         __wasi_size_t path_len, __wasi_size_t filestat_retptr) {
    const char*        path     = NULL;
    __wasi_errno_t     rc       = 0;
    __wasi_filestat_t* filestat = NULL;

    wasi_serdes_check_bounds(filestat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filestat_t);
    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    filestat = &CURRENT_MEMORY_BASE[filestat_retptr];
    path     = &CURRENT_MEMORY_BASE[path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_filestat_get(CURRENT_WASI_CONTEXT, fd, flags, path, path_len, filestat);

done:
    return (uint32_t)rc;
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
uint32_t
wasi_snapshot_preview1_path_filestat_set_times(__wasi_fd_t fd, __wasi_lookupflags_t flags, __wasi_size_t path_baseptr,
                                               __wasi_size_t path_len, __wasi_timestamp_t atim, __wasi_timestamp_t mtim,
                                               uint32_t fst_flags) {
    __wasi_errno_t rc   = 0;
    const char*    path = NULL;

    if (unlikely(fst_flags > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    path = &CURRENT_MEMORY_BASE[path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_filestat_set_times(CURRENT_WASI_CONTEXT, fd, flags, path, path_len, atim,
                                                                mtim, (__wasi_fstflags_t)fst_flags);

done:
    return (uint32_t)rc;
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
uint32_t
wasi_snapshot_preview1_path_link(__wasi_fd_t old_fd, __wasi_lookupflags_t old_flags, __wasi_size_t old_path_baseptr,
                                 __wasi_size_t old_path_len, __wasi_fd_t new_fd, __wasi_size_t new_path_baseptr,
                                 __wasi_size_t new_path_len) {
    const char*    old_path = NULL;
    const char*    new_path = NULL;
    __wasi_errno_t rc       = 0;

    wasi_serdes_check_bounds(old_path_baseptr, CURRENT_MEMORY_SIZE, old_path_len);
    wasi_serdes_check_bounds(new_path_baseptr, CURRENT_MEMORY_SIZE, new_path_len);

    old_path = (const char*)&CURRENT_MEMORY_BASE[old_path_baseptr];
    new_path = (const char*)&CURRENT_MEMORY_BASE[new_path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_link(CURRENT_WASI_CONTEXT, old_fd, old_flags, old_path, old_path_len,
                                                  new_fd, new_path, new_path_len);

done:
    return (uint32_t)rc;
}

/**
 * Open a file or directory
 * The returned file descriptor is not guaranteed to be the lowest-numbered
 * file descriptor not currently open; it is randomized to prevent
 * applications from depending on making assumptions about indexes, since this
 * is error-prone in multi-threaded contexts. The returned file descriptor is
 * guaranteed to be less than 2**31.
 *
 * @param dirfd
 * @param lookupflags
 * @param path_baseptr
 * @param path_len
 * @param oflags
 * @param fs_rights_base
 * @param fs_rights_inheriting
 * @param fdflags
 * @param fd_retptr The file descriptor of the file that has been opened.
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_path_open(__wasi_fd_t dirfd, __wasi_lookupflags_t lookupflags, __wasi_size_t path_baseptr,
                                 __wasi_size_t path_len, uint32_t oflags, __wasi_rights_t fs_rights_base,
                                 __wasi_rights_t fs_rights_inheriting, uint32_t fdflags, __wasi_size_t fd_retptr) {
    __wasi_fd_t*   fd   = NULL;
    const char*    path = NULL;
    __wasi_errno_t rc   = 0;

    if (unlikely(oflags > UINT16_MAX || fdflags > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    wasi_serdes_check_bounds(fd_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_fd_t);
    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    fd   = &CURRENT_MEMORY_BASE[fd_retptr];
    path = &CURRENT_MEMORY_BASE[path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_open(CURRENT_WASI_CONTEXT, dirfd, lookupflags, path, path_len,
                                                  (__wasi_oflags_t)oflags, fs_rights_base, fs_rights_inheriting,
                                                  (__wasi_fdflags_t)fdflags, fd);

done:
    return (uint32_t)rc;
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
uint32_t
wasi_snapshot_preview1_path_readlink(__wasi_fd_t fd, __wasi_size_t path_baseptr, __wasi_size_t path_len,
                                     __wasi_size_t buf_baseretptr, __wasi_size_t buf_len, __wasi_size_t nread_retptr) {
    uint8_t*       buf   = NULL;
    const char*    path  = NULL;
    __wasi_size_t* nread = NULL;
    __wasi_errno_t rc    = 0;

    wasi_serdes_check_bounds(buf_baseretptr, CURRENT_MEMORY_SIZE, buf_len);
    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);
    wasi_serdes_check_bounds(nread_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    buf   = &CURRENT_MEMORY_BASE[buf_baseretptr];
    path  = &CURRENT_MEMORY_BASE[path_baseptr];
    nread = &CURRENT_MEMORY_BASE[nread_retptr];

    rc = wasi_snapshot_preview1_backing_path_readlink(CURRENT_WASI_CONTEXT, fd, path, path_len, buf, buf_len, nread);

done:
    return (uint32_t)rc;
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
uint32_t
wasi_snapshot_preview1_path_remove_directory(__wasi_fd_t fd, __wasi_size_t path_baseptr, __wasi_size_t path_len) {
    const char*    path = NULL;
    __wasi_errno_t rc   = 0;

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    path = &CURRENT_MEMORY_BASE[path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_remove_directory(CURRENT_WASI_CONTEXT, fd, path, path_len);

    return (uint32_t)rc;
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
uint32_t
wasi_snapshot_preview1_path_rename(__wasi_fd_t fd, __wasi_size_t old_path_baseptr, __wasi_size_t old_path_len,
                                   __wasi_fd_t new_fd, __wasi_size_t new_path_baseptr, __wasi_size_t new_path_len) {
    const char*    new_path = NULL;
    const char*    old_path = NULL;
    __wasi_errno_t rc       = 0;

    wasi_serdes_check_bounds(new_path_baseptr, CURRENT_MEMORY_SIZE, new_path_len);
    wasi_serdes_check_bounds(old_path_baseptr, CURRENT_MEMORY_SIZE, old_path_len);

    new_path = &CURRENT_MEMORY_BASE[new_path_baseptr];
    old_path = &CURRENT_MEMORY_BASE[old_path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_rename(CURRENT_WASI_CONTEXT, fd, old_path, old_path_len, new_fd, new_path,
                                                    new_path_len);

    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_path_symlink(__wasi_size_t old_path_baseptr, __wasi_size_t old_path_len, __wasi_fd_t fd,
                                             __wasi_size_t new_path_baseptr, __wasi_size_t new_path_len) {
    const char*    new_path = NULL;
    const char*    old_path = NULL;
    __wasi_errno_t rc       = 0;

    wasi_serdes_check_bounds(new_path_baseptr, CURRENT_MEMORY_SIZE, new_path_len);
    wasi_serdes_check_bounds(old_path_baseptr, CURRENT_MEMORY_SIZE, old_path_len);

    new_path = &CURRENT_MEMORY_BASE[new_path_baseptr];
    old_path = &CURRENT_MEMORY_BASE[old_path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_symlink(CURRENT_WASI_CONTEXT, old_path, old_path_len, fd, new_path,
                                                     new_path_len);

    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_path_unlink_file(__wasi_fd_t fd, __wasi_size_t path_baseptr, __wasi_size_t path_len) {
    const char*    path = NULL;
    __wasi_errno_t rc   = 0;

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    path = &CURRENT_MEMORY_BASE[path_baseptr];

    rc = wasi_snapshot_preview1_backing_path_unlink_file(CURRENT_WASI_CONTEXT, fd, path, path_len);

    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_poll_oneoff(__wasi_size_t in_baseptr, __wasi_size_t out_baseptr,
                                            __wasi_size_t nsubscriptions, __wasi_size_t nevents_retptr) {
    const __wasi_subscription_t* in      = NULL;
    __wasi_size_t*               nevents = NULL;
    __wasi_event_t*              out     = NULL;
    __wasi_errno_t               rc      = 0;

    wasi_serdes_check_array_bounds(in_baseptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_subscription_t, nsubscriptions);
    wasi_serdes_check_bounds(nevents_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);
    wasi_serdes_check_array_bounds(out_baseptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_event_t, nsubscriptions);

    in      = &CURRENT_MEMORY_BASE[in_baseptr];
    nevents = &CURRENT_MEMORY_BASE[nevents_retptr];
    out     = &CURRENT_MEMORY_BASE[out_baseptr];

    /* Call WASI syscall */
    rc = wasi_snapshot_preview1_backing_poll_oneoff(CURRENT_WASI_CONTEXT, in, out, nsubscriptions, nevents);

done:
    return (uint32_t)rc;
}

/**
 * Terminate the process normally. An exit code of 0 indicates successful
 * termination of the program. The meanings of other values is dependent on
 * the environment.
 *
 * @param exitcode
 */
__attribute__((noreturn)) void wasi_snapshot_preview1_proc_exit(__wasi_exitcode_t exitcode) {
    wasi_snapshot_preview1_backing_proc_exit(CURRENT_WASI_CONTEXT, exitcode);
    awsm_assert(0);
}

/**
 * Send a signal to the process of the calling thread.
 *
 * @param sig The signal condition to trigger.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_proc_raise(uint32_t sig) {
    __wasi_errno_t rc = 0;

    if (sig > UINT8_MAX) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_proc_raise(CURRENT_WASI_CONTEXT, (__wasi_signal_t)sig);

done:
    return (uint32_t)rc;
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
uint32_t wasi_snapshot_preview1_random_get(__wasi_size_t buf_baseretptr, __wasi_size_t buf_len) {
    uint8_t*       buf = NULL;
    __wasi_errno_t rc  = 0;

    /* Check Bounds */
    wasi_serdes_check_bounds(buf_baseretptr, CURRENT_MEMORY_SIZE, buf_len);

    buf = &CURRENT_MEMORY_BASE[buf_baseretptr];

    /* Write random bytes directly to linear memory */
    rc = wasi_snapshot_preview1_backing_random_get(CURRENT_WASI_CONTEXT, buf, buf_len);

done:
    return (uint32_t)rc;
}

/**
 * Temporarily yield execution of the calling thread similar to `sched_yield` in POSIX.
 * This implementation ignores client calls and silently returns RC 0
 *
 * @return WASI_ESUCCESS
 */
uint32_t wasi_snapshot_preview1_sched_yield(void) {
    __wasi_errno_t rc = 0;

    rc = wasi_snapshot_preview1_backing_sched_yield(CURRENT_WASI_CONTEXT);

done:
    return (uint32_t)rc;
}

/**
 * Receive a message from a socket.
 * Note: This WASI syscall is in flux pending a decision on whether WASI
 * should only support fd_read and fd_write
 * See: https://github.com/WebAssembly/WASI/issues/4
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
uint32_t wasi_snapshot_preview1_sock_recv(__wasi_fd_t fd, __wasi_size_t ri_data_baseretptr, __wasi_size_t ri_data_len,
                                          uint32_t ri_flags, __wasi_size_t ri_data_nbytes_retptr,
                                          __wasi_size_t message_nbytes_retptr) {
    __wasi_errno_t rc = 0;

    if (unlikely(ri_flags > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    rc = wasi_unsupported_syscall(__func__);

done:
    return (uint32_t)rc;
}

/**
 * Send a message on a socket.
 * Note: This WASI syscall is in flux pending a decision on whether WASI
 * should only support fd_read and fd_write
 * See: https://github.com/WebAssembly/WASI/issues/4
 * Note: This is similar to `send` in POSIX, though it also supports writing
 * the data from multiple buffers in the manner of `writev`.
 *
 * @param fd
 * @param si_data_baseptr List of scatter/gather vectors to which to retrieve data
 * @param si_data_len The length of the array pointed to by `si_data`.
 * @param si_flags Message flags.
 * @param nbytes_retptr Number of bytes transmitted.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_sock_send(__wasi_fd_t fd, __wasi_size_t si_data_baseptr, __wasi_size_t si_data_len,
                                          uint32_t si_flags, __wasi_size_t retptr0) {
    __wasi_errno_t rc = 0;

    if (unlikely(si_flags > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    rc = wasi_unsupported_syscall(__func__);

done:
    return (uint32_t)rc;
}

/**
 * Shut down socket send and receive channels.
 * Note: This WASI syscall is in flux pending a decision on whether WASI
 * should only support fd_read and fd_write
 * See: https://github.com/WebAssembly/WASI/issues/4
 *
 * @param fd
 * @param how Which channels on the socket to shut down.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_sock_shutdown(__wasi_fd_t fd, uint32_t how) {
    __wasi_errno_t rc = 0;

    if (unlikely(how > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    rc = wasi_unsupported_syscall(__func__);

done:
    return (uint32_t)rc;
}
