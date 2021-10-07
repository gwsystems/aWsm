#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wasi_impl.h"
#include "wasi_serdes.h"

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_args_get(%u,%u)\n", argv_retptr, argv_buf_retptr);
#endif

    // TODO: const pointer?
    const __wasi_size_t argc          = wasi_context_get_argc(CURRENT_WASI_CONTEXT);
    const __wasi_size_t argv_buf_size = wasi_context_get_argv_buf_size(CURRENT_WASI_CONTEXT);
    assert(argv_buf_size < 255);
    char** wasi_context_argv = wasi_context_get_argv(CURRENT_WASI_CONTEXT);
    assert(wasi_context_argv != NULL);

    /* Check Bounds */
    wasi_serdes_check_bounds(argv_retptr, CURRENT_MEMORY_SIZE, sizeof(__wasi_size_t) * argc);
    wasi_serdes_check_bounds(argv_buf_retptr, CURRENT_MEMORY_SIZE, argv_buf_size);

    /*
     * Write results to temporary buffers argv and argv_buf
     * TODO: Should we write argv_buf directly to memory to make more efficient?
     */
    char*         argv[argc];
    char          argv_buf[argv_buf_size];
    __wasi_size_t argv_offset[argc];
    __wasi_size_t rc = wasi_snapshot_preview1_backing_args_get(CURRENT_WASI_CONTEXT, argv, argv_buf);

    if (rc != __WASI_ERRNO_SUCCESS) {
        fprintf(stderr, "not good!\n");
        goto done;
    }

    /* Adjust argv to use offsets, not host pointers */
    for (size_t i = 0; i < argc; i++) {
        argv_offset[i] = argv_buf_retptr + (wasi_context_argv[i] - wasi_context_argv[0]);
    }

    /* Write argv and argv_buf to linear memory */
    memcpy(&CURRENT_MEMORY_BASE[argv_retptr], argv_offset, argc * sizeof(__wasi_size_t));
    memcpy(&CURRENT_MEMORY_BASE[argv_buf_retptr], argv_buf, argv_buf_size);

done:
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_args_sizes_get(%u,%u)\n", argc_retptr, argv_buf_len_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(argc_retptr, CURRENT_MEMORY_SIZE, sizeof(uint32_t));
    wasi_serdes_check_bounds(argv_buf_len_retptr, CURRENT_MEMORY_SIZE, sizeof(uint32_t));

    /* Get sizes */
    __wasi_size_t  argc;
    __wasi_size_t  argv_buf_size;
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_args_sizes_get(CURRENT_WASI_CONTEXT, &argc, &argv_buf_size);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write to linear memory */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, argc_retptr, argc);
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, argv_buf_len_retptr, argv_buf_size);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_clock_res_get(%u,%u)\n", id, res_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(res_retptr, CURRENT_MEMORY_SIZE, sizeof(__wasi_timestamp_t));

    /* Read resolution into buffer */
    __wasi_timestamp_t resolution;
    __wasi_errno_t     rc = wasi_snapshot_preview1_backing_clock_res_get(CURRENT_WASI_CONTEXT, id, &resolution);

    /* write buffer to linear memory */
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    wasi_serdes_write_timestamp_t(CURRENT_MEMORY_BASE, res_retptr, resolution);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_clock_time_get(%u,%lu,%u)\n", clock_id, precision, time_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(time_retptr, CURRENT_MEMORY_SIZE, sizeof(__wasi_timestamp_t));

    /* Make WASI call */
    __wasi_timestamp_t time;
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_clock_time_get(CURRENT_WASI_CONTEXT, clock_id, precision, &time);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write to Linear Memory */
    wasi_serdes_write_timestamp_t(CURRENT_MEMORY_BASE, time_retptr, time);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_environ_get(%u,%u)\n", environ_retptr, environ_buf_retptr);
#endif

    const __wasi_size_t envc         = wasi_context_get_envc(CURRENT_WASI_CONTEXT);
    const __wasi_size_t env_buf_size = wasi_context_get_env_buf_size(CURRENT_WASI_CONTEXT);

    /* Check Bounds */
    wasi_serdes_check_bounds(environ_retptr, CURRENT_MEMORY_SIZE, envc * WASI_SERDES_SIZE_size_t);
    wasi_serdes_check_bounds(environ_buf_retptr, CURRENT_MEMORY_SIZE, env_buf_size);

    /* Write environment to temporary buffer and environment_buf directly to linear memory */
    char*          environment[envc];
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_environ_get(CURRENT_WASI_CONTEXT, environment,
                                                                   (char*)&CURRENT_MEMORY_BASE[environ_buf_retptr]);

    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Convert environment from pointers to offsets and write to linear memory */
    for (size_t i = 0; i < envc; i++) {
        __wasi_size_t offset = environ_buf_retptr + (environment[i] - environment[0]);

        wasi_serdes_write_uint32_t(CURRENT_MEMORY_BASE, environ_retptr + (i * WASI_SERDES_SIZE_uint32_t), offset);
    }

done:
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_environ_sizes_get(%u,%u)\n", environc_retptr, environv_buf_len_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(environc_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);
    wasi_serdes_check_bounds(environv_buf_len_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Make WASI Call */
    __wasi_size_t  envc;
    __wasi_size_t  env_buf_size;
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_environ_sizes_get(CURRENT_WASI_CONTEXT, &envc, &env_buf_size);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write results to linear memory */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, environc_retptr, envc);
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, environv_buf_len_retptr, env_buf_size);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_advise(%u,%lu,%lu,%u)\n", fd, offset, len, advice);
#endif

    __wasi_errno_t rc;

    if (unlikely(advice > UINT8_MAX))
        goto err_advice_overflow;

    rc = wasi_snapshot_preview1_backing_fd_advise(CURRENT_WASI_CONTEXT, fd, offset, len, (__wasi_advice_t)advice);

done:
    return (uint32_t)rc;
err_advice_overflow:
    rc = __WASI_ERRNO_INVAL;
    goto done;
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_allocate(%u,%lu,%lu)\n", fd, offset, len);
#endif

    return (uint32_t)wasi_snapshot_preview1_backing_fd_allocate(CURRENT_WASI_CONTEXT, fd, offset, len);
};

/**
 * Close a file descriptor.
 *
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_close(__wasi_fd_t fd) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_close(%u)\n", fd);
#endif

    return (uint32_t)wasi_snapshot_preview1_backing_fd_close(CURRENT_WASI_CONTEXT, fd);
}

/**
 * Synchronize the data of a file to disk.
 *
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_datasync(__wasi_fd_t fd) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_datasync(%u)\n", fd);
#endif

    return (uint32_t)wasi_snapshot_preview1_backing_fd_datasync(CURRENT_WASI_CONTEXT, fd);
}

/**
 * Get the attributes of a file descriptor.
 *
 * @param fd
 * @param fdstat_retptr the offset where the resulting wasi_fdstat structure should be written
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_fdstat_get(__wasi_fd_t fd, __wasi_size_t fdstat_retptr) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_fdstat_get(%u,%u)\n", fd, fdstat_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(fdstat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_fdstat_t);

    /* Make WASI Call */
    __wasi_fdstat_t stats;
    __wasi_errno_t  rc = wasi_snapshot_preview1_backing_fd_fdstat_get(CURRENT_WASI_CONTEXT, fd, &stats);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write result to linear memory */
    wasi_serdes_write_fdstat_t(CURRENT_MEMORY_BASE, fdstat_retptr, &stats);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_fdstat_set_flags(%u,%u)\n", fd, fdflags);
#endif

    __wasi_errno_t rc;

    /* fdflags should be a zero-extended uint16_t */
    if (fdflags > UINT16_MAX)
        goto err_fdflags_overflow;

    rc = wasi_snapshot_preview1_backing_fdstat_set_flags(CURRENT_WASI_CONTEXT, fd, (__wasi_fdflags_t)fdflags);

done:
    return (uint32_t)rc;
err_fdflags_overflow:
    rc = __WASI_ERRNO_INVAL;
    goto done;
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_fdstat_set_rights(%u,%lu,%lu)\n", fd, fs_rights_base,
            fs_rights_inheriting);
#endif
    return (uint32_t)wasi_snapshot_preview1_backing_fdstat_set_rights(CURRENT_WASI_CONTEXT, fd, fs_rights_base, fs_rights_inheriting);
}

/**
 * Return the attributes of an open file.
 *
 * @param fd
 * @param filestat_retptr The buffer where we should store the file's attributes
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_filestat_get(__wasi_fd_t fd, __wasi_size_t filestat_retptr) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_filestat_get(%u,%u)\n", fd, filestat_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(filestat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filestat_t);

    /* Make WASI Call */
    __wasi_filestat_t stats;
    __wasi_errno_t    rc = wasi_snapshot_preview1_backing_fd_filestat_get(CURRENT_WASI_CONTEXT, fd, &stats);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Copy result into linear memory */
    wasi_serdes_write_filestat_t(CURRENT_MEMORY_BASE, filestat_retptr, &stats);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_filestat_set_size(%u,%lu)\n", fd, size);
#endif

    return (uint32_t)wasi_snapshot_preview1_backing_fd_filestat_set_size(CURRENT_WASI_CONTEXT, fd, size);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_filestat_set_times(%u,%lu,%lu,%u)\n", fd, atim, mtim, fst_flags);
#endif

    __wasi_errno_t rc;
    if (unlikely(fst_flags > UINT16_MAX))
        goto err_fst_flags_overflow;

    rc = wasi_snapshot_preview1_backing_filestat_set_times(CURRENT_WASI_CONTEXT, fd, atim, mtim, (__wasi_fstflags_t)fst_flags);

done:
    return (uint32_t)rc;
err_fst_flags_overflow:
    rc = __WASI_ERRNO_INVAL;
    goto done;
}

/**
 * Read from a file descriptor without updating the descriptor's offset
 *
 * @param fd
 * @param iovs_baseptr List of scatter/gather vectors in which to store data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param offset The offset within the file at which to read.
 * @param nbytes_retptr The number of bytes read.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_pread(__wasi_fd_t fd, __wasi_size_t iovs_baseptr, __wasi_size_t iovs_len,
                                         __wasi_filesize_t offset, __wasi_size_t nbytes_retptr) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_pread(%u,%u,%u,%lu,%u)\n", fd, iovs_baseptr, iovs_len, offset,
            nbytes_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, iovs_len * WASI_SERDES_SIZE_iovec_t);
    wasi_serdes_check_bounds(nbytes_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Read iovec from linear memory */
    __wasi_iovec_t iovs[iovs_len];
    __wasi_errno_t rc = wasi_serdes_readv_iovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs,
                                                  iovs_len);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Execute WASI call */
    __wasi_size_t nread;
    rc = wasi_snapshot_preview1_backing_fd_pread(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, offset, &nread);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write Result */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, nbytes_retptr, nread);

done:
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_prestat_get(%u,%u)\n", fd, prestat_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(prestat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_prestat_t);

    /* Execute WASI call */
    __wasi_prestat_t prestat;
    __wasi_errno_t   rc = wasi_snapshot_preview1_backing_fd_prestat_get(CURRENT_WASI_CONTEXT, fd, &prestat);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write result to linear memory */
    wasi_serdes_write_prestat_t(CURRENT_MEMORY_BASE, prestat_retptr, &prestat);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_prestat_dir_name(%u,%u,%u)\n", fd, path_retptr, path_len);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(path_retptr, CURRENT_MEMORY_SIZE, path_len);

    /* Execute WASI call, writing results directly to linear memory */
    return (uint32_t)wasi_snapshot_preview1_backing_fd_prestat_dir_name(CURRENT_WASI_CONTEXT, fd, (char*)&CURRENT_MEMORY_BASE[path_retptr],
                                                path_len);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_pwrite(%u,%u,%u,%lu,%u)\n", fd, iovs_baseptr, iovs_len, offset,
            nwritten_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, iovs_len * WASI_SERDES_SIZE_ciovec_t);
    wasi_serdes_check_bounds(nwritten_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Copy iovs into memory */
    __wasi_ciovec_t iovs[iovs_len];
    __wasi_errno_t  rc = wasi_serdes_readv_ciovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs,
                                                   iovs_len);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Execute WASI call */
    __wasi_size_t nwritten;
    rc = wasi_snapshot_preview1_backing_fd_pwrite(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, offset, &nwritten);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write result to linear memory */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, nwritten_retptr, nwritten);

done:
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_read(%u,%u,%u,%u)\n", fd, iovs_baseptr, iovs_len, nread_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, iovs_len * WASI_SERDES_SIZE_iovec_t);
    wasi_serdes_check_bounds(nread_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Copy iovs into memory */
    __wasi_iovec_t iovs[iovs_len];
    __wasi_errno_t rc = wasi_serdes_readv_iovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs,
                                                  iovs_len);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Execute WASI call */
    __wasi_size_t nread;
    rc = wasi_snapshot_preview1_backing_fd_read(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, &nread);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write result to linear memory */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, nread_retptr, nread);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_readdir(%u,%u,%u,%lu,%u)\n", fd, buf_baseptr, buf_len, cookie,
            nread_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(buf_baseptr, CURRENT_MEMORY_SIZE, buf_len);
    wasi_serdes_check_bounds(nread_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Execute WASI call */
    __wasi_size_t  nread;
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_fd_readdir(CURRENT_WASI_CONTEXT, fd, (uint8_t*)&CURRENT_MEMORY_BASE[buf_baseptr],
                                          buf_len, cookie, &nread);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write result to linear memory */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, nread_retptr, nread);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_renumber(%u,%u)\n", fd, to);
#endif

    return (uint32_t)wasi_snapshot_preview1_backing_fd_renumber(CURRENT_WASI_CONTEXT, fd, to);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_seek(%u,%ld,%u,%u)\n", fd, file_offset, whence, newoffset_retptr);
#endif

    __wasi_errno_t rc;

    /* Validate types zero-extended by wasm32 ABI */
    if (unlikely(whence > UINT8_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }

    /* Check Bounds */
    wasi_serdes_check_bounds(newoffset_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filesize_t);

    /* Execute WASI syscall */
    __wasi_filesize_t newoffset;
    rc = wasi_snapshot_preview1_backing_fd_seek(CURRENT_WASI_CONTEXT, fd, file_offset, (__wasi_whence_t)whence, &newoffset);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write results to linear memory */
    wasi_serdes_write_filesize_t(CURRENT_MEMORY_BASE, newoffset_retptr, newoffset);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_sync(%u)\n", fd);
#endif

    return (uint32_t)wasi_snapshot_preview1_backing_fd_sync(CURRENT_WASI_CONTEXT, fd);
}

/**
 * Return the current offset of a file descriptor
 *
 * @param fd
 * @param fileoffset_retptr The current offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_tell(__wasi_fd_t fd, __wasi_size_t fileoffset_retptr) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_tell(%u,%u)\n", fd, fileoffset_retptr);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(fileoffset_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filesize_t);

    /* Execute WASI Call */
    __wasi_filesize_t fileoffset;
    __wasi_errno_t    rc = wasi_snapshot_preview1_backing_fd_tell(CURRENT_WASI_CONTEXT, fd, &fileoffset);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write results to linear memory */
    wasi_serdes_write_filesize_t(CURRENT_MEMORY_BASE, fileoffset_retptr, fileoffset);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_fd_write(%u,%u,%u,%u)\n", fd, iovs_baseptr, iovs_len, nwritten_retptr);
#endif

    /* Check bounds */
    wasi_serdes_check_bounds(iovs_baseptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_ciovec_t * iovs_len);
    wasi_serdes_check_bounds(nwritten_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    /* Read iovec into memory */
    __wasi_ciovec_t iovs[iovs_len];
    __wasi_errno_t  rc = wasi_serdes_readv_ciovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseptr, iovs,
                                                   iovs_len);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Read iovec to target fd */
    __wasi_size_t nwritten;
    rc = wasi_snapshot_preview1_backing_fd_write(CURRENT_WASI_CONTEXT, fd, iovs, iovs_len, &nwritten);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write nwritten back into linear memory */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, nwritten_retptr, nwritten);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_create_directory(%u,%u,%u)\n", fd, path_baseptr, path_len);
#endif

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_create_directory(CURRENT_WASI_CONTEXT, fd,
                                                  (const char*)&CURRENT_MEMORY_BASE[path_baseptr], path_len);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_filestat_get(%u,%u,%u,%u,%u)\n", fd, flags, path_baseptr, path_len,
            filestat_retptr);
#endif

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);
    wasi_serdes_check_bounds(filestat_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_filestat_t);

    __wasi_filestat_t stats;
    __wasi_errno_t    rc = wasi_snapshot_preview1_backing_path_filestat_get(CURRENT_WASI_CONTEXT, fd, flags,
                                                 (const char*)&CURRENT_MEMORY_BASE[path_baseptr], path_len, &stats);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    wasi_serdes_write_filestat_t(CURRENT_MEMORY_BASE, filestat_retptr, &stats);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_filestat_set_times(%u,%u,%u,%u,%lu,%lu,%u)\n", fd, flags, path_baseptr,
            path_len, atim, mtim, fst_flags);
#endif

    __wasi_errno_t rc;


    if (unlikely(fst_flags > UINT16_MAX))
        goto err_fst_flags_overflow;

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    return wasi_snapshot_preview1_backing_path_filestat_set_times(CURRENT_WASI_CONTEXT, fd, flags,
                                          (const char*)&CURRENT_MEMORY_BASE[path_baseptr], path_len, atim, mtim,
                                          (__wasi_fstflags_t)fst_flags);

done:
    return (uint32_t)rc;
err_fst_flags_overflow:
    rc = __WASI_ERRNO_INVAL;
    goto done;
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_link(%u,%u,%u,%u,%u,%u,%u)\n", old_fd, old_flags, old_path_baseptr,
            old_path_len, new_fd, new_path_baseptr, new_path_len);
#endif

    wasi_serdes_check_bounds(old_path_baseptr, CURRENT_MEMORY_SIZE, old_path_len);
    wasi_serdes_check_bounds(new_path_baseptr, CURRENT_MEMORY_SIZE, new_path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_link(CURRENT_WASI_CONTEXT, old_fd, old_flags,
                                      (const char*)&CURRENT_MEMORY_BASE[old_path_baseptr], old_path_len, new_fd,
                                      (const char*)&CURRENT_MEMORY_BASE[new_path_baseptr], new_path_len);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_open(%u,%u,%u,%u,%u,%lu,%lu,%u,%u)\n", dirfd, lookupflags,
            path_baseptr, path_len, oflags, fs_rights_base, fs_rights_inheriting, fdflags, fd_retptr);
#endif

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);
    wasi_serdes_check_bounds(fd_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_fd_t);

    if (unlikely(oflags > UINT16_MAX))
        goto err_oflags_overflow;
    if (unlikely(fdflags > UINT16_MAX))
        goto err_fdflags_overflow;

    __wasi_fd_t    fd;
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_path_open(CURRENT_WASI_CONTEXT, dirfd, lookupflags,
                                         (const char*)&CURRENT_MEMORY_BASE[path_baseptr], path_len,
                                         (__wasi_oflags_t)oflags, fs_rights_base, fs_rights_inheriting,
                                         (__wasi_fdflags_t)fdflags, &fd);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, fd_retptr, fd);

done:
    return (uint32_t)rc;
err_oflags_overflow:
err_fdflags_overflow:
    rc = __WASI_ERRNO_INVAL;
    goto done;
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_open(%u,%u,%u,%u,%u,%u)\n", fd, path_baseptr, path_len, buf_baseretptr,
            buf_len, nread_retptr);
#endif

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);
    wasi_serdes_check_bounds(buf_baseretptr, CURRENT_MEMORY_SIZE, buf_len);
    wasi_serdes_check_bounds(nread_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    __wasi_size_t  nread;
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_path_readlink(CURRENT_WASI_CONTEXT, fd, (const char*)&CURRENT_MEMORY_BASE[path_baseptr],
                                             path_len, &CURRENT_MEMORY_BASE[buf_baseretptr], buf_len, &nread);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, nread_retptr, nread);

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_remove_directory(%u,%u,%u)\n", fd, path_baseptr, path_len);
#endif

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_remove_directory(CURRENT_WASI_CONTEXT, fd,
                                                  (const char*)&CURRENT_MEMORY_BASE[path_baseptr], path_len);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_remove_directory(%u,%u,%u,%u,%u,%u)\n", fd, old_path_baseptr,
            old_path_len, new_fd, new_path_baseptr, new_path_len);
#endif

    wasi_serdes_check_bounds(old_path_baseptr, CURRENT_MEMORY_SIZE, old_path_len);
    wasi_serdes_check_bounds(new_path_baseptr, CURRENT_MEMORY_SIZE, new_path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_rename(CURRENT_WASI_CONTEXT, fd, (const char*)&CURRENT_MEMORY_BASE[old_path_baseptr],
                                        old_path_len, new_fd, (const char*)&CURRENT_MEMORY_BASE[new_path_baseptr],
                                        new_path_len);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_symlink(%u,%u,%u,%u,%u)\n", old_path_baseptr, old_path_len, fd,
            new_path_baseptr, new_path_len);
#endif

    wasi_serdes_check_bounds(old_path_baseptr, CURRENT_MEMORY_SIZE, old_path_len);
    wasi_serdes_check_bounds(new_path_baseptr, CURRENT_MEMORY_SIZE, new_path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_symlink(CURRENT_WASI_CONTEXT, (const char*)&CURRENT_MEMORY_BASE[old_path_baseptr],
                                         old_path_len, fd, &CURRENT_MEMORY_BASE[new_path_baseptr], new_path_len);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_path_unlink_file(%u,%u,%u)\n", fd, path_baseptr, path_len);
#endif

    wasi_serdes_check_bounds(path_baseptr, CURRENT_MEMORY_SIZE, path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_unlink_file(CURRENT_WASI_CONTEXT, fd, (const char*)&CURRENT_MEMORY_BASE[path_baseptr],
                                             path_len);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_poll_oneoff(%u,%u,%u,%u)\n", in_baseptr, out_baseptr, nsubscriptions,
            nevents_retptr);
#endif

    wasi_serdes_check_bounds(in_baseptr, CURRENT_MEMORY_SIZE, nsubscriptions * WASI_SERDES_SIZE_subscription_t);
    wasi_serdes_check_bounds(out_baseptr, CURRENT_MEMORY_SIZE, nsubscriptions * WASI_SERDES_SIZE_event_t);
    wasi_serdes_check_bounds(nevents_retptr, CURRENT_MEMORY_SIZE, WASI_SERDES_SIZE_size_t);

    __wasi_subscription_t in[nsubscriptions];
    __wasi_event_t        out[nsubscriptions];

    /* Read in from linear memory */
    for (__wasi_size_t i = 0; i < nsubscriptions; i++) {
        wasi_serdes_read_subscription_t(CURRENT_MEMORY_BASE, in_baseptr, &in[i]);
        in_baseptr += WASI_SERDES_SIZE_subscription_t;
    }

    /* Call WASI syscall */
    __wasi_size_t  nevents;
    __wasi_errno_t rc = wasi_snapshot_preview1_backing_poll_oneoff(CURRENT_WASI_CONTEXT, in, out, nsubscriptions, &nevents);
    if (rc != __WASI_ERRNO_SUCCESS)
        goto done;

    /* Write results to linear memory */
    wasi_serdes_write_size_t(CURRENT_MEMORY_BASE, nevents_retptr, nevents);

    for (__wasi_size_t i = 0; i < nsubscriptions; i++) {
        wasi_serdes_write_event_t(CURRENT_MEMORY_BASE, out_baseptr, &out[i]);
        out_baseptr += WASI_SERDES_SIZE_event_t;
    }

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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_proc_exit(%u)\n", exitcode);
#endif

    wasi_snapshot_preview1_backing_proc_exit(CURRENT_WASI_CONTEXT, exitcode);
    assert(0);
}

/**
 * Send a signal to the process of the calling thread.
 *
 * @param sig The signal condition to trigger.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_proc_raise(uint32_t sig) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_proc_raise(%u)\n", sig);
#endif

    __wasi_errno_t rc;

    if (sig > UINT8_MAX)
        goto err_sig_overflow;

    rc = wasi_snapshot_preview1_backing_proc_raise(CURRENT_WASI_CONTEXT, (__wasi_signal_t)sig);

done:
    return (uint32_t)rc;
err_sig_overflow:
    rc = __WASI_ERRNO_INVAL;
    goto done;
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_random_get(%u,%u)\n", buf_baseretptr, buf_len);
#endif

    /* Check Bounds */
    wasi_serdes_check_bounds(buf_baseretptr, CURRENT_MEMORY_SIZE, buf_len);

    /* Write random bytes directly to linear memory */
    return (uint32_t)wasi_snapshot_preview1_backing_random_get(CURRENT_WASI_CONTEXT, &CURRENT_MEMORY_BASE[buf_baseretptr], buf_len);
}

/**
 * Temporarily yield execution of the calling thread similar to `sched_yield` in POSIX.
 * This implementation ignores client calls and silently returns RC 0
 *
 * @return WASI_ESUCCESS
 */
uint32_t wasi_snapshot_preview1_sched_yield(void) {
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_sched_yield()\n");
#endif

    return (uint32_t)wasi_snapshot_preview1_backing_sched_yield(CURRENT_WASI_CONTEXT);
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_sock_recv(%u,%u,%u,%u,%u,%u)\n", fd, ri_data_baseretptr, ri_data_len,
            ri_flags, ri_data_nbytes_retptr, message_nbytes_retptr);
#endif

    return wasi_unsupported_syscall(__func__);

    /* ri_flags is type __wasi_riflags_t, which is uint16_t and zero extended to an wasm i32 */
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_sock_send(%u,%u,%u,%u,%u)\n", fd, si_data_baseptr, si_data_len, si_flags,
            retptr0);
#endif
    return wasi_unsupported_syscall(__func__);

    /* TODO: si_flags is type __wasi_siflags_t, which is uint16_t and zero extended to an wasm i32 */
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
#ifdef LOG_WASI
    fprintf(stderr, "wasi_snapshot_preview1_sock_shutdown(%u,%u)\n", fd, how);
#endif

    return wasi_unsupported_syscall(__func__);

    /* TODO: how is type __wasi_sdflags_t, which is uint8_t and zero extended to an wasm i32 */
}
