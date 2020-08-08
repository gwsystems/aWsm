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

int main(int argc, char* argv[]) {
    runtime_main(argc, argv);
    printf("mem use = %d\n", (int) memory_size);
}

// The symbol the binary gives us to init libc
extern void wasmf___wasm_call_ctors(void);

// Called before wasmf_main
void stub_init() {
    switch_out_of_runtime();
    wasmf___wasm_call_ctors();
    switch_into_runtime();
}


// WASI API implementations

// some definitions we need
struct wasi_iovec {
    i32 base_offset;
    i32 len;
};

struct wasi_fdstat {
    u8  fs_filetype;
    u16 fs_flags;
    u64 fs_rights_base;
    u64 fs_rights_inheriting;
};

struct wasi_filestat {
    u64 st_dev;
    u64 st_ino;
    u8 st_filetype;
    u32 st_nlink;
    u64 st_size;
    u64 st_atim;
    u64 st_mtim;
    u64 st_ctim;
};

enum wasi_filetype {
    WASI_FILETYPE_UNKNOWN           = 0,
    WASI_FILETYPE_BLOCK_DEVICE      = 1,
    WASI_FILETYPE_CHARACTER_DEVICE  = 2,
    WASI_FILETYPE_DIRECTORY         = 3,
    WASI_FILETYPE_REGULAR_FILE      = 4,
    WASI_FILETYPE_SOCKET_DGRAM      = 5,
    WASI_FILETYPE_SOCKET_STREAM     = 6,
    WASI_FILETYPE_SYMBOLIC_LINK     = 7,
};

enum wasi_oflags {
    WASI_O_CREAT        = 0x0001,
    WASI_O_DIRECTORY    = 0x0002,
    WASI_O_EXCL         = 0x0004,
    WASI_O_TRUNC        = 0x0008,
};

enum wasi_fdflags {
    WASI_FDFLAG_APPEND      = 0x0001,
    WASI_FDFLAG_DSYNC       = 0x0002,
    WASI_FDFLAG_NONBLOCK    = 0x0004,
    WASI_FDFLAG_RSYNC       = 0x0008,
    WASI_FDFLAG_SYNC        = 0x0010,
};

enum wasi_rights {
    WASI_RIGHT_FD_DATASYNC              = 0x0000000000000001ULL,
    WASI_RIGHT_FD_READ                  = 0x0000000000000002ULL,
    WASI_RIGHT_FD_SEEK                  = 0x0000000000000004ULL,
    WASI_RIGHT_FD_FDSTAT_SET_FLAGS      = 0x0000000000000008ULL,
    WASI_RIGHT_FD_SYNC                  = 0x0000000000000010ULL,
    WASI_RIGHT_FD_TELL                  = 0x0000000000000020ULL,
    WASI_RIGHT_FD_WRITE                 = 0x0000000000000040ULL,
    WASI_RIGHT_FD_ADVISE                = 0x0000000000000080ULL,
    WASI_RIGHT_FD_ALLOCATE              = 0x0000000000000100ULL,
    WASI_RIGHT_PATH_CREATE_DIRECTORY    = 0x0000000000000200ULL,
    WASI_RIGHT_PATH_CREATE_FILE         = 0x0000000000000400ULL,
    WASI_RIGHT_PATH_LINK_SOURCE         = 0x0000000000000800ULL,
    WASI_RIGHT_PATH_LINK_TARGET         = 0x0000000000001000ULL,
    WASI_RIGHT_PATH_OPEN                = 0x0000000000002000ULL,
    WASI_RIGHT_FD_READDIR               = 0x0000000000004000ULL,
    WASI_RIGHT_PATH_READLINK            = 0x0000000000008000ULL,
    WASI_RIGHT_PATH_RENAME_SOURCE       = 0x0000000000010000ULL,
    WASI_RIGHT_PATH_RENAME_TARGET       = 0x0000000000020000ULL,
    WASI_RIGHT_PATH_FILESTAT_GET        = 0x0000000000040000ULL,
    WASI_RIGHT_PATH_FILESTAT_SET_SIZE   = 0x0000000000080000ULL,
    WASI_RIGHT_PATH_FILESTAT_SET_TIMES  = 0x0000000000100000ULL,
    WASI_RIGHT_FD_FILESTAT_GET          = 0x0000000000200000ULL,
    WASI_RIGHT_FD_FILESTAT_SET_SIZE     = 0x0000000000400000ULL,
    WASI_RIGHT_FD_FILESTAT_SET_TIMES    = 0x0000000000800000ULL,
    WASI_RIGHT_PATH_SYMLINK             = 0x0000000001000000ULL,
    WASI_RIGHT_PATH_REMOVE_DIRECTORY    = 0x0000000002000000ULL,
    WASI_RIGHT_PATH_UNLINK_FILE         = 0x0000000004000000ULL,
    WASI_RIGHT_POLL_FD_READWRITE        = 0x0000000008000000ULL,
    WASI_RIGHT_SOCK_SHUTDOWN            = 0x0000000010000000ULL,
};


// file operations
i32 wasi_unstable_path_open(
        i32 dirfd,
        u32 lookupflags,
        u32 path_off,
        u32 path_len,
        u32 oflags,
        u64 fs_rights_base,
        u64 fs_rights_inheriting,
        u32 fdflags,
        u32 fd_off) {
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
        return -errno;
    }

    set_i32(fd_off, fd);
    return 0;
}

i32 wasi_unstable_fd_close(i32 fd) {
    i32 res = (i32) close(fd);

    if (res == -1) {
        return -errno;
    }
    return res;
}

i32 wasi_unstable_fd_fdstat_get(i32 fd, u32 buf_offset) {
    struct wasi_fdstat* fdstat = get_memory_ptr_void(buf_offset, sizeof(struct wasi_fdstat));

    struct stat stat;
    i32 res = fstat(fd, &stat);
    if (res == -1) {
        return -errno;
    }
    int mode = stat.st_mode;

    i32 fl = fcntl(fd, F_GETFL);
    if (fl < 0) {
        return -errno;
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

    return 0;
}

i32 wasi_unstable_fd_fdstat_set_flags(i32 fd, u32 fdflags) {
    int flags = (
        ((flags & WASI_FDFLAG_APPEND  ) ? O_APPEND   : 0) |
        ((flags & WASI_FDFLAG_DSYNC   ) ? O_DSYNC    : 0) |
        ((flags & WASI_FDFLAG_NONBLOCK) ? O_NONBLOCK : 0) |
        ((flags & WASI_FDFLAG_RSYNC   ) ? O_RSYNC    : 0) |
        ((flags & WASI_FDFLAG_SYNC    ) ? O_SYNC     : 0));
    int err = fcntl(fd, F_SETFL, fdflags);
    if (err < 0) {
        return -errno;
    }
    return 0;
}

i32 wasi_unstable_fd_seek(i32 fd, i64 file_offset, i32 whence, u32 newoffset_off) {
    off_t res = lseek(fd, (off_t)file_offset, whence);

    if (res == -1) {
        return -errno;
    }

    set_i64(newoffset_off, res);
    return 0;
}

i32 wasi_unstable_fd_datasync(i32 fd) {
    int res = fdatasync(fd);
    if (res == -1) {
        return -errno;
    }

    return 0;
}

i32 wasi_unstable_fd_read(i32 fd, i32 iov_offset, i32 iovcnt, i32 nread_off) {
    i32 sum = 0;
    struct wasi_iovec *iov = get_memory_ptr_void(iov_offset, iovcnt * sizeof(struct wasi_iovec));

    for (int i = 0; i < iovcnt; i++) {
        void* ptr = get_memory_ptr_void(iov[i].base_offset, iov[i].len);
        ssize_t res = read(fd, ptr, iov[i].len);
        if (res == -1) {
            return -errno;
        }

        sum += res;
    }

    set_i32(nread_off, sum);
    return 0;
}

i32 wasi_unstable_fd_write(i32 fd, i32 iov_offset, i32 iovcnt, i32 nwritten_off) {
    i32 sum = 0;
    struct wasi_iovec *iov = get_memory_ptr_void(iov_offset, iovcnt * sizeof(struct wasi_iovec));

    for (int i = 0; i < iovcnt; i++) {
        void* ptr = get_memory_ptr_void(iov[i].base_offset, iov[i].len);
        ssize_t res = write(fd, ptr, iov[i].len);
        if (res == -1) {
            return -errno;
        }

        sum += res;
    }

    set_i32(nwritten_off, sum);
    return 0;
}

// other filesystem operations
i32 wasi_unstable_path_filestat_get(i32 fd, u32 flags, u32 path_off, u32 path_len, u32 buf_off) {
    // get path/filestat
    char* path = get_memory_string(path_off);
    struct wasi_filestat* filestat = get_memory_ptr_void(buf_off, sizeof(struct wasi_filestat));

    struct stat stat;
    int res = fstatat(fd, path, &stat, 0);
    if (res == -1) {
        return -errno;
    }

    filestat->st_dev = stat.st_dev;
    filestat->st_ino = stat.st_ino;
    filestat->st_filetype =  stat.st_mode;
    filestat->st_nlink = stat.st_nlink;
    filestat->st_size = stat.st_size;
    filestat->st_atim = stat.st_atime;
    filestat->st_mtim = stat.st_mtime;
    filestat->st_ctim = stat.st_ctime;

    return 0;
}

i32 wasi_unstable_path_unlink_file(i32 fd, u32 path_off, u32 path_len) {
    // get path
    char* path = get_memory_string(path_off);

    int res = unlinkat(fd, path, 0);
    if (res == -1) {
        return -errno;
    }

    return 0;
}

i32 wasi_unstable_path_create_directory(i32 fd, u32 path_off, u32 path_len) {
    // get path
    char* path = get_memory_string(path_off);

    int res = mkdirat(fd, path, 0777);
    if (res == -1) {
        return -errno;
    }

    return 0;
}

// clock operations
i32 wasi_unstable_clock_time_get(u32 clock_id, u64 precision, u32 time_off) {
    struct timespec tp;
    clock_gettime(clock_id, &tp);
    set_i64(time_off, (uint64_t)tp.tv_sec*1000000000ULL + (uint64_t)tp.tv_nsec);
    return 0;
}

// process operations
__attribute__((noreturn))
void wasi_unstable_proc_exit(i32 exitcode) {
    exit(exitcode);
}

