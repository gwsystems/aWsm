#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <uvwasi.h>

#include "../runtime.h"

/* Code that actually runs the wasm code */
IMPORT void wasmf__start(void);

#define unlikely(x) __builtin_expect(!!(x), 0)

static uvwasi_t uvwasi;

/* TODO: This likely makes assumptions about the memory model and needs to be generalized */
static inline void check_bounds(uint32_t offset, uint32_t bounds_check)
{
    // Due to how we setup memory for x86, the virtual memory mechanism will catch the error, if bounds < WASM_PAGE_SIZE
    assert(bounds_check < WASM_PAGE_SIZE || (memory_size > bounds_check && offset <= memory_size - bounds_check));
}

/**
 * @brief Lifecycle function that runs at end at termination of WebAssembly module. Suitable for logging and cleanup
 */
void runtime_on_module_exit()
{
    return;
}

int main(int argc, char *argv[])
{

    runtime_init();

    uvwasi_options_t init_options;
    uvwasi_options_init(&init_options);

    /* Pass through arguments from host process */
    init_options.argc = argc;
    init_options.argv = calloc(argc, sizeof(char *));
    for (int i = 0; i < argc; i++)
    {
        init_options.argv[i] = argv[i];
    }

    /* Pass through environment from host process */
    extern const char **environ;
    init_options.envp = environ;

    /* Mount local directory as /home */
    init_options.preopenc = 1;
    init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
    init_options.preopens[0].mapped_path = "/sandbox";
    init_options.preopens[0].real_path = ".";

    /* Initialize the sandbox. */
    uvwasi_errno_t err = uvwasi_init(&uvwasi, &init_options);
    assert(err == UVWASI_ESUCCESS);

    atexit(runtime_on_module_exit);
    wasmf__start();

    /* Clean up resources. */
    uvwasi_destroy(&uvwasi);

    exit(EXIT_SUCCESS);
}

__attribute__((noreturn)) void wasi_unsupported_syscall(const char *syscall)
{
    fprintf(stderr, "Syscall %s is not supported\n", syscall);
    exit(EXIT_FAILURE);
}

/**
 * @brief Writes argument offsets and buffer into linear memory write
 * 
 * @param argv_retptr
 * @param argv_buf_retptr
 * @return status code
 */
uint32_t wasi_snapshot_preview1_args_get(
    uvwasi_size_t argv_retptr,
    uvwasi_size_t argv_buf_retptr)
{
    /* Check Bounds */
    check_bounds(argv_retptr, sizeof(uvwasi_size_t) * uvwasi.argc);
    check_bounds(argv_buf_retptr, uvwasi.argv_buf_size);

    /* 
     * Write results to temporary buffers argv and argv_buf
     * Should we write argv_buf directly to memory to make more efficient? 
     */
    uvwasi_size_t argv[uvwasi.argc];
    char argv_buf[uvwasi.argv_buf_size];
    uvwasi_errno_t rc = uvwasi_args_get(&uvwasi, (char **)&argv, argv_buf);

    if (rc == UVWASI_ESUCCESS)
    {
        /* Adjust argv to use offsets, not host pointers */
        for (size_t i = 0; i < uvwasi.argc; i++)
        {
            argv[i] = argv_buf_retptr + (uvwasi.argv[i] - uvwasi.argv[0]);
        }

        /* Write argv and argv_buf to linear memory */
        memcpy(&memory[argv_retptr], &argv, uvwasi.argc * sizeof(uvwasi_size_t));
        memcpy(&memory[argv_buf_retptr], &argv_buf, uvwasi.argv_buf_size);
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
uint32_t wasi_snapshot_preview1_args_sizes_get(
    uvwasi_size_t argc_retptr,
    uvwasi_size_t argv_buf_len_retptr)
{
    /* Check Bounds */
    check_bounds(argc_retptr, sizeof(uint32_t));
    check_bounds(argv_buf_len_retptr, sizeof(uint32_t));

    /* Get sizes */
    uvwasi_size_t argc;
    uvwasi_size_t argv_buf_size;
    uvwasi_errno_t rc = uvwasi_args_sizes_get(&uvwasi, &argc, &argv_buf_size);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write to linear memory */
    uvwasi_serdes_write_size_t(memory, argc_retptr, argc);
    uvwasi_serdes_write_size_t(memory, argv_buf_len_retptr, argv_buf_size);

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
uint32_t wasi_snapshot_preview1_clock_res_get(
    uvwasi_clockid_t id,
    uvwasi_size_t res_retptr)
{
    /* Check Bounds */
    check_bounds(res_retptr, sizeof(uvwasi_timestamp_t));

    /* Read resolution into buffer */
    uvwasi_timestamp_t resolution;
    uvwasi_errno_t rc = uvwasi_clock_res_get(&uvwasi,
                                             id,
                                             &resolution);

    /* write buffer to linear memory */
    if (rc == UVWASI_ESUCCESS)
        uvwasi_serdes_write_timestamp_t(memory, res_retptr, resolution);

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
uint32_t wasi_snapshot_preview1_clock_time_get(
    uvwasi_clockid_t clock_id,
    uvwasi_timestamp_t precision,
    uvwasi_size_t time_retptr)
{
    /* Check Bounds */
    check_bounds(time_retptr, sizeof(uvwasi_timestamp_t));

    /* Make WASI call */
    uvwasi_timestamp_t time;
    uvwasi_errno_t rc = uvwasi_clock_time_get(&uvwasi, clock_id, precision, &time);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write to Linear Memory */
    uvwasi_serdes_write_timestamp_t(memory, time_retptr, time);

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
uint32_t wasi_snapshot_preview1_environ_get(
    uvwasi_size_t environ_retptr,
    uvwasi_size_t environ_buf_retptr)
{
    /* Check Bounds */
    check_bounds(environ_retptr, uvwasi.envc * UVWASI_SERDES_SIZE_size_t);
    check_bounds(environ_buf_retptr, uvwasi.env_buf_size);

    /* Write environment to temporary buffer and environment_buf directly to linear memory */
    char *environment[uvwasi.envc];
    uvwasi_errno_t rc = uvwasi_environ_get(&uvwasi, environment, &memory[environ_buf_retptr]);

    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Convert environment from pointers to offsets and write to linear memory */
    for (size_t i = 0; i < uvwasi.envc; i++)
    {
        uvwasi_size_t offset = environ_buf_retptr + (environment[i] - environment[0]);

        uvwasi_serdes_write_uint32_t(memory,
                                     environ_retptr +
                                         (i * UVWASI_SERDES_SIZE_uint32_t),
                                     offset);
    }

done:
    return (uint32_t)rc;
}

/**
 * Returns the number of environment variable arguments and the size of the environment variable data.
 * 
 * @param environc_retptr - the offset where the resulting number of environment variable arguments should be written
 * @param environv_buf_len_retptr - the offset where the resulting size of the environment variable data should be written
 * @return status code
 */
uint32_t wasi_snapshot_preview1_environ_sizes_get(
    uvwasi_size_t environc_retptr,
    uvwasi_size_t environv_buf_len_retptr)
{
    /* Check Bounds */
    check_bounds(environc_retptr, UVWASI_SERDES_SIZE_size_t);
    check_bounds(environv_buf_len_retptr, UVWASI_SERDES_SIZE_size_t);

    /* Make WASI Call */
    uvwasi_size_t envc;
    uvwasi_size_t env_buf_size;
    uvwasi_errno_t rc = uvwasi_environ_sizes_get(&uvwasi,
                                                 &envc,
                                                 &env_buf_size);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write results to linear memory */
    uvwasi_serdes_write_size_t(memory, environc_retptr, envc);
    uvwasi_serdes_write_size_t(memory, environv_buf_len_retptr, env_buf_size);

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
uint32_t wasi_snapshot_preview1_fd_advise(
    uvwasi_fd_t fd,
    uvwasi_filesize_t offset,
    uvwasi_filesize_t len,
    uint32_t advice)
{
    uvwasi_errno_t rc;

    if (unlikely(advice > UINT8_MAX))
        goto err_advice_overflow;

    rc = uvwasi_fd_advise(&uvwasi, fd, offset, len, (uvwasi_advice_t)advice);

done:
    return (uint32_t)rc;
err_advice_overflow:
    rc = UVWASI_EINVAL;
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
uint32_t wasi_snapshot_preview1_fd_allocate(
    uvwasi_fd_t fd,
    uvwasi_filesize_t offset,
    uvwasi_filesize_t len)
{
    return (uint32_t)uvwasi_fd_allocate(&uvwasi, fd, offset, len);
};

/**
 * Close a file descriptor.
 * 
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_close(uvwasi_fd_t fd)
{
    return (uint32_t)uvwasi_fd_close(&uvwasi, fd);
}

/**
 * Synchronize the data of a file to disk.
 * 
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_datasync(uvwasi_fd_t fd)
{
    return (uint32_t)uvwasi_fd_datasync(&uvwasi, fd);
}

/**
 * Get the attributes of a file descriptor.
 * 
 * @param fd 
 * @param fdstat_retptr the offset where the resulting wasi_fdstat structure should be written
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_fdstat_get(
    uvwasi_fd_t fd,
    uvwasi_size_t fdstat_retptr)
{
    /* Check Bounds */
    check_bounds(fdstat_retptr, UVWASI_SERDES_SIZE_fdstat_t);

    /* Make WASI Call */
    uvwasi_fdstat_t stats;
    uvwasi_errno_t rc = uvwasi_fd_fdstat_get(&uvwasi, (uvwasi_fd_t)fd, &stats);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write result to linear memory */
    uvwasi_serdes_write_fdstat_t(memory, fdstat_retptr, &stats);

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
uint32_t wasi_snapshot_preview1_fd_fdstat_set_flags(
    uvwasi_fd_t fd,
    uint32_t fdflags)
{
    uvwasi_errno_t rc;

    /* fdflags should be a zero-extended uint16_t */
    if (fdflags > UINT16_MAX)
        goto err_fdflags_overflow;

    rc = uvwasi_fd_fdstat_set_flags(&uvwasi, fd, (uvwasi_fdflags_t)fdflags);

done:
    return (uint32_t)rc;
err_fdflags_overflow:
    rc = UVWASI_EINVAL;
    goto done;
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
uint32_t wasi_snapshot_preview1_fd_fdstat_set_rights(
    uvwasi_fd_t fd,
    uvwasi_rights_t fs_rights_base,
    uvwasi_rights_t fs_rights_inheriting)
{
    return (uint32_t)uvwasi_fd_fdstat_set_rights(&uvwasi,
                                                 fd,
                                                 fs_rights_base,
                                                 fs_rights_inheriting);
}

/**
 * Return the attributes of an open file.
 * 
 * @param fd
 * @param filestat_retptr The buffer where we should store the file's attributes
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_filestat_get(
    uvwasi_fd_t fd,
    uvwasi_size_t filestat_retptr)
{
    /* Check Bounds */
    check_bounds(filestat_retptr, UVWASI_SERDES_SIZE_filestat_t);

    /* Make WASI Call */
    uvwasi_filestat_t stats;
    uvwasi_errno_t rc = uvwasi_fd_filestat_get(&uvwasi, fd, &stats);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Copy result into linear memory */
    uvwasi_serdes_write_filestat_t(memory, filestat_retptr, &stats);

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
uint32_t wasi_snapshot_preview1_fd_filestat_set_size(
    uvwasi_fd_t fd,
    uvwasi_filesize_t size)
{
    return (uint32_t)uvwasi_fd_filestat_set_size(&uvwasi, fd, size);
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
uint32_t wasi_snapshot_preview1_fd_filestat_set_times(
    uvwasi_fd_t fd,
    uvwasi_timestamp_t atim,
    uvwasi_timestamp_t mtim,
    uint32_t fst_flags)
{
    uvwasi_errno_t rc;
    if (unlikely(fst_flags > UINT16_MAX))
        goto err_fst_flags_overflow;

    rc = uvwasi_fd_filestat_set_times(&uvwasi,
                                      fd,
                                      atim,
                                      mtim,
                                      (uvwasi_fstflags_t)fst_flags);

done:
    return (uint32_t)rc;
err_fst_flags_overflow:
    rc = UVWASI_EINVAL;
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
uint32_t wasi_snapshot_preview1_fd_pread(
    uvwasi_fd_t fd,
    uvwasi_size_t iovs_baseptr,
    uvwasi_size_t iovs_len,
    uvwasi_filesize_t offset,
    uvwasi_size_t nbytes_retptr)
{
    /* Check Bounds */
    check_bounds(iovs_baseptr, iovs_len * UVWASI_SERDES_SIZE_iovec_t);
    check_bounds(nbytes_retptr, UVWASI_SERDES_SIZE_size_t);

    /* Read iovec from linear memory */
    uvwasi_iovec_t iovs[iovs_len];
    uvwasi_errno_t rc = uvwasi_serdes_readv_iovec_t(memory,
                                                    memory_size,
                                                    iovs_baseptr,
                                                    iovs,
                                                    iovs_len);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Execute WASI call */
    uvwasi_size_t nread;
    rc = uvwasi_fd_pread(&uvwasi, fd, iovs, iovs_len, offset, &nread);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write Result */
    uvwasi_serdes_write_size_t(memory, nbytes_retptr, nread);

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
uint32_t wasi_snapshot_preview1_fd_prestat_get(
    uvwasi_fd_t fd,
    uvwasi_size_t prestat_retptr)
{
    /* Check Bounds */
    check_bounds(prestat_retptr, UVWASI_SERDES_SIZE_prestat_t);

    /* Execute WASI call */
    uvwasi_prestat_t prestat;
    uvwasi_errno_t rc = uvwasi_fd_prestat_get(&uvwasi, fd, &prestat);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write result to linear memory */
    uvwasi_serdes_write_prestat_t(memory, prestat_retptr, &prestat);

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
uint32_t wasi_snapshot_preview1_fd_prestat_dir_name(
    uvwasi_fd_t fd,
    uvwasi_size_t path_retptr,
    uvwasi_size_t path_len)
{
    /* Check Bounds */
    check_bounds(path_retptr, path_len);

    /* Execute WASI call, writing results directly to linear memory */
    return (uint32_t)uvwasi_fd_prestat_dir_name(&uvwasi,
                                                fd,
                                                &memory[path_retptr],
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
uint32_t wasi_snapshot_preview1_fd_pwrite(
    uvwasi_fd_t fd,
    uvwasi_size_t iovs_baseptr,
    uvwasi_size_t iovs_len,
    uvwasi_filesize_t offset,
    uvwasi_size_t nwritten_retptr)
{
    /* Check Bounds */
    check_bounds(iovs_baseptr, iovs_len * UVWASI_SERDES_SIZE_ciovec_t);
    check_bounds(nwritten_retptr, UVWASI_SERDES_SIZE_size_t);

    /* Copy iovs into memory */
    uvwasi_ciovec_t iovs[iovs_len];
    uvwasi_errno_t rc = uvwasi_serdes_readv_ciovec_t(memory,
                                                     memory_size,
                                                     iovs_baseptr,
                                                     iovs,
                                                     iovs_len);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Execute WASI call */
    uvwasi_size_t nwritten;
    rc = uvwasi_fd_pwrite(&uvwasi,
                          fd,
                          iovs,
                          iovs_len,
                          offset,
                          &nwritten);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write result to linear memory */
    uvwasi_serdes_write_size_t(memory, nwritten_retptr, nwritten);

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
 * @return WASI_ESUCCESS, WASI_EAGAIN, WASI_EWOULDBLOCK, WASI_EBADF, WASI_EFAULT, WASI_EINTR, WASI_EIO, WASI_EISDIR, or others
 */
uint32_t wasi_snapshot_preview1_fd_read(
    uvwasi_fd_t fd,
    uvwasi_size_t iovs_baseptr,
    uvwasi_size_t iovs_len,
    uvwasi_size_t nread_retptr)
{
    /* Check Bounds */
    check_bounds(iovs_baseptr, iovs_len * UVWASI_SERDES_SIZE_iovec_t);
    check_bounds(nread_retptr, UVWASI_SERDES_SIZE_size_t);

    /* Copy iovs into memory */
    uvwasi_iovec_t iovs[iovs_len];
    uvwasi_errno_t rc = uvwasi_serdes_readv_iovec_t(memory,
                                                    memory_size,
                                                    iovs_baseptr,
                                                    iovs,
                                                    iovs_len);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Execute WASI call */
    uvwasi_size_t nread;
    rc = uvwasi_fd_read(&uvwasi, fd, iovs, iovs_len, &nread);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write result to linear memory */
    uvwasi_serdes_write_size_t(memory, nread_retptr, nread);

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
 * @param nread_retptr The number of bytes stored in the read buffer. If less than the size of the read buffer, the end of the directory has been reached.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_readdir(
    uvwasi_fd_t fd,
    uvwasi_size_t buf_baseptr,
    uvwasi_size_t buf_len,
    uvwasi_dircookie_t cookie,
    uvwasi_size_t nread_retptr)
{
    /* Check Bounds */
    check_bounds(buf_baseptr, buf_len);
    check_bounds(nread_retptr, UVWASI_SERDES_SIZE_size_t);

    /* Execute WASI call */
    uvwasi_size_t nread;
    uvwasi_errno_t rc = uvwasi_fd_readdir(&uvwasi,
                                          fd,
                                          &memory[buf_baseptr],
                                          buf_len,
                                          cookie,
                                          &nread);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write result to linear memory */
    uvwasi_serdes_write_size_t(memory, nread_retptr, nread);

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
uint32_t wasi_snapshot_preview1_fd_renumber(
    uvwasi_fd_t fd,
    uvwasi_fd_t to)
{
    return (uint32_t)uvwasi_fd_renumber(&uvwasi, fd, to);
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
uint32_t wasi_snapshot_preview1_fd_seek(
    uvwasi_fd_t fd,
    uvwasi_filedelta_t file_offset,
    uint32_t whence,
    uvwasi_size_t newoffset_retptr)
{
    uvwasi_errno_t rc;

    /* Validate types zero-extended by wasm32 ABI */
    if (unlikely(whence > UINT8_MAX))
    {
        rc = UVWASI_EINVAL;
        goto done;
    }

    /* Check Bounds */
    check_bounds(newoffset_retptr, UVWASI_SERDES_SIZE_filesize_t);

    /* Execute WASI syscall */
    uvwasi_filesize_t newoffset;
    rc = uvwasi_fd_seek(&uvwasi,
                        fd,
                        file_offset,
                        (uvwasi_whence_t)whence,
                        &newoffset);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write results to linear memory */
    uvwasi_serdes_write_filesize_t(memory, newoffset_retptr, newoffset);

done:
    return (uint32_t)rc;
}

/**
 * Synchronize the data and metadata of a file to disk
 * 
 * @param fd
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_sync(uvwasi_fd_t fd)
{
    return (uint32_t)uvwasi_fd_sync(&uvwasi, fd);
}

/**
 * Return the current offset of a file descriptor
 * 
 * @param fd
 * @param fileoffset_retptr The current offset of the file descriptor, relative to the start of the file.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_fd_tell(
    uvwasi_fd_t fd,
    uvwasi_size_t fileoffset_retptr)
{
    /* Check Bounds */
    check_bounds(fileoffset_retptr, UVWASI_SERDES_SIZE_filesize_t);

    /* Execute WASI Call */
    uvwasi_filesize_t fileoffset;
    uvwasi_errno_t rc = uvwasi_fd_tell(&uvwasi, fd, &fileoffset);
    if (rc == UVWASI_ESUCCESS)
        goto done;

    /* Write results to linear memory */
    uvwasi_serdes_write_filesize_t(memory, fileoffset_retptr, fileoffset);

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
uint32_t wasi_snapshot_preview1_fd_write(
    uvwasi_fd_t fd,
    uvwasi_size_t iovs_baseptr,
    uvwasi_size_t iovs_len,
    uvwasi_size_t nwritten_retptr)
{
    /* Check bounds */
    check_bounds(iovs_baseptr, UVWASI_SERDES_SIZE_ciovec_t * iovs_len);
    check_bounds(nwritten_retptr, UVWASI_SERDES_SIZE_size_t);

    /* Read iovec into memory */
    uvwasi_ciovec_t iovs[iovs_len];
    uvwasi_errno_t rc = uvwasi_serdes_readv_ciovec_t(memory, memory_size, iovs_baseptr, iovs, iovs_len);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Read iovec to target fd */
    uvwasi_size_t nwritten;
    rc = uvwasi_fd_write(&uvwasi,
                         fd,
                         iovs,
                         iovs_len,
                         &nwritten);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write nwritten back into linear memory */
    uvwasi_serdes_write_size_t(memory, nwritten_retptr, nwritten);

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
uint32_t wasi_snapshot_preview1_path_create_directory(
    uvwasi_fd_t fd,
    uvwasi_size_t path_baseptr,
    uvwasi_size_t path_len)
{
    check_bounds(path_baseptr, path_len);

    return (uint32_t)uvwasi_path_create_directory(&uvwasi,
                                                  fd,
                                                  &memory[path_baseptr],
                                                  path_len);
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
uint32_t wasi_snapshot_preview1_path_filestat_get(
    uvwasi_fd_t fd,
    uvwasi_lookupflags_t flags,
    uvwasi_size_t path_baseptr,
    uvwasi_size_t path_len,
    uvwasi_size_t filestat_retptr)
{
    check_bounds(path_baseptr, path_len);
    check_bounds(filestat_retptr, UVWASI_SERDES_SIZE_filestat_t);

    uvwasi_filestat_t stats;
    uvwasi_errno_t rc = uvwasi_path_filestat_get(&uvwasi,
                                                 fd,
                                                 flags,
                                                 &memory[path_baseptr],
                                                 path_len,
                                                 &stats);
    if (rc == UVWASI_ESUCCESS)
        goto done;

    uvwasi_serdes_write_filestat_t(memory, filestat_retptr, &stats);

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
uint32_t wasi_snapshot_preview1_path_filestat_set_times(
    uvwasi_fd_t fd,
    uvwasi_lookupflags_t flags,
    uvwasi_size_t path_baseptr,
    uvwasi_size_t path_len,
    uvwasi_timestamp_t atim,
    uvwasi_timestamp_t mtim,
    uint32_t fst_flags)
{
    uvwasi_errno_t rc;

    if (unlikely(fst_flags > UINT16_MAX))
        goto err_fst_flags_overflow;

    check_bounds(path_baseptr, path_len);

    return uvwasi_path_filestat_set_times(&uvwasi,
                                          fd,
                                          flags,
                                          &memory[path_baseptr],
                                          path_len,
                                          atim,
                                          mtim,
                                          (uvwasi_fstflags_t)fst_flags);

done:
    return (uint32_t)rc;
err_fst_flags_overflow:
    rc = UVWASI_EINVAL;
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
uint32_t wasi_snapshot_preview1_path_link(
    uvwasi_fd_t old_fd,
    uvwasi_lookupflags_t old_flags,
    uvwasi_size_t old_path_baseptr,
    uvwasi_size_t old_path_len,
    uvwasi_fd_t new_fd,
    uvwasi_size_t new_path_baseptr,
    uvwasi_size_t new_path_len)
{
    check_bounds(old_path_baseptr, old_path_len);
    check_bounds(new_path_baseptr, new_path_len);

    return (uint32_t)uvwasi_path_link(&uvwasi,
                                      old_fd,
                                      old_flags,
                                      &memory[old_path_baseptr],
                                      old_path_len,
                                      new_fd,
                                      &memory[new_path_baseptr],
                                      new_path_len);
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
uint32_t wasi_snapshot_preview1_path_open(
    uvwasi_fd_t dirfd,
    uvwasi_lookupflags_t lookupflags,
    uvwasi_size_t path_baseptr,
    uvwasi_size_t path_len,
    uint32_t oflags,
    uvwasi_rights_t fs_rights_base,
    uvwasi_rights_t fs_rights_inheriting,
    uint32_t fdflags,
    uvwasi_size_t fd_retptr)
{
    check_bounds(path_baseptr, path_len);
    check_bounds(fd_retptr, UVWASI_SERDES_SIZE_fd_t);

    if (unlikely(oflags > UINT16_MAX))
        goto err_oflags_overflow;
    if (unlikely(fdflags > UINT16_MAX))
        goto err_fdflags_overflow;

    uvwasi_fd_t fd;
    uvwasi_errno_t rc = uvwasi_path_open(&uvwasi,
                                         dirfd,
                                         lookupflags,
                                         &memory[path_baseptr],
                                         path_len,
                                         (uvwasi_oflags_t)oflags,
                                         fs_rights_base,
                                         fs_rights_inheriting,
                                         (uvwasi_fdflags_t)fdflags,
                                         &fd);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    uvwasi_serdes_write_size_t(memory, fd_retptr, fd);

done:
    return (uint32_t)rc;
err_oflags_overflow:
err_fdflags_overflow:
    rc = UVWASI_EINVAL;
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
uint32_t wasi_snapshot_preview1_path_readlink(
    uvwasi_fd_t fd,
    uvwasi_size_t path_baseptr,
    uvwasi_size_t path_len,
    uvwasi_size_t buf_baseretptr,
    uvwasi_size_t buf_len,
    uvwasi_size_t nread_retptr)
{
    check_bounds(path_baseptr, path_len);
    check_bounds(buf_baseretptr, buf_len);
    check_bounds(nread_retptr, UVWASI_SERDES_SIZE_size_t);

    uvwasi_size_t nread;
    uvwasi_errno_t rc = uvwasi_path_readlink(&uvwasi,
                                             fd,
                                             &memory[path_baseptr],
                                             path_len,
                                             &memory[buf_baseretptr],
                                             buf_len,
                                             &nread);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    uvwasi_serdes_write_size_t(memory, nread_retptr, nread);

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
uint32_t wasi_snapshot_preview1_path_remove_directory(
    uvwasi_fd_t fd,
    uvwasi_size_t path_baseptr,
    uvwasi_size_t path_len)
{
    check_bounds(path_baseptr, path_len);

    return (uint32_t)uvwasi_path_remove_directory(&uvwasi,
                                                  fd,
                                                  &memory[path_baseptr],
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
uint32_t wasi_snapshot_preview1_path_rename(
    uvwasi_fd_t fd,
    uvwasi_size_t old_path_baseptr,
    uvwasi_size_t old_path_len,
    uvwasi_fd_t new_fd,
    uvwasi_size_t new_path_baseptr,
    uvwasi_size_t new_path_len)
{
    check_bounds(old_path_baseptr, old_path_len);
    check_bounds(new_path_baseptr, new_path_len);

    return (uint32_t)uvwasi_path_rename(&uvwasi,
                                        fd,
                                        &memory[old_path_baseptr],
                                        old_path_len,
                                        new_fd,
                                        &memory[new_path_baseptr],
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
uint32_t wasi_snapshot_preview1_path_symlink(
    uvwasi_size_t old_path_baseptr,
    uvwasi_size_t old_path_len,
    uvwasi_fd_t fd,
    uvwasi_size_t new_path_baseptr,
    uvwasi_size_t new_path_len)
{
    check_bounds(old_path_baseptr, old_path_len);
    check_bounds(new_path_baseptr, new_path_len);

    return (uint32_t)uvwasi_path_symlink(&uvwasi,
                                         &memory[old_path_baseptr],
                                         old_path_len,
                                         fd,
                                         &memory[new_path_baseptr],
                                         new_path_len);
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
uint32_t wasi_snapshot_preview1_path_unlink_file(
    uvwasi_fd_t fd,
    uvwasi_size_t path_baseptr,
    uvwasi_size_t path_len)
{
    check_bounds(path_baseptr, path_len);

    return (uint32_t)uvwasi_path_unlink_file(&uvwasi,
                                             fd,
                                             &memory[path_baseptr],
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
uint32_t wasi_snapshot_preview1_poll_oneoff(
    uvwasi_size_t in_baseptr,
    uvwasi_size_t out_baseptr,
    uvwasi_size_t nsubscriptions,
    uvwasi_size_t nevents_retptr)
{
    check_bounds(in_baseptr,
                 nsubscriptions * UVWASI_SERDES_SIZE_subscription_t);
    check_bounds(out_baseptr,
                 nsubscriptions * UVWASI_SERDES_SIZE_event_t);
    check_bounds(nevents_retptr, UVWASI_SERDES_SIZE_size_t);

    uvwasi_subscription_t in[nsubscriptions];
    uvwasi_event_t out[nsubscriptions];

    /* Read in from linear memory */
    for (uvwasi_size_t i = 0; i < nsubscriptions; i++)
    {
        uvwasi_serdes_read_subscription_t(memory, in_baseptr, &in[i]);
        in_baseptr += UVWASI_SERDES_SIZE_subscription_t;
    }

    /* Call WASI syscall */
    uvwasi_size_t nevents;
    uvwasi_errno_t rc = uvwasi_poll_oneoff(&uvwasi,
                                           in,
                                           out,
                                           nsubscriptions,
                                           &nevents);
    if (rc != UVWASI_ESUCCESS)
        goto done;

    /* Write results to linear memory */
    uvwasi_serdes_write_size_t(memory, nevents_retptr, nevents);

    for (uvwasi_size_t i = 0; i < nsubscriptions; i++)
    {
        uvwasi_serdes_write_event_t(memory, out_baseptr, &out[i]);
        out_baseptr += UVWASI_SERDES_SIZE_event_t;
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
__attribute__((noreturn)) void wasi_snapshot_preview1_proc_exit(uvwasi_exitcode_t exitcode)
{
    uvwasi_proc_exit(&uvwasi, exitcode);
    assert(0);
}

/**
 * Send a signal to the process of the calling thread.
 * 
 * @param sig The signal condition to trigger.
 * @return status code
 */
uint32_t wasi_snapshot_preview1_proc_raise(
    uint32_t sig)
{
    uvwasi_errno_t rc;

    if (sig > UINT8_MAX)
        goto err_sig_overflow;

    rc = uvwasi_proc_raise(&uvwasi, (uvwasi_signal_t)sig);

done:
    return (uint32_t)rc;
err_sig_overflow:
    rc = UVWASI_EINVAL;
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
uint32_t wasi_snapshot_preview1_random_get(
    uvwasi_size_t buf_baseretptr,
    uvwasi_size_t buf_len)
{
    /* Check Bounds */
    check_bounds(buf_baseretptr, buf_len);

    /* Write random bytes directly to linear memory */
    return (uint32_t)uvwasi_random_get(&uvwasi, &memory[buf_baseretptr], buf_len);
}

/**
 * Temporarily yield execution of the calling thread similar to `sched_yield` in POSIX.
 * This implementation ignores client calls and silently returns RC 0
 * 
 * @return WASI_ESUCCESS
 */
uint32_t wasi_snapshot_preview1_sched_yield(void)
{
    return (uint32_t)uvwasi_sched_yield(&uvwasi);
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
uint32_t wasi_snapshot_preview1_sock_recv(
    uvwasi_fd_t fd,
    uvwasi_size_t ri_data_baseretptr,
    uvwasi_size_t ri_data_len,
    uint32_t ri_flags,
    uvwasi_size_t ri_data_nbytes_retptr,
    uvwasi_size_t message_nbytes_retptr)
{
    wasi_unsupported_syscall(__func__);

    /* ri_flags is type uvwasi_riflags_t, which is uint16_t and zero extended to an wasm i32 */
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
uint32_t wasi_snapshot_preview1_sock_send(
    uvwasi_fd_t fd,
    uvwasi_size_t si_data_baseptr,
    uvwasi_size_t si_data_len,
    uint32_t si_flags,
    uvwasi_size_t *retptr0)
{
    wasi_unsupported_syscall(__func__);

    /* si_flags is type uvwasi_siflags_t, which is uint16_t and zero extended to an wasm i32 */
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
uint32_t wasi_snapshot_preview1_sock_shutdown(
    uvwasi_fd_t fd,
    uint32_t how)
{
    wasi_unsupported_syscall(__func__);

    /* how is type uvwasi_sdflags_t, which is uint8_t and zero extended to an wasm i32 */
}
