#ifndef __WASI_SERDES_H__
#define __WASI_SERDES_H__

// TODO: Add Node.js license

#include "wasi_spec.h"

/* Basic uint{8,16,32,64}_t read/write functions. */

#define BASIC_TYPE(name, type)                                           \
    void wasi_serdes_write_##name(void* ptr, size_t offset, type value); \
    type wasi_serdes_read_##name(const void* ptr, size_t offset);

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
    __wasi_errno_t wasi_serdes_read_##name(const void* ptr, size_t end, size_t offset, __wasi_##name* value);

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

__wasi_errno_t
wasi_serdes_readv_ciovec_t(const void* ptr, size_t end, size_t offset, __wasi_ciovec_t* iovs, __wasi_size_t iovs_len);

__wasi_errno_t
wasi_serdes_readv_iovec_t(const void* ptr, size_t end, size_t offset, __wasi_iovec_t* iovs, __wasi_size_t iovs_len);

/* Helper functions for memory bounds checking. */
int wasi_serdes_check_bounds(size_t offset, size_t end, size_t size);
int wasi_serdes_check_array_bounds(size_t offset, size_t end, size_t size, size_t count);

#endif /* __WASI_SERDES_H__ */
