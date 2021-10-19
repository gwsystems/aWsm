#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <uvwasi.h>

#include "./wasi_impl.h"

/* Return abstract handle */
void* wasi_context_init(wasi_options_t* options) {
    uvwasi_t* wasi_context = malloc(sizeof(uvwasi_t));

    uvwasi_options_t init_options;
    uvwasi_options_init(&init_options);

    /*
     * Pass through arguments from host process
     * The name of the executable is passed as the first arg.
     * This truncates the relative path from the caller's host pwd
     */
    init_options.argc = options->argc;
    init_options.argv = calloc(options->argc, sizeof(char*));
    int i;
    for (i = strlen(options->argv[0]); i > 0; i--) {
        if (options->argv[0][i] == '/') {
            i++;
            break;
        }
    }
    init_options.argv[0] = &options->argv[0][i];

    for (int i = 1; i < options->argc; i++) {
        init_options.argv[i] = options->argv[i];
    }

    /* Pass through environment from host process */
    init_options.envp = options->envp;

    /* Mount local directory as /home */
    init_options.preopenc                = 1;
    init_options.preopens                = calloc(1, sizeof(uvwasi_preopen_t));
    init_options.preopens[0].mapped_path = "/sandbox";
    init_options.preopens[0].real_path   = ".";

    /* Initialize the sandbox */
    uvwasi_errno_t err = uvwasi_init(wasi_context, &init_options);

    free(init_options.argv);
    free(init_options.preopens);
    awsm_assert(err == UVWASI_ESUCCESS);

    return wasi_context;
}

void wasi_context_destroy(void* wasi_context) {
    uvwasi_destroy((uvwasi_t*)wasi_context);
};

__wasi_size_t wasi_context_get_argc(void* wasi_context) {
    return ((uvwasi_t*)wasi_context)->argc;
}


char** wasi_context_get_argv(void* wasi_context) {
    return ((uvwasi_t*)wasi_context)->argv;
}

__wasi_size_t wasi_context_get_argv_buf_size(void* wasi_context) {
    return ((uvwasi_t*)wasi_context)->argv_buf_size;
}

__wasi_size_t wasi_context_get_envc(void* wasi_context) {
    return ((uvwasi_t*)wasi_context)->envc;
}

__wasi_size_t wasi_context_get_env_buf_size(void* wasi_context) {
    return ((uvwasi_t*)wasi_context)->env_buf_size;
}

_Static_assert(sizeof(__wasi_errno_t) == sizeof(uvwasi_errno_t), "uvwasi_errno_t");
_Static_assert(sizeof(__wasi_size_t) == sizeof(uvwasi_size_t), "uvwasi_size_t");
_Static_assert(sizeof(__wasi_clockid_t) == sizeof(uvwasi_clockid_t), "uvwasi_clockid_t");
_Static_assert(sizeof(__wasi_timestamp_t) == sizeof(uvwasi_timestamp_t), "uvwasi_timestamp_t");
_Static_assert(sizeof(__wasi_fd_t) == sizeof(uvwasi_fd_t), "uvwasi_fd_t");
_Static_assert(sizeof(__wasi_filesize_t) == sizeof(uvwasi_filesize_t), "uvwasi_filesize_t");
_Static_assert(sizeof(__wasi_advice_t) == sizeof(uvwasi_advice_t), "uvwasi_advice_t");
_Static_assert(sizeof(__wasi_fdstat_t) == sizeof(uvwasi_fdstat_t), "uvwasi_fdstat_t");
_Static_assert(sizeof(__wasi_fdflags_t) == sizeof(uvwasi_fdflags_t), "uvwasi_fdflags_t");
_Static_assert(sizeof(__wasi_rights_t) == sizeof(uvwasi_rights_t), "uvwasi_rights_t");
_Static_assert(sizeof(__wasi_filestat_t) == sizeof(uvwasi_filestat_t), "uvwasi_filestat_t");
_Static_assert(sizeof(__wasi_iovec_t) == sizeof(uvwasi_iovec_t), "uvwasi_iovec_t");
_Static_assert(sizeof(__wasi_ciovec_t) == sizeof(uvwasi_ciovec_t), "uvwasi_ciovec_t");
_Static_assert(sizeof(__wasi_prestat_t) == sizeof(uvwasi_prestat_t), "uvwasi_prestat_t");
_Static_assert(sizeof(__wasi_dircookie_t) == sizeof(uvwasi_dircookie_t), "uvwasi_dircookie_t");
_Static_assert(sizeof(__wasi_filedelta_t) == sizeof(uvwasi_filedelta_t), "uvwasi_filedelta_t");
_Static_assert(sizeof(__wasi_whence_t) == sizeof(uvwasi_whence_t), "uvwasi_whence_t");
_Static_assert(sizeof(__wasi_lookupflags_t) == sizeof(uvwasi_lookupflags_t), "uvwasi_lookupflags_t");
_Static_assert(sizeof(__wasi_fstflags_t) == sizeof(uvwasi_fstflags_t), "uvwasi_fstflags_t");
_Static_assert(sizeof(__wasi_oflags_t) == sizeof(uvwasi_oflags_t), "uvwasi_oflags_t");
_Static_assert(sizeof(__wasi_subscription_t) == sizeof(uvwasi_subscription_t), "uvwasi_subscription_t");
_Static_assert(sizeof(__wasi_event_t) == sizeof(uvwasi_event_t), "uvwasi_event_t");
_Static_assert(sizeof(__wasi_exitcode_t) == sizeof(uvwasi_exitcode_t), "uvwasi_exitcode_t");
_Static_assert(sizeof(__wasi_signal_t) == sizeof(uvwasi_signal_t), "uvwasi_signal_t");
_Static_assert(sizeof(__wasi_riflags_t) == sizeof(uvwasi_riflags_t), "uvwasi_riflags_t");
_Static_assert(sizeof(__wasi_roflags_t) == sizeof(uvwasi_roflags_t), "uvwasi_roflags_t");
_Static_assert(sizeof(__wasi_siflags_t) == sizeof(uvwasi_siflags_t), "uvwasi_siflags_t");
_Static_assert(sizeof(__wasi_sdflags_t) == sizeof(uvwasi_sdflags_t), "uvwasi_sdflags_t");

__wasi_errno_t wasi_snapshot_preview1_backing_args_get(void* wasi_context, char** argv, char* argv_buf) {
    // printf("glue: %s\n", ((uvwasi_t*)wasi_context)->argv_buf);
    return uvwasi_args_get((uvwasi_t*)wasi_context, argv, argv_buf);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_args_sizes_get(void* wasi_context, __wasi_size_t* retptr0, __wasi_size_t* retptr1) {
    return uvwasi_args_sizes_get((uvwasi_t*)wasi_context, retptr0, retptr1);
}

__wasi_errno_t wasi_snapshot_preview1_backing_environ_get(void* wasi_context, char** environ, char* environ_buf) {
    return uvwasi_environ_get((uvwasi_t*)wasi_context, environ, environ_buf);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_environ_sizes_get(void* wasi_context, __wasi_size_t* retptr0, __wasi_size_t* retptr1) {
    return uvwasi_environ_sizes_get((uvwasi_t*)wasi_context, retptr0, retptr1);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_clock_res_get(void* wasi_context, __wasi_clockid_t id, __wasi_timestamp_t* retptr0) {
    return uvwasi_clock_res_get((uvwasi_t*)wasi_context, id, retptr0);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_clock_time_get(void* wasi_context, __wasi_clockid_t id, __wasi_timestamp_t precision,
                                              __wasi_timestamp_t* retptr0) {
    return uvwasi_clock_time_get((uvwasi_t*)wasi_context, id, precision, retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_advise(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t offset,
                                                        __wasi_filesize_t len, __wasi_advice_t advice) {
    return uvwasi_fd_advise((uvwasi_t*)wasi_context, fd, offset, len, advice);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_allocate(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t offset,
                                                          __wasi_filesize_t len) {
    return uvwasi_fd_allocate((uvwasi_t*)wasi_context, fd, offset, len);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_close(void* wasi_context, __wasi_fd_t fd) {
    return uvwasi_fd_close((uvwasi_t*)wasi_context, fd);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_datasync(void* wasi_context, __wasi_fd_t fd) {
    return uvwasi_fd_datasync((uvwasi_t*)wasi_context, fd);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_fdstat_get(void* wasi_context, __wasi_fd_t fd, __wasi_fdstat_t* retptr0) {
    return uvwasi_fd_fdstat_get((uvwasi_t*)wasi_context, fd, (uvwasi_fdstat_t*)retptr0);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fdstat_set_flags(void* wasi_context, __wasi_fd_t fd, __wasi_fdflags_t flags) {
    return uvwasi_fd_fdstat_set_flags((uvwasi_t*)wasi_context, fd, flags);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fdstat_set_rights(void* wasi_context, __wasi_fd_t fd, __wasi_rights_t fs_rights_base,
                                                 __wasi_rights_t fs_rights_inheriting) {
    return uvwasi_fd_fdstat_set_rights((uvwasi_t*)wasi_context, fd, fs_rights_base, fs_rights_inheriting);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_filestat_get(void* wasi_context, __wasi_fd_t fd, __wasi_filestat_t* retptr0) {
    return uvwasi_fd_filestat_get((uvwasi_t*)wasi_context, fd, (uvwasi_filestat_t*)retptr0);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_filestat_set_size(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t size) {
    return uvwasi_fd_filestat_set_size((uvwasi_t*)wasi_context, fd, size);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_filestat_set_times(void* wasi_context, __wasi_fd_t fd, __wasi_timestamp_t atim,
                                                  __wasi_timestamp_t mtim, __wasi_fstflags_t fst_flags) {
    return uvwasi_fd_filestat_set_times((uvwasi_t*)wasi_context, fd, atim, mtim, fst_flags);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_pread(void* wasi_context, __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len,
                                        __wasi_filesize_t offset, __wasi_size_t* retptr0) {
    return uvwasi_fd_pread((uvwasi_t*)wasi_context, fd, (uvwasi_iovec_t*)iovs, iovs_len, offset, retptr0);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_prestat_get(void* wasi_context, __wasi_fd_t fd, __wasi_prestat_t* retptr0) {
    return uvwasi_fd_prestat_get((uvwasi_t*)wasi_context, fd, (uvwasi_prestat_t*)retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_prestat_dir_name(void* wasi_context, __wasi_fd_t fd, char* path,
                                                                  __wasi_size_t path_len) {
    return uvwasi_fd_prestat_dir_name((uvwasi_t*)wasi_context, fd, path, path_len);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_pwrite(void* wasi_context, __wasi_fd_t fd, const __wasi_ciovec_t* iovs,
                                         size_t iovs_len, __wasi_filesize_t offset, __wasi_size_t* retptr0) {
    return uvwasi_fd_pwrite((uvwasi_t*)wasi_context, fd, (const uvwasi_ciovec_t*)iovs, iovs_len, offset, retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_read(void* wasi_context, __wasi_fd_t fd, const __wasi_iovec_t* iovs,
                                                      size_t iovs_len, __wasi_size_t* retptr0) {
    return uvwasi_fd_read((uvwasi_t*)wasi_context, fd, (const uvwasi_iovec_t*)iovs, iovs_len, retptr0);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_fd_readdir(void* wasi_context, __wasi_fd_t fd, uint8_t* buf, __wasi_size_t buf_len,
                                          __wasi_dircookie_t cookie, __wasi_size_t* retptr0) {
    return uvwasi_fd_readdir((uvwasi_t*)wasi_context, fd, buf, buf_len, cookie, retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_renumber(void* wasi_context, __wasi_fd_t fd, __wasi_fd_t to) {
    return uvwasi_fd_renumber((uvwasi_t*)wasi_context, fd, to);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_seek(void* wasi_context, __wasi_fd_t fd, __wasi_filedelta_t offset,
                                                      __wasi_whence_t whence, __wasi_filesize_t* retptr0) {
    return uvwasi_fd_seek((uvwasi_t*)wasi_context, fd, offset, whence, retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_sync(void* wasi_context, __wasi_fd_t fd) {
    return uvwasi_fd_sync((uvwasi_t*)wasi_context, fd);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_tell(void* wasi_context, __wasi_fd_t fd, __wasi_filesize_t* retptr0) {
    return uvwasi_fd_tell((uvwasi_t*)wasi_context, fd, retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_fd_write(void* wasi_context, __wasi_fd_t fd, const __wasi_ciovec_t* iovs,
                                                       size_t iovs_len, __wasi_size_t* nwritten_retptr) {
    return uvwasi_fd_write((uvwasi_t*)wasi_context, fd, (const uvwasi_ciovec_t*)iovs, iovs_len, nwritten_retptr);
}

__wasi_errno_t wasi_snapshot_preview1_backing_path_create_directory(void* wasi_context, __wasi_fd_t fd,
                                                                    const char* path, __wasi_size_t path_len) {
    return uvwasi_path_create_directory((uvwasi_t*)wasi_context, fd, path, path_len);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_path_filestat_get(void* wasi_context, __wasi_fd_t fd, __wasi_lookupflags_t flags,
                                                 const char* path, __wasi_size_t path_len, __wasi_filestat_t* retptr0) {
    return uvwasi_path_filestat_get((uvwasi_t*)wasi_context, fd, flags, path, path_len, (uvwasi_filestat_t*)retptr0);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_path_filestat_set_times(void* wasi_context, __wasi_fd_t fd, __wasi_lookupflags_t flags,
                                                       const char* path, __wasi_size_t path_len,
                                                       __wasi_timestamp_t atim, __wasi_timestamp_t mtim,
                                                       __wasi_fstflags_t fst_flags) {
    return wasi_snapshot_preview1_backing_path_filestat_set_times((uvwasi_t*)wasi_context, fd, flags, path, path_len,
                                                                  atim, mtim, fst_flags);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_path_link(void* wasi_context, __wasi_fd_t old_fd, __wasi_lookupflags_t old_flags,
                                         const char* old_path, __wasi_size_t old_path_len, __wasi_fd_t new_fd,
                                         const char* new_path, __wasi_size_t new_path_len) {
    return uvwasi_path_link((uvwasi_t*)wasi_context, old_fd, old_flags, old_path, old_path_len, new_fd, new_path,
                            new_path_len);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_path_open(void* wasi_context, __wasi_fd_t fd, __wasi_lookupflags_t dirflags,
                                         const char* path, __wasi_size_t path_len, __wasi_oflags_t oflags,
                                         __wasi_rights_t fs_rights_base, __wasi_rights_t fs_rights_inheriting,
                                         __wasi_fdflags_t fdflags, __wasi_fd_t* retptr0) {
    return uvwasi_path_open((uvwasi_t*)wasi_context, fd, dirflags, path, path_len, oflags, fs_rights_base,
                            fs_rights_inheriting, fdflags, (uvwasi_fd_t*)retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_path_readlink(void* wasi_context, __wasi_fd_t fd, const char* path,
                                                            __wasi_size_t path_len, uint8_t* buf, __wasi_size_t buf_len,
                                                            __wasi_size_t* retptr0) {
    return uvwasi_path_readlink((uvwasi_t*)wasi_context, fd, path, path_len, (char*)buf, buf_len, retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_path_remove_directory(void* wasi_context, __wasi_fd_t fd,
                                                                    const char* path, __wasi_size_t path_len) {
    return uvwasi_path_remove_directory((uvwasi_t*)wasi_context, fd, path, path_len);
}

__wasi_errno_t wasi_snapshot_preview1_backing_path_rename(void* wasi_context, __wasi_fd_t fd, const char* old_path,
                                                          __wasi_size_t old_path_len, __wasi_fd_t new_fd,
                                                          const char* new_path, __wasi_size_t new_path_len) {
    return uvwasi_path_rename((uvwasi_t*)wasi_context, fd, old_path, old_path_len, new_fd, new_path, new_path_len);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_path_symlink(void* wasi_context, const char* old_path, __wasi_size_t old_path_len,
                                            __wasi_fd_t fd, const char* new_path, __wasi_size_t new_path_len) {
    return uvwasi_path_symlink((uvwasi_t*)wasi_context, old_path, old_path_len, fd, new_path, new_path_len);
}

__wasi_errno_t wasi_snapshot_preview1_backing_path_unlink_file(void* wasi_context, __wasi_fd_t fd, const char* path,
                                                               __wasi_size_t path_len) {
    return uvwasi_path_unlink_file((uvwasi_t*)wasi_context, fd, path, path_len);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_poll_oneoff(void* wasi_context, const __wasi_subscription_t* in, __wasi_event_t* out,
                                           __wasi_size_t nsubscriptions, __wasi_size_t* retptr0) {
    return uvwasi_poll_oneoff((uvwasi_t*)wasi_context, (uvwasi_subscription_t*)in, (uvwasi_event_t*)out, nsubscriptions,
                              retptr0);
}

_Noreturn void wasi_snapshot_preview1_backing_proc_exit(void* wasi_context, __wasi_exitcode_t rval) {
    uvwasi_proc_exit((uvwasi_t*)wasi_context, rval);
    awsm_assert(0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_proc_raise(void* wasi_context, __wasi_signal_t sig) {
    return uvwasi_proc_raise((uvwasi_t*)wasi_context, sig);
}

__wasi_errno_t wasi_snapshot_preview1_backing_sched_yield(void* wasi_context) {
    return uvwasi_sched_yield((uvwasi_t*)wasi_context);
}

__wasi_errno_t wasi_snapshot_preview1_backing_random_get(void* wasi_context, uint8_t* buf, __wasi_size_t buf_len) {
    return uvwasi_random_get((uvwasi_t*)wasi_context, buf, buf_len);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_sock_recv(void* wasi_context, __wasi_fd_t fd, const __wasi_iovec_t* ri_data,
                                         size_t ri_data_len, __wasi_riflags_t ri_flags, __wasi_size_t* retptr0,
                                         __wasi_roflags_t* retptr1) {
    return uvwasi_sock_recv((uvwasi_t*)wasi_context, fd, (uvwasi_iovec_t*)ri_data, ri_data_len, ri_flags, retptr0,
                            retptr1);
}

__wasi_errno_t
wasi_snapshot_preview1_backing_sock_send(void* wasi_context, __wasi_fd_t fd, const __wasi_ciovec_t* si_data,
                                         size_t si_data_len, __wasi_siflags_t si_flags, __wasi_size_t* retptr0) {
    return uvwasi_sock_send((uvwasi_t*)wasi_context, fd, (const uvwasi_ciovec_t*)si_data, si_data_len, si_flags,
                            retptr0);
}

__wasi_errno_t wasi_snapshot_preview1_backing_sock_shutdown(void* wasi_context, __wasi_fd_t fd, __wasi_sdflags_t how) {
    return uvwasi_sock_shutdown((uvwasi_t*)wasi_context, fd, how);
}
