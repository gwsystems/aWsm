#ifndef __WASI_SERDES_H__
#define __WASI_SERDES_H__

// TODO: Add Node.js license

#include "wasi_spec.h"

/* Basic uint{8,16,32,64}_t read/write functions. */

#define BASIC_TYPE(name, type)                                           \
    static inline void wasi_serdes_write_##name(void* ptr, size_t offset, type value); \
    static inline type wasi_serdes_read_##name(const void* ptr, size_t offset);

#define BASIC_TYPE_WASI(type) BASIC_TYPE(type, __wasi_##type)

#define WASI_SERDES_SIZE_uint8_t sizeof(uint8_t)
BASIC_TYPE(uint8_t, uint8_t)
#define WASI_SERDES_SIZE_uint16_t sizeof(uint16_t)
BASIC_TYPE(uint16_t, uint16_t)
#define WASI_SERDES_SIZE_uint32_t sizeof(uint32_t)
BASIC_TYPE(uint32_t, uint32_t)
#define WASI_SERDES_SIZE_uint64_t sizeof(uint64_t)
BASIC_TYPE(uint64_t, uint64_t)

#define WASI_SERDES_SIZE_advice_t sizeof(__wasi_advice_t)
BASIC_TYPE_WASI(advice_t)
#define WASI_SERDES_SIZE_clockid_t sizeof(__wasi_clockid_t)
BASIC_TYPE_WASI(clockid_t)
#define WASI_SERDES_SIZE_device_t sizeof(__wasi_device_t)
BASIC_TYPE_WASI(device_t)
#define WASI_SERDES_SIZE_dircookie_t sizeof(__wasi_dircookie_t)
BASIC_TYPE_WASI(dircookie_t)
#define WASI_SERDES_SIZE_eventrwflags_t sizeof(__wasi_eventrwflags_t)
BASIC_TYPE_WASI(eventrwflags_t)
#define WASI_SERDES_SIZE_eventtype_t sizeof(__wasi_eventtype_t)
BASIC_TYPE_WASI(eventtype_t)
#define WASI_SERDES_SIZE_exitcode_t sizeof(__wasi_exitcode_t)
BASIC_TYPE_WASI(exitcode_t)
#define WASI_SERDES_SIZE_fd_t sizeof(__wasi_fd_t)
BASIC_TYPE_WASI(fd_t)
#define WASI_SERDES_SIZE_fdflags_t sizeof(__wasi_fdflags_t)
BASIC_TYPE_WASI(fdflags_t)
#define WASI_SERDES_SIZE_filesize_t sizeof(__wasi_filesize_t)
BASIC_TYPE_WASI(filesize_t)
#define WASI_SERDES_SIZE_fstflags_t sizeof(__wasi_fstflags_t)
BASIC_TYPE_WASI(fstflags_t)
#define WASI_SERDES_SIZE_inode_t sizeof(__wasi_inode_t)
BASIC_TYPE_WASI(inode_t)
#define WASI_SERDES_SIZE_linkcount_t sizeof(__wasi_linkcount_t)
BASIC_TYPE_WASI(linkcount_t)
#define WASI_SERDES_SIZE_lookupflags_t sizeof(__wasi_lookupflags_t)
BASIC_TYPE_WASI(lookupflags_t)
#define WASI_SERDES_SIZE_oflags_t sizeof(__wasi_oflags_t)
BASIC_TYPE_WASI(oflags_t)
#define WASI_SERDES_SIZE_preopentype_t sizeof(__wasi_preopentype_t)
BASIC_TYPE_WASI(preopentype_t)
#define WASI_SERDES_SIZE_riflags_t sizeof(__wasi_riflags_t)
BASIC_TYPE_WASI(riflags_t)
#define WASI_SERDES_SIZE_rights_t sizeof(__wasi_rights_t)
BASIC_TYPE_WASI(rights_t)
#define WASI_SERDES_SIZE_roflags_t sizeof(__wasi_roflags_t)
BASIC_TYPE_WASI(roflags_t)
#define WASI_SERDES_SIZE_sdflags_t sizeof(__wasi_sdflags_t)
BASIC_TYPE_WASI(sdflags_t)
#define WASI_SERDES_SIZE_siflags_t sizeof(__wasi_siflags_t)
BASIC_TYPE_WASI(siflags_t)
#define WASI_SERDES_SIZE_size_t sizeof(__wasi_size_t)
BASIC_TYPE_WASI(size_t)
#define WASI_SERDES_SIZE_inode_t sizeof(__wasi_inode_t)
BASIC_TYPE_WASI(inode_t)
#define WASI_SERDES_SIZE_signal_t sizeof(__wasi_signal_t)
BASIC_TYPE_WASI(signal_t)
#define WASI_SERDES_SIZE_subclockflags_t sizeof(__wasi_subclockflags_t)
BASIC_TYPE_WASI(subclockflags_t)
#define WASI_SERDES_SIZE_timestamp_t sizeof(__wasi_timestamp_t)
BASIC_TYPE_WASI(timestamp_t)
#define WASI_SERDES_SIZE_userdata_t sizeof(__wasi_userdata_t)
BASIC_TYPE_WASI(userdata_t)
#define WASI_SERDES_SIZE_whence_t sizeof(__wasi_whence_t)
BASIC_TYPE_WASI(whence_t)

#undef BASIC_TYPE_WASI
#undef BASIC_TYPE

/* WASI structure read/write functions. */

#define STRUCT(name)                                                                     \
    void wasi_serdes_write_##name(void* ptr, size_t offset, const __wasi_##name* value); \
    void wasi_serdes_read_##name(const void* ptr, size_t offset, __wasi_##name* value);

/* iovs currently only need to be read from WASM memory. */
#define IOVS_STRUCT(name) \
    static inline __wasi_errno_t wasi_serdes_read_##name(const void* ptr, size_t end, size_t offset, __wasi_##name* value);

#define WASI_SERDES_SIZE_ciovec_t 8
IOVS_STRUCT(ciovec_t)

#define WASI_SERDES_SIZE_iovec_t 8
IOVS_STRUCT(iovec_t)

#define WASI_SERDES_SIZE_dirent_t 24
STRUCT(dirent_t)

#define WASI_SERDES_SIZE_fdstat_t 24
STRUCT(fdstat_t)

#define WASI_SERDES_SIZE_filestat_t 64
STRUCT(filestat_t)

#define WASI_SERDES_SIZE_prestat_t 8
STRUCT(prestat_t)

#define WASI_SERDES_SIZE_event_t 32
STRUCT(event_t)

#define WASI_SERDES_SIZE_subscription_t 48
STRUCT(subscription_t)

#undef STRUCT
#undef IOVS_STRUCT

static inline __wasi_errno_t
wasi_serdes_readv_ciovec_t(const void* ptr, size_t end, size_t offset, __wasi_ciovec_t* iovs, __wasi_size_t iovs_len);

static inline __wasi_errno_t
wasi_serdes_readv_iovec_t(const void* ptr, size_t end, size_t offset, __wasi_iovec_t* iovs, __wasi_size_t iovs_len);

/* Helper functions for memory bounds checking. */
static inline int wasi_serdes_check_bounds(size_t offset, size_t end, size_t size);
static inline int wasi_serdes_check_array_bounds(size_t offset, size_t end, size_t size, size_t count);

static inline int wasi_serdes_check_bounds(size_t offset, size_t end, size_t size) {
    return end > offset && size <= (end - offset);
}

static inline void wasi_serdes_write_uint64_t(void* ptr, size_t offset, uint64_t value) {
    wasi_serdes_write_uint32_t(ptr, offset, (uint32_t)value);
    wasi_serdes_write_uint32_t(ptr, offset + 4, value >> 32);
}

static inline void wasi_serdes_write_uint32_t(void* ptr, size_t offset, uint32_t value) {
    wasi_serdes_write_uint16_t(ptr, offset, (uint16_t)value);
    wasi_serdes_write_uint16_t(ptr, offset + 2, value >> 16);
}

static inline void wasi_serdes_write_uint16_t(void* ptr, size_t offset, uint16_t value) {
    wasi_serdes_write_uint8_t(ptr, offset, (uint8_t)value);
    wasi_serdes_write_uint8_t(ptr, offset + 1, value >> 8);
}

static inline void wasi_serdes_write_uint8_t(void* ptr, size_t offset, uint8_t value) {
    ((uint8_t*)ptr)[offset] = value;
}

static inline uint64_t wasi_serdes_read_uint64_t(const void* ptr, size_t offset) {
    uint64_t low  = wasi_serdes_read_uint32_t(ptr, offset);
    uint64_t high = wasi_serdes_read_uint32_t(ptr, offset + 4);
    return low | (high << 32);
}

static inline uint32_t wasi_serdes_read_uint32_t(const void* ptr, size_t offset) {
    uint32_t low  = wasi_serdes_read_uint16_t(ptr, offset);
    uint32_t high = wasi_serdes_read_uint16_t(ptr, offset + 2);
    return low | (high << 16);
}

static inline uint16_t wasi_serdes_read_uint16_t(const void* ptr, size_t offset) {
    uint16_t low  = wasi_serdes_read_uint8_t(ptr, offset);
    uint16_t high = wasi_serdes_read_uint8_t(ptr, offset + 1);
    return low | (high << 8);
}

static inline uint8_t wasi_serdes_read_uint8_t(const void* ptr, size_t offset) {
    return ((const uint8_t*)ptr)[offset];
}

#define TYPE_SWITCH switch (value->type)
#define TAG_SWITCH  switch (value->u.tag)

#define ALL_TYPES(STRUCT, FIELD, ALIAS)                                                           \
                                                                                                  \
    ALIAS(advice_t, uint8_t)                                                                      \
    ALIAS(clockid_t, uint32_t)                                                                    \
    ALIAS(device_t, uint64_t)                                                                     \
    ALIAS(dircookie_t, uint64_t)                                                                  \
    ALIAS(errno_t, uint16_t)                                                                      \
    ALIAS(eventrwflags_t, uint16_t)                                                               \
    ALIAS(eventtype_t, uint8_t)                                                                   \
    ALIAS(exitcode_t, uint32_t)                                                                   \
    ALIAS(fd_t, uint32_t)                                                                         \
    ALIAS(fdflags_t, uint16_t)                                                                    \
    ALIAS(filesize_t, uint64_t)                                                                   \
    ALIAS(filetype_t, uint8_t)                                                                    \
    ALIAS(fstflags_t, uint16_t)                                                                   \
    ALIAS(inode_t, uint64_t)                                                                      \
    ALIAS(linkcount_t, uint64_t)                                                                  \
    ALIAS(lookupflags_t, uint32_t)                                                                \
    ALIAS(oflags_t, uint16_t)                                                                     \
    ALIAS(preopentype_t, uint8_t)                                                                 \
    ALIAS(riflags_t, uint16_t)                                                                    \
    ALIAS(rights_t, uint64_t)                                                                     \
    ALIAS(roflags_t, uint16_t)                                                                    \
    ALIAS(sdflags_t, uint8_t)                                                                     \
    ALIAS(siflags_t, uint16_t)                                                                    \
    ALIAS(signal_t, uint8_t)                                                                      \
    ALIAS(size_t, uint32_t)                                                                       \
    ALIAS(subclockflags_t, uint16_t)                                                              \
    ALIAS(timestamp_t, uint64_t)                                                                  \
    ALIAS(userdata_t, uint64_t)                                                                   \
    ALIAS(whence_t, uint8_t)                                                                      \
                                                                                                  \
    STRUCT(dirent_t) {                                                                            \
        FIELD(0, dircookie_t, d_next);                                                            \
        FIELD(8, inode_t, d_ino);                                                                 \
        FIELD(16, uint32_t, d_namlen);                                                            \
        FIELD(20, filetype_t, d_type);                                                            \
    }                                                                                             \
                                                                                                  \
    STRUCT(fdstat_t) {                                                                            \
        FIELD(0, filetype_t, fs_filetype);                                                        \
        FIELD(2, fdflags_t, fs_flags);                                                            \
        FIELD(8, rights_t, fs_rights_base);                                                       \
        FIELD(16, rights_t, fs_rights_inheriting);                                                \
    }                                                                                             \
                                                                                                  \
    STRUCT(filestat_t) {                                                                          \
        FIELD(0, device_t, dev);                                                                  \
        FIELD(8, inode_t, ino);                                                                   \
        FIELD(16, filetype_t, filetype);                                                          \
        FIELD(24, linkcount_t, nlink);                                                            \
        FIELD(32, filesize_t, size);                                                              \
        FIELD(40, timestamp_t, atim);                                                             \
        FIELD(48, timestamp_t, mtim);                                                             \
        FIELD(56, timestamp_t, ctim);                                                             \
    }                                                                                             \
                                                                                                  \
    STRUCT(prestat_t) {                                                                           \
        FIELD(0, preopentype_t, tag);                                                             \
        FIELD(4, uint32_t, u.dir.pr_name_len);                                                    \
    }                                                                                             \
                                                                                                  \
    STRUCT(event_t) {                                                                             \
        FIELD(0, userdata_t, userdata);                                                           \
        FIELD(8, errno_t, error);                                                                 \
        FIELD(10, eventtype_t, type);                                                             \
        TYPE_SWITCH {                                                                             \
            case __WASI_EVENTTYPE_FD_READ:                                                        \
            case __WASI_EVENTTYPE_FD_WRITE:                                                       \
                FIELD(16, filesize_t, fd_readwrite.nbytes);                                       \
                FIELD(24, eventrwflags_t, fd_readwrite.flags);                                    \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    STRUCT(subscription_t) {                                                                      \
        FIELD(0, userdata_t, userdata);                                                           \
        FIELD(8, eventtype_t, u.tag);                                                             \
        TAG_SWITCH {                                                                              \
            case __WASI_EVENTTYPE_CLOCK:                                                          \
                FIELD(16, clockid_t, u.u.clock.id);                                               \
                FIELD(24, timestamp_t, u.u.clock.timeout);                                        \
                FIELD(32, timestamp_t, u.u.clock.precision);                                      \
                FIELD(40, subclockflags_t, u.u.clock.flags);                                      \
                break;                                                                            \
            case __WASI_EVENTTYPE_FD_READ: FIELD(16, fd_t, u.u.fd_read.file_descriptor); break;   \
            case __WASI_EVENTTYPE_FD_WRITE: FIELD(16, fd_t, u.u.fd_write.file_descriptor); break; \
        }                                                                                         \
    }

#define WRITE_STRUCT(name) void wasi_serdes_write_##name(void* ptr, size_t offset, const __wasi_##name* value)

#define READ_STRUCT(name) void wasi_serdes_read_##name(const void* ptr, size_t offset, __wasi_##name* value)

#define WRITE_FIELD(field_offset, type, field)                              \
    do {                                                                    \
        wasi_serdes_write_##type(ptr, offset + field_offset, value->field); \
    } while (0)

#define READ_FIELD(field_offset, type, field)                               \
    do {                                                                    \
        value->field = wasi_serdes_read_##type(ptr, offset + field_offset); \
    } while (0)

#define WRITE_ALIAS(new_name, old_name)                                                    \
    void wasi_serdes_write_##new_name(void* ptr, size_t offset, __wasi_##new_name value) { \
        wasi_serdes_write_##old_name(ptr, offset, value);                                  \
    }

#define READ_ALIAS(new_name, old_name)                                              \
    __wasi_##new_name wasi_serdes_read_##new_name(const void* ptr, size_t offset) { \
        return wasi_serdes_read_##old_name(ptr, offset);                            \
    }

ALL_TYPES(WRITE_STRUCT, WRITE_FIELD, WRITE_ALIAS)
ALL_TYPES(READ_STRUCT, READ_FIELD, READ_ALIAS)


static inline __wasi_errno_t wasi_serdes_read_ciovec_t(const void* ptr, size_t end, size_t offset, __wasi_ciovec_t* value) {
    uint32_t buf_ptr;

    buf_ptr        = wasi_serdes_read_uint32_t(ptr, offset);
    value->buf_len = wasi_serdes_read_size_t(ptr, offset + 4);

    if (!wasi_serdes_check_bounds(buf_ptr, end, value->buf_len))
        return __WASI_ERRNO_OVERFLOW;

    value->buf = ((uint8_t*)ptr + buf_ptr);
    return __WASI_ERRNO_SUCCESS;
}


static inline __wasi_errno_t wasi_serdes_read_iovec_t(const void* ptr, size_t end, size_t offset, __wasi_iovec_t* value) {
    uint32_t buf_ptr;

    buf_ptr        = wasi_serdes_read_uint32_t(ptr, offset);
    value->buf_len = wasi_serdes_read_size_t(ptr, offset + 4);

    if (!wasi_serdes_check_bounds(buf_ptr, end, value->buf_len))
        return __WASI_ERRNO_OVERFLOW;

    value->buf = ((uint8_t*)ptr + buf_ptr);
    return __WASI_ERRNO_SUCCESS;
}


static inline __wasi_errno_t
wasi_serdes_readv_ciovec_t(const void* ptr, size_t end, size_t offset, __wasi_ciovec_t* iovs, __wasi_size_t iovs_len) {
    __wasi_errno_t err;
    __wasi_size_t  i;

    for (i = 0; i < iovs_len; i++) {
        err = wasi_serdes_read_ciovec_t(ptr, end, offset, &iovs[i]);
        if (err != __WASI_ERRNO_SUCCESS)
            return err;
        offset += WASI_SERDES_SIZE_ciovec_t;
    }

    return __WASI_ERRNO_SUCCESS;
}


static inline __wasi_errno_t
wasi_serdes_readv_iovec_t(const void* ptr, size_t end, size_t offset, __wasi_iovec_t* iovs, __wasi_size_t iovs_len) {
    __wasi_errno_t err;
    __wasi_size_t  i;

    for (i = 0; i < iovs_len; i++) {
        err = wasi_serdes_read_iovec_t(ptr, end, offset, &iovs[i]);
        if (err != __WASI_ERRNO_SUCCESS)
            return err;
        offset += WASI_SERDES_SIZE_iovec_t;
    }

    return __WASI_ERRNO_SUCCESS;
}

static inline int wasi_serdes_check_array_bounds(size_t offset, size_t end, size_t size, size_t count) {
    return end > offset && ((count * size) / size == count) && (count * size <= end - offset);
}


#endif /* __WASI_SERDES_H__ */
