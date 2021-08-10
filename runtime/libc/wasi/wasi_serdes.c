#include "wasi_serdes.h"
#include "wasi_spec.h"

void wasi_serdes_write_uint64_t(void* ptr, size_t offset, uint64_t value) {
    wasi_serdes_write_uint32_t(ptr, offset, (uint32_t)value);
    wasi_serdes_write_uint32_t(ptr, offset + 4, value >> 32);
}

void wasi_serdes_write_uint32_t(void* ptr, size_t offset, uint32_t value) {
    wasi_serdes_write_uint16_t(ptr, offset, (uint16_t)value);
    wasi_serdes_write_uint16_t(ptr, offset + 2, value >> 16);
}

void wasi_serdes_write_uint16_t(void* ptr, size_t offset, uint16_t value) {
    wasi_serdes_write_uint8_t(ptr, offset, (uint8_t)value);
    wasi_serdes_write_uint8_t(ptr, offset + 1, value >> 8);
}

void wasi_serdes_write_uint8_t(void* ptr, size_t offset, uint8_t value) {
    ((uint8_t*)ptr)[offset] = value;
}

uint64_t wasi_serdes_read_uint64_t(const void* ptr, size_t offset) {
    uint64_t low  = wasi_serdes_read_uint32_t(ptr, offset);
    uint64_t high = wasi_serdes_read_uint32_t(ptr, offset + 4);
    return low | (high << 32);
}

uint32_t wasi_serdes_read_uint32_t(const void* ptr, size_t offset) {
    uint32_t low  = wasi_serdes_read_uint16_t(ptr, offset);
    uint32_t high = wasi_serdes_read_uint16_t(ptr, offset + 2);
    return low | (high << 16);
}

uint16_t wasi_serdes_read_uint16_t(const void* ptr, size_t offset) {
    uint16_t low  = wasi_serdes_read_uint8_t(ptr, offset);
    uint16_t high = wasi_serdes_read_uint8_t(ptr, offset + 1);
    return low | (high << 8);
}

uint8_t wasi_serdes_read_uint8_t(const void* ptr, size_t offset) {
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


__wasi_errno_t wasi_serdes_read_ciovec_t(const void* ptr, size_t end, size_t offset, __wasi_ciovec_t* value) {
    uint32_t buf_ptr;

    buf_ptr        = wasi_serdes_read_uint32_t(ptr, offset);
    value->buf_len = wasi_serdes_read_size_t(ptr, offset + 4);

    if (!wasi_serdes_check_bounds(buf_ptr, end, value->buf_len))
        return __WASI_ERRNO_OVERFLOW;

    value->buf = ((uint8_t*)ptr + buf_ptr);
    return __WASI_ERRNO_SUCCESS;
}


__wasi_errno_t wasi_serdes_read_iovec_t(const void* ptr, size_t end, size_t offset, __wasi_iovec_t* value) {
    uint32_t buf_ptr;

    buf_ptr        = wasi_serdes_read_uint32_t(ptr, offset);
    value->buf_len = wasi_serdes_read_size_t(ptr, offset + 4);

    if (!wasi_serdes_check_bounds(buf_ptr, end, value->buf_len))
        return __WASI_ERRNO_OVERFLOW;

    value->buf = ((uint8_t*)ptr + buf_ptr);
    return __WASI_ERRNO_SUCCESS;
}


__wasi_errno_t
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


__wasi_errno_t
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


int wasi_serdes_check_bounds(size_t offset, size_t end, size_t size) {
    return end > offset && size <= (end - offset);
}


int wasi_serdes_check_array_bounds(size_t offset, size_t end, size_t size, size_t count) {
    return end > offset && ((count * size) / size == count) && (count * size <= end - offset);
}
