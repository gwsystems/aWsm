#pragma once

/* The functions in this header are responsible for swizzling and unswizzling wasm offsets and narrowing
 * arguments that were automatically zero-extended to 32-bits. The pass host pointers and proper wasm types
 * to a WASI backing.
 */

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
 * @brief Writes argument offsets and buffer into linear memory
 *
 * @param argv_retoffset
 * @param argv_buf_retoffset
 * @return status code
 */
uint32_t wasi_snapshot_preview1_args_get(__wasi_size_t argv_retoffset, __wasi_size_t argv_buf_retoffset) {
    __wasi_size_t rc = 0;

    const __wasi_size_t argc = wasi_context_get_argc(CURRENT_WASI_CONTEXT);
    if (unlikely(argc == 0)) {
        goto done;
    }

    __wasi_size_t*      argv_retptr     = (__wasi_size_t*)get_memory_ptr_for_runtime(argv_retoffset,
                                                                                     WASI_SERDES_SIZE_size_t * argc);
    const __wasi_size_t argv_buf_size   = wasi_context_get_argv_buf_size(CURRENT_WASI_CONTEXT);
    char*               argv_buf_retptr = get_memory_ptr_for_runtime(argv_buf_retoffset, argv_buf_size);

    /* args_get backings return a vector of host pointers. We need a host buffer to store this
     * temporarily before unswizzling and writing to linear memory */
    char** argv_temp = calloc(sizeof(char*), argc);
    if (unlikely(argv_temp == NULL)) {
        goto done;
    }

    /* Writes argv_buf to linear memory and argv vector to our temporary buffer */
    rc = wasi_snapshot_preview1_backing_args_get(CURRENT_WASI_CONTEXT, argv_temp, argv_buf_retptr);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Unswizzle argv */
    for (int i = 0; i < argc; i++) {
        argv_retptr[i] = argv_buf_retoffset + (uint32_t)(argv_temp[i] - argv_temp[0]);
    }

done:
    if (likely(argv_temp != NULL)) {
        free(argv_temp);
        argv_temp = NULL;
    }

    return (uint32_t)rc;
}

/**
 * @brief Used by a WASI module to determine the argument count and size of the requried
 * argument buffer
 *
 * @param argc_retoffset linear memory offset where we should write argc
 * @param argv_buf_len_retoffset linear memory offset where we should write the length of the args buffer
 * @return status code
 */
uint32_t wasi_snapshot_preview1_args_sizes_get(__wasi_size_t argc_retoffset, __wasi_size_t argv_buf_len_retoffset) {
    __wasi_size_t* argc_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(argc_retoffset, WASI_SERDES_SIZE_size_t);
    __wasi_size_t* argv_buf_len_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(argv_buf_len_retoffset,
                                                                                    WASI_SERDES_SIZE_size_t);

    return (uint32_t)wasi_snapshot_preview1_backing_args_sizes_get(CURRENT_WASI_CONTEXT, argc_retptr,
                                                                   argv_buf_len_retptr);
}

/**
 * @brief Return the resolution of a clock
 * Implementations are required to provide a non-zero value for supported clocks. For unsupported clocks,
 * return `errno::inval`.
 *
 * @param id The clock for which to return the resolution.
 * @param resolution_retoffset - The resolution of the clock
 * @return status code
 */
uint32_t wasi_snapshot_preview1_clock_res_get(__wasi_clockid_t id, __wasi_size_t resolution_retoffset) {
    __wasi_timestamp_t* resolution_retptr = (__wasi_timestamp_t*)
      get_memory_ptr_for_runtime(resolution_retoffset, WASI_SERDES_SIZE_timestamp_t);

    return (uint32_t)wasi_snapshot_preview1_backing_clock_res_get(CURRENT_WASI_CONTEXT, id, resolution_retptr);
}

/**
 * Return the time value of a clock
 *
 * @param clock_id The clock for which to return the time.
 * @param precision The maximum lag (exclusive) that the returned time value may have, compared to its actual value.
 * @param time_retoffset  The time value of the clock.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_clock_time_get(__wasi_clockid_t clock_id, __wasi_timestamp_t precision,
                                               __wasi_size_t time_retoffset) {
    __wasi_timestamp_t* time_retptr = (__wasi_timestamp_t*)get_memory_ptr_for_runtime(time_retoffset,
                                                                                      WASI_SERDES_SIZE_timestamp_t);

    return (uint32_t)wasi_snapshot_preview1_backing_clock_time_get(CURRENT_WASI_CONTEXT, clock_id, precision,
                                                                   time_retptr);
}

/**
 * Read environment variable data.
 * The sizes of the buffers should match that returned by `environ_sizes_get`.
 *
 * @param environ_retoffset
 * @param environ_buf_retoffset
 * @return status code
 */
uint32_t wasi_snapshot_preview1_environ_get(__wasi_size_t env_retoffset, __wasi_size_t env_buf_retoffset) {
    __wasi_errno_t rc = 0;

    const __wasi_size_t envc = wasi_context_get_envc(CURRENT_WASI_CONTEXT);
    if (envc == 0) {
        goto done;
    }

    const __wasi_size_t env_buf_size = wasi_context_get_env_buf_size(CURRENT_WASI_CONTEXT);
    awsm_assert(env_buf_size > envc);

    /* wasi_snapshot_preview1_backing_environ_get returns a vector of host pointers. We write
     * these results to environ_temp temporarily before converting to offsets and writing to
     * linear memory. We could technically write this to linear memory and then do a "fix up,"
     * but this would leak host information and constitue a security issue */
    char** env_temp = calloc(sizeof(char*), envc);
    if (unlikely(env_temp == NULL)) {
        goto done;
    }

    __wasi_size_t* env_retptr     = (__wasi_size_t*)get_memory_ptr_for_runtime(env_retoffset,
                                                                               WASI_SERDES_SIZE_size_t * envc);
    char*          env_buf_retptr = get_memory_ptr_for_runtime(env_buf_retoffset, env_buf_size);

    rc = wasi_snapshot_preview1_backing_environ_get(CURRENT_WASI_CONTEXT, env_temp, env_buf_retptr);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    /* Unswizzle env and write to linear memory */
    for (int i = 0; i < envc; i++) {
        env_retptr[i] = env_buf_retoffset + (uint32_t)(env_temp[i] - env_temp[0]);
    }

done:
    if (likely(env_temp != NULL)) {
        free(env_temp);
        env_temp = NULL;
    }

    return (uint32_t)rc;
}

/**
 * Returns the number of environment variable arguments and the size of the environment variable data.
 *
 * @param envc_retoffset - the offset where the resulting number of environment variable arguments should be written
 * @param env_buf_len_retoffset - the offset where the resulting size of the environment variable data should be
 * written
 * @return status code
 */
uint32_t wasi_snapshot_preview1_environ_sizes_get(__wasi_size_t envc_retoffset, __wasi_size_t env_buf_len_retoffset) {
    __wasi_size_t* envc_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(envc_retoffset, WASI_SERDES_SIZE_size_t);
    __wasi_size_t* env_buf_len_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(env_buf_len_retoffset,
                                                                                   WASI_SERDES_SIZE_size_t);

    return (uint32_t)wasi_snapshot_preview1_backing_environ_sizes_get(CURRENT_WASI_CONTEXT, envc_retptr,
                                                                      env_buf_len_retptr);
}

/**
 * Provide file advisory information on a file descriptor.
 * Note: similar to `posix_fadvise` in POSIX
 *
 * @param fd
 * @param file_offset The offset within the file to which the advisory applies.
 * @param len The length of the region to which the advisory applies.
 * @param advice
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_advise(__wasi_fd_t fd, __wasi_filesize_t file_offset, __wasi_filesize_t len,
                                          uint32_t advice_extended) {
    __wasi_errno_t rc = 0;

    /* Narrow and cast advice from opaque 32-bit wasm32 type to __wasi_advice_t */
    if (unlikely(advice_extended > UINT8_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_advice_t advice = (__wasi_advice_t)advice_extended;

    rc = wasi_snapshot_preview1_backing_fd_advise(CURRENT_WASI_CONTEXT, fd, file_offset, len, advice);

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
    return (uint32_t)wasi_snapshot_preview1_backing_fd_allocate(CURRENT_WASI_CONTEXT, fd, offset, len);
};

/**
 * Close a file descriptor.
 *
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_close(__wasi_fd_t fd) {
    return (uint32_t)wasi_snapshot_preview1_backing_fd_close(CURRENT_WASI_CONTEXT, fd);
}

/**
 * Synchronize the data of a file to disk.
 *
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_datasync(__wasi_fd_t fd) {
    return (uint32_t)wasi_snapshot_preview1_backing_fd_datasync(CURRENT_WASI_CONTEXT, fd);
}

/**
 * Get the attributes of a file descriptor.
 *
 * @param fd
 * @param fdstat_retoffset return param of resulting wasi_fdstat structure
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_fdstat_get(__wasi_fd_t fd, __wasi_size_t fdstat_retoffset) {
    __wasi_fdstat_t* fdstat_retptr = (__wasi_fdstat_t*)get_memory_ptr_for_runtime(fdstat_retoffset,
                                                                                  WASI_SERDES_SIZE_fdstat_t);

    return (uint32_t)wasi_snapshot_preview1_backing_fd_fdstat_get(CURRENT_WASI_CONTEXT, fd, fdstat_retptr);
}

/**
 * Adjust the flags associated with a file descriptor
 *
 * @param fd
 * @param fdflags_extended The desired values of the file descriptor flags, zero extended to 32-bits
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EAGAIN, WASI_EBADF, WASI_EFAULT, WASI_EINVAL, WASI_ENOENT, or WASI_EPERM
 */
uint32_t wasi_snapshot_preview1_fd_fdstat_set_flags(__wasi_fd_t fd, uint32_t fdflags_extended) {
    __wasi_errno_t rc = 0;

    /* Narrow and cast fdflags from opaque 32-bit wasm32 type to __wasi_fdflags_t */
    if (unlikely(fdflags_extended > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_fdflags_t fdflags = (__wasi_fdflags_t)fdflags_extended;

    rc = wasi_snapshot_preview1_backing_fd_fdstat_set_flags(CURRENT_WASI_CONTEXT, fd, fdflags);

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
    return (uint32_t)wasi_snapshot_preview1_backing_fd_fdstat_set_rights(CURRENT_WASI_CONTEXT, fd, fs_rights_base,
                                                                         fs_rights_inheriting);
}

/**
 * Return the attributes of an open file.
 *
 * @param fd
 * @param filestat_retoffset The buffer where we should store the file's attributes
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_filestat_get(__wasi_fd_t fd, __wasi_size_t filestat_retoffset) {
    __wasi_filestat_t* filestat_retptr = (__wasi_filestat_t*)get_memory_ptr_for_runtime(filestat_retoffset,
                                                                                        WASI_SERDES_SIZE_filestat_t);

    return (uint32_t)wasi_snapshot_preview1_backing_fd_filestat_get(CURRENT_WASI_CONTEXT, fd, filestat_retptr);
}

/**
 * Adjust the size of an open file, zeroing extra bytes on increase
 *
 * @param fd
 * @param size The desired file size.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_filestat_set_size(__wasi_fd_t fd, __wasi_filesize_t size) {
    return (uint32_t)wasi_snapshot_preview1_backing_fd_filestat_set_size(CURRENT_WASI_CONTEXT, fd, size);
}

/**
 * Adjust the timestamps of an open file or directory
 *
 * @param fd
 * @param atim The desired values of the data access timestamp.
 * @param mtim The desired values of the data modification timestamp.
 * @param fstflags A bitmask indicating which timestamps to adjust.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_filestat_set_times(__wasi_fd_t fd, __wasi_timestamp_t atim, __wasi_timestamp_t mtim,
                                                      uint32_t fstflags_extended) {
    __wasi_errno_t rc = 0;

    /* Narrow and cast fst_flags from opaque 32-bit wasm32 type to __wasi_fstflags_t */
    if (unlikely(fstflags_extended > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_fstflags_t fstflags = (__wasi_fstflags_t)fstflags_extended;

    rc = wasi_snapshot_preview1_backing_fd_filestat_set_times(CURRENT_WASI_CONTEXT, fd, atim, mtim, fstflags);

done:
    return (uint32_t)rc;
}

/**
 * Read from a file descriptor without updating the descriptor's offset
 *
 * @param fd
 * @param iovs_baseoffset List of scatter/gather vectors in which to store data.
 * @param iovs_len The length of the iovs vector
 * @param offset The offset within the file at which to read.
 * @param nread_retoffset The number of bytes read.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_pread(__wasi_fd_t fd, __wasi_size_t iovs_baseoffset, __wasi_size_t iovs_len,
                                         __wasi_filesize_t offset, __wasi_size_t nread_retoffset) {
    __wasi_errno_t rc           = 0;
    __wasi_size_t* nread_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(nread_retoffset, WASI_SERDES_SIZE_size_t);

    /* Swizzle iovs vector, saving to temp buffer */
    check_bounds(iovs_baseoffset, WASI_SERDES_SIZE_iovec_t * iovs_len);
    __wasi_iovec_t* iovs_baseptr = calloc(iovs_len, sizeof(__wasi_iovec_t));
    if (unlikely(iovs_baseptr == NULL)) {
        goto done;
    }
    rc = wasi_serdes_readv_iovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseoffset, iovs_baseptr, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_fd_pread(CURRENT_WASI_CONTEXT, fd, iovs_baseptr, iovs_len, offset,
                                                 nread_retptr);

done:
    if (likely(iovs_baseptr != NULL)) {
        free(iovs_baseptr);
        iovs_baseptr = NULL;
    }

    return (uint32_t)rc;
}

/**
 * Return a description of the given preopened file descriptor.
 *
 * @param fd
 * @param prestat_retoffset The buffer where the description is stored.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_prestat_get(__wasi_fd_t fd, __wasi_size_t prestat_retoffset) {
    __wasi_prestat_t* prestat_retptr = (__wasi_prestat_t*)get_memory_ptr_for_runtime(prestat_retoffset,
                                                                                     WASI_SERDES_SIZE_prestat_t);

    return (uint32_t)wasi_snapshot_preview1_backing_fd_prestat_get(CURRENT_WASI_CONTEXT, fd, prestat_retptr);
}

/**
 * Return a description of the given preopened file descriptor.
 *
 * @param fd
 * @param dirname_retoffset A buffer into which to write the preopened directory name.
 * @param dirname_len The length of the buffer at path_retptr
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_fd_prestat_dir_name(__wasi_fd_t fd, __wasi_size_t dirname_retoffset, __wasi_size_t dirname_len) {
    char* dirname_retptr = get_memory_ptr_for_runtime(dirname_retoffset, dirname_len);

    return (uint32_t)wasi_snapshot_preview1_backing_fd_prestat_dir_name(CURRENT_WASI_CONTEXT, fd, dirname_retptr,
                                                                        dirname_len);
}

/**
 * Write to a file descriptor without updating the descriptor's offset
 *
 * @param fd
 * @param iovs_baseoffset List of scatter/gather vectors from which to retrieve data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param offset The offset within the file at which to write.
 * @param nwritten_retoffset The number of bytes written.
 * @return status code
 *
 */
uint32_t wasi_snapshot_preview1_fd_pwrite(__wasi_fd_t fd, __wasi_size_t iovs_baseoffset, __wasi_size_t iovs_len,
                                          __wasi_filesize_t file_offset, __wasi_size_t nwritten_retoffset) {
    __wasi_errno_t rc              = 0;
    __wasi_size_t* nwritten_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(nwritten_retoffset,
                                                                                WASI_SERDES_SIZE_size_t);

    /* Swizzle iovs, writting to temp buffer */
    check_bounds(iovs_baseoffset, iovs_len * WASI_SERDES_SIZE_ciovec_t);
    __wasi_ciovec_t* iovs_baseptr = calloc(iovs_len, sizeof(__wasi_ciovec_t));
    if (unlikely(iovs_baseptr == NULL)) {
        goto done;
    }
    rc = wasi_serdes_readv_ciovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseoffset, iovs_baseptr, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_fd_pwrite(CURRENT_WASI_CONTEXT, fd, iovs_baseptr, iovs_len, file_offset,
                                                  nwritten_retptr);

done:
    if (likely(iovs_baseptr != NULL)) {
        free(iovs_baseptr);
        iovs_baseptr = NULL;
    }

    return (uint32_t)rc;
}

/**
 * Read from a file descriptor
 *
 * @param fd
 * @param iovs_baseptr
 * @param iovs_len
 * @param nread_retoffset The number of bytes read.
 * @return WASI_ESUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT, WASI_EINTR, WASI_EIO, WASI_EISDIR, or
 * others
 */
uint32_t wasi_snapshot_preview1_fd_read(__wasi_fd_t fd, __wasi_size_t iovs_baseoffset, __wasi_size_t iovs_len,
                                        __wasi_size_t nread_retoffset) {
    __wasi_errno_t rc           = 0;
    __wasi_size_t* nread_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(nread_retoffset, WASI_SERDES_SIZE_size_t);

    /* Swizzle iovs, writting to temp buffer */
    check_bounds(iovs_baseoffset, WASI_SERDES_SIZE_iovec_t * iovs_len);
    __wasi_iovec_t* iovs_baseptr = calloc(iovs_len, sizeof(__wasi_iovec_t));
    if (unlikely(iovs_baseptr == NULL)) {
        goto done;
    }
    rc = wasi_serdes_readv_iovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseoffset, iovs_baseptr, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_fd_read(CURRENT_WASI_CONTEXT, fd, iovs_baseptr, iovs_len, nread_retptr);

done:
    if (likely(iovs_baseptr != NULL)) {
        free(iovs_baseptr);
        iovs_baseptr = NULL;
    }

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
 * @param buf_baseoffset The buffer where directory entries are stored
 * @param buf_len
 * @param cookie The location within the directory to start reading
 * @param nread_retoffset The number of bytes stored in the read buffer. If less than the size of the read buffer, the
 * end of the directory has been reached.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_readdir(__wasi_fd_t fd, __wasi_size_t buf_baseoffset, __wasi_size_t buf_len,
                                           __wasi_dircookie_t cookie, __wasi_size_t nread_retoffset) {
    uint8_t*       buf_baseptr  = (uint8_t*)get_memory_ptr_for_runtime(buf_baseoffset, buf_len);
    __wasi_size_t* nread_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(nread_retoffset, WASI_SERDES_SIZE_size_t);

    return (uint32_t)wasi_snapshot_preview1_backing_fd_readdir(CURRENT_WASI_CONTEXT, fd, buf_baseptr, buf_len, cookie,
                                                               nread_retptr);
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
    return (uint32_t)wasi_snapshot_preview1_backing_fd_renumber(CURRENT_WASI_CONTEXT, fd, to);
}

/**
 * Move the offset of a file descriptor
 *
 * @param fd
 * @param file_offset The number of bytes to move.
 * @param whence_extended The base from which the offset is relative.
 * @param newoffset_retoffset The new offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_seek(__wasi_fd_t fd, __wasi_filedelta_t file_offset, uint32_t whence_extended,
                                        __wasi_size_t newoffset_retoffset) {
    __wasi_errno_t rc = 0;

    /* Narrow and cast whence from opaque 32-bit wasm32 type to __wasi_whence_t */
    if (unlikely(whence_extended > UINT8_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_whence_t whence = (__wasi_whence_t)whence_extended;

    __wasi_filesize_t* newoffset_retptr = (__wasi_filesize_t*)get_memory_ptr_for_runtime(newoffset_retoffset,
                                                                                         WASI_SERDES_SIZE_filesize_t);

    rc = wasi_snapshot_preview1_backing_fd_seek(CURRENT_WASI_CONTEXT, fd, file_offset, whence, newoffset_retptr);

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
    return (uint32_t)wasi_snapshot_preview1_backing_fd_sync(CURRENT_WASI_CONTEXT, fd);
}

/**
 * Return the current offset of a file descriptor
 *
 * @param fd
 * @param fileoffset_retoffset The current offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_tell(__wasi_fd_t fd, __wasi_size_t fileoffset_retoffset) {
    __wasi_filesize_t* fileoffset_retptr = (__wasi_filesize_t*)get_memory_ptr_for_runtime(fileoffset_retoffset,
                                                                                          WASI_SERDES_SIZE_filesize_t);

    return (uint32_t)wasi_snapshot_preview1_backing_fd_tell(CURRENT_WASI_CONTEXT, fd, fileoffset_retptr);
}

/**
 * Write to a file descriptor
 *
 * @param fd
 * @param iovs_baseoffset List of scatter/gather vectors from which to retrieve data.
 * @param iovs_len The length of the array pointed to by `iovs`.
 * @param nwritten_retoffset
 * @return WASI_ESUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT,
 * WASI_EFBIG, WASI_EINTR, WASI_EIO, WASI_ENOSPC, WASI_EPERM, WASI_EPIPE, or others
 */
uint32_t wasi_snapshot_preview1_fd_write(__wasi_fd_t fd, __wasi_size_t iovs_baseoffset, __wasi_size_t iovs_len,
                                         __wasi_size_t nwritten_retoffset) {
    __wasi_errno_t rc              = 0;
    __wasi_size_t* nwritten_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(nwritten_retoffset,
                                                                                WASI_SERDES_SIZE_size_t);

    /* Swizzle iovs, writting to temporary buffer */
    check_bounds(iovs_baseoffset, WASI_SERDES_SIZE_ciovec_t * iovs_len);
    __wasi_ciovec_t* iovs_baseptr = calloc(iovs_len, sizeof(__wasi_ciovec_t));
    if (unlikely(iovs_baseptr == NULL)) {
        goto done;
    }
    rc = wasi_serdes_readv_ciovec_t(CURRENT_MEMORY_BASE, CURRENT_MEMORY_SIZE, iovs_baseoffset, iovs_baseptr, iovs_len);
    if (unlikely(rc != __WASI_ERRNO_SUCCESS)) {
        goto done;
    }

    rc = wasi_snapshot_preview1_backing_fd_write(CURRENT_WASI_CONTEXT, fd, iovs_baseptr, iovs_len, nwritten_retptr);

done:
    if (likely(iovs_baseptr != NULL)) {
        free(iovs_baseptr);
        iovs_baseptr = NULL;
    }

    return (uint32_t)rc;
}

/**
 * Create a directory
 *
 * @param fd
 * @param path_baseoffset
 * @param path_len
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EBADF, WASI_EDQUOT, WASI_EEXIST,
 * WASI_EFAULT, WASI_EINVAL, WASI_ELOOP, WASI_EMLINK, WASI_ENAMETOOLONG,
 * WASI_ENOENT, WASI_ENOMEM, WASI_ENOSPC, WASI_ENOTDIR, WASI_EPERM, or WASI_EROFS
 */
uint32_t
wasi_snapshot_preview1_path_create_directory(__wasi_fd_t fd, __wasi_size_t path_baseoffset, __wasi_size_t path_len) {
    const char* path_baseptr = (const char*)get_memory_ptr_for_runtime(path_baseoffset, path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_create_directory(CURRENT_WASI_CONTEXT, fd, path_baseptr,
                                                                          path_len);
}

/**
 * Return the attributes of a file or directory
 *
 * @param fd
 * @param flags Flags determining the method of how the path is resolved.
 * @param path_baseoffset The path of the file or directory to inspect.
 * @param filestat_retoffset The buffer where the file's attributes are stored.
 * @return WASI_ESUCCESS, WASI_EACCES, WASI_EBAD, WASI_EFAUL, WASI_EINVAL, WASI_ELOOP,
 * WASI_ENAMETOOLON, WASI_ENOENT, WASI_ENOENT, WASI_ENOMEM, WASI_ENOTDI, or WASI_EOVERFLOW
 */
uint32_t
wasi_snapshot_preview1_path_filestat_get(__wasi_fd_t fd, __wasi_lookupflags_t flags, __wasi_size_t path_baseoffset,
                                         __wasi_size_t path_len, __wasi_size_t filestat_retoffset) {
    const char*        path_baseptr    = (const char*)get_memory_ptr_for_runtime(path_baseoffset, path_len);
    __wasi_filestat_t* filestat_retptr = (__wasi_filestat_t*)get_memory_ptr_for_runtime(filestat_retoffset,
                                                                                        WASI_SERDES_SIZE_filestat_t);

    return (uint32_t)wasi_snapshot_preview1_backing_path_filestat_get(CURRENT_WASI_CONTEXT, fd, flags, path_baseptr,
                                                                      path_len, filestat_retptr);
}

/**
 * Adjust the timestamps of a file or directory
 *
 * @param fd
 * @param flags Flags determining the method of how the path is resolved.
 * @param path_baseoffset The path of the file or directory to operate on.
 * @param path_len
 * @param atim The desired values of the data access timestamp.
 * @param mtim The desired values of the data modification timestamp.
 * @param fstflags_extended A bitmask indicating which timestamps to adjust.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_path_filestat_set_times(__wasi_fd_t fd, __wasi_lookupflags_t flags,
                                                        __wasi_size_t path_baseoffset, __wasi_size_t path_len,
                                                        __wasi_timestamp_t atim, __wasi_timestamp_t mtim,
                                                        uint32_t fstflags_extended) {
    __wasi_errno_t rc           = 0;
    const char*    path_baseptr = (const char*)get_memory_ptr_for_runtime(path_baseoffset, path_len);

    /* Narrow and cast fst_flags from opaque 32-bit wasm32 type to __wasi_fstflags_t */
    if (unlikely(fstflags_extended > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_fstflags_t fstflags = (__wasi_fstflags_t)fstflags_extended;

    rc = wasi_snapshot_preview1_backing_path_filestat_set_times(CURRENT_WASI_CONTEXT, fd, flags, path_baseptr, path_len,
                                                                atim, mtim, fstflags);

done:
    return (uint32_t)rc;
}

/**
 * Create a hard link
 *
 * @param old_fd
 * @param old_flags Flags determining the method of how the path is resolved.
 * @param old_path_baseoffset The source path from which to link.
 * @param old_path_len
 * @param new_fd The working directory at which the resolution of the new path starts.
 * @param new_path_baseoffset The destination path at which to create the hard link.
 * @param new_path_len
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_path_link(__wasi_fd_t old_fd, __wasi_lookupflags_t old_flags, __wasi_size_t old_path_baseoffset,
                                 __wasi_size_t old_path_len, __wasi_fd_t new_fd, __wasi_size_t new_path_baseoffset,
                                 __wasi_size_t new_path_len) {
    const char* old_path_baseptr = (const char*)get_memory_ptr_for_runtime(old_path_baseoffset, old_path_len);
    const char* new_path_baseptr = (const char*)get_memory_ptr_for_runtime(new_path_baseoffset, new_path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_link(CURRENT_WASI_CONTEXT, old_fd, old_flags, old_path_baseptr,
                                                              old_path_len, new_fd, new_path_baseptr, new_path_len);
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
 * @param path_baseoffset
 * @param path_len
 * @param oflags
 * @param fs_rights_base
 * @param fs_rights_inheriting
 * @param fdflags
 * @param fd_retoffset The file descriptor of the file that has been opened.
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_path_open(__wasi_fd_t dirfd, __wasi_lookupflags_t lookupflags, __wasi_size_t path_baseoffset,
                                 __wasi_size_t path_len, uint32_t oflags_extended, __wasi_rights_t fs_rights_base,
                                 __wasi_rights_t fs_rights_inheriting, uint32_t fdflags_extended,
                                 __wasi_size_t fd_retoffset) {
    __wasi_errno_t rc           = 0;
    const char*    path_baseptr = (const char*)get_memory_ptr_for_runtime(path_baseoffset, path_len);

    /* Narrow and cast oflags from opaque 32-bit wasm32 type to __wasi_oflags_t */
    if (unlikely(oflags_extended > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_oflags_t oflags = (__wasi_oflags_t)oflags_extended;

    /* Narrow and cast fdflags from opaque 32-bit wasm32 type to __wasi_fdflags_t */
    if (unlikely(fdflags_extended > UINT16_MAX)) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_fdflags_t fdflags = (__wasi_fdflags_t)fdflags_extended;

    __wasi_fd_t* fd_retptr = (__wasi_fd_t*)get_memory_ptr_for_runtime(fd_retoffset, WASI_SERDES_SIZE_fd_t);

    rc = wasi_snapshot_preview1_backing_path_open(CURRENT_WASI_CONTEXT, dirfd, lookupflags, path_baseptr, path_len,
                                                  oflags, fs_rights_base, fs_rights_inheriting, fdflags, fd_retptr);

done:
    return (uint32_t)rc;
}

/**
 * Read the contents of a symbolic link
 *
 * @param fd
 * @param path_baseoffset The path of the symbolic link from which to read.
 * @param path_len
 * @param buf_baseretoffset The buffer to which to write the contents of the symbolic link.
 * @param buf_len
 * @param nread_retoffset The number of bytes placed in the buffer.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_path_readlink(__wasi_fd_t fd, __wasi_size_t path_baseoffset, __wasi_size_t path_len,
                                              __wasi_size_t buf_baseretoffset, __wasi_size_t buf_len,
                                              __wasi_size_t nread_retoffset) {
    uint8_t*       buf_retptr   = (uint8_t*)get_memory_ptr_for_runtime(buf_baseretoffset, buf_len);
    const char*    path_baseptr = (const char*)get_memory_ptr_for_runtime(path_baseoffset, path_len);
    __wasi_size_t* nread_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(nread_retoffset, WASI_SERDES_SIZE_size_t);

    return (uint32_t)wasi_snapshot_preview1_backing_path_readlink(CURRENT_WASI_CONTEXT, fd, path_baseptr, path_len,
                                                                  buf_retptr, buf_len, nread_retptr);
}

/**
 * Remove a directory
 * Return `errno::notempty` if the directory is not empty.
 *
 * @param fd
 * @param path_baseoffset The path to a directory to remove.
 * @param path_len
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_path_remove_directory(__wasi_fd_t fd, __wasi_size_t path_baseoffset, __wasi_size_t path_len) {
    const char* path_baseptr = (const char*)get_memory_ptr_for_runtime(path_baseoffset, path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_remove_directory(CURRENT_WASI_CONTEXT, fd, path_baseptr,
                                                                          path_len);
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
wasi_snapshot_preview1_path_rename(__wasi_fd_t fd, __wasi_size_t old_path_baseoffset, __wasi_size_t old_path_len,
                                   __wasi_fd_t new_fd, __wasi_size_t new_path_baseoffset, __wasi_size_t new_path_len) {
    const char* old_path_baseptr = (const char*)get_memory_ptr_for_runtime(old_path_baseoffset, old_path_len);
    const char* new_path_baseptr = (const char*)get_memory_ptr_for_runtime(new_path_baseoffset, new_path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_rename(CURRENT_WASI_CONTEXT, fd, old_path_baseptr,
                                                                old_path_len, new_fd, new_path_baseptr, new_path_len);
}

/**
 * Create a symbolic link
 *
 * @param old_path_baseoffset The contents of the symbolic link.
 * @param old_path_len
 * @param fd
 * @param new_path_baseoffset The path where we want the symbolic link.
 * @param new_path_len
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_path_symlink(__wasi_size_t old_path_baseoffset, __wasi_size_t old_path_len, __wasi_fd_t fd,
                                    __wasi_size_t new_path_baseoffset, __wasi_size_t new_path_len) {
    const char* old_path_baseptr = (const char*)get_memory_ptr_for_runtime(old_path_baseoffset, old_path_len);
    const char* new_path_baseptr = (const char*)get_memory_ptr_for_runtime(new_path_baseoffset, new_path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_symlink(CURRENT_WASI_CONTEXT, old_path_baseptr, old_path_len,
                                                                 fd, new_path_baseptr, new_path_len);
}

/**
 * Unlink a file
 * Return `errno::isdir` if the path refers to a directory.
 *
 * @param fd
 * @param path_baseoffset
 * @param path_len
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_path_unlink_file(__wasi_fd_t fd, __wasi_size_t path_baseoffset, __wasi_size_t path_len) {
    const char* path_baseptr = (const char*)get_memory_ptr_for_runtime(path_baseoffset, path_len);

    return (uint32_t)wasi_snapshot_preview1_backing_path_unlink_file(CURRENT_WASI_CONTEXT, fd, path_baseptr, path_len);
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
uint32_t wasi_snapshot_preview1_poll_oneoff(__wasi_size_t in_baseoffset, __wasi_size_t out_baseoffset,
                                            __wasi_size_t nsubscriptions, __wasi_size_t nevents_retoffset) {
    const __wasi_subscription_t* in_baseptr = (const __wasi_subscription_t*)
      get_memory_ptr_for_runtime(in_baseoffset, WASI_SERDES_SIZE_subscription_t * nsubscriptions);
    __wasi_event_t* out_baseptr    = (__wasi_event_t*)get_memory_ptr_for_runtime(out_baseoffset,
                                                                                 WASI_SERDES_SIZE_subscription_t
                                                                                   * nsubscriptions);
    __wasi_size_t*  nevents_retptr = (__wasi_size_t*)get_memory_ptr_for_runtime(nevents_retoffset,
                                                                                WASI_SERDES_SIZE_size_t);

    return (uint32_t)wasi_snapshot_preview1_backing_poll_oneoff(CURRENT_WASI_CONTEXT, in_baseptr, out_baseptr,
                                                                nsubscriptions, nevents_retptr);
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
 * @param sig_extended The signal condition to trigger.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_proc_raise(uint32_t sig_extended) {
    __wasi_errno_t rc = 0;

    /* Narrow and cast sig from opaque 32-bit wasm32 type to __wasi_signal_t */
    if (sig_extended > UINT8_MAX) {
        rc = __WASI_ERRNO_INVAL;
        goto done;
    }
    __wasi_signal_t sig = (__wasi_signal_t)sig_extended;

    rc = wasi_snapshot_preview1_backing_proc_raise(CURRENT_WASI_CONTEXT, sig);

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
 * @param buf_baseretoffset The buffer to fill with random data.
 * @param buf_len The length of the buffer
 * @return status code
 */
uint32_t wasi_snapshot_preview1_random_get(__wasi_size_t buf_baseretoffset, __wasi_size_t buf_len) {
    uint8_t* buf_baseretptr = (uint8_t*)get_memory_ptr_for_runtime(buf_baseretoffset, buf_len);

    return (uint32_t)wasi_snapshot_preview1_backing_random_get(CURRENT_WASI_CONTEXT, buf_baseretptr, buf_len);
}

/**
 * Temporarily yield execution of the calling thread similar to `sched_yield` in POSIX.
 * This implementation ignores client calls and silently returns RC 0
 *
 * @return WASI_ESUCCESS
 */
uint32_t wasi_snapshot_preview1_sched_yield(void) {
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
 * @param ri_data_baseretoffset List of scatter/gather vectors to which to store data.
 * @param ri_data_len The length of the array pointed to by `ri_data`.
 * @param ri_flags Message flags.
 * @param ri_data_nbytes_retoffset Number of bytes stored in ri_data flags.
 * @param message_nbytes_retoffset Number of bytes stored in message flags.
 * @return status code
 */
uint32_t
wasi_snapshot_preview1_sock_recv(__wasi_fd_t fd, __wasi_size_t ri_data_baseretoffset, __wasi_size_t ri_data_len,
                                 uint32_t ri_flags_extended, __wasi_size_t ri_data_nbytes_retoffset,
                                 __wasi_size_t message_nbytes_retoffset) {
    return wasi_unsupported_syscall(__func__);
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
 * @param si_data_baseoffset List of scatter/gather vectors to which to retrieve data
 * @param si_data_len The length of the array pointed to by `si_data`.
 * @param si_flags Message flags.
 * @param nbytes_retoffset Number of bytes transmitted.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_sock_send(__wasi_fd_t fd, __wasi_size_t si_data_baseoffset, __wasi_size_t si_data_len,
                                          uint32_t si_flags_extended, __wasi_size_t nbytes_retoffset) {
    return wasi_unsupported_syscall(__func__);
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
    return wasi_unsupported_syscall(__func__);
}
