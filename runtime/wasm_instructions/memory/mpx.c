#include "../../runtime.h"

#define MPX_BC(adr, sz)                                                 \
	{                                                               \
		asm volatile("bndcu " #sz "(%0), %%bnd0" : : "r"(adr)); \
	}

INLINE float
get_f32(u32 offset)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	// Bounds check
	MPX_BC(address, 0x3);

	float v = *(float *)address;
	return v;
}

INLINE double
get_f64(u32 offset)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x7);

	double v = *(double *)address;
	return v;
}

INLINE i8
get_i8(u32 offset)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x0);

	return *(i8 *)address;
}

INLINE i16
get_i16(u32 offset)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x1);

	return *(i16 *)address;
}

INLINE i32
get_i32(u32 offset)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x3);

	i32 v = *(i32 *)address;
	return v;
}

INLINE i64
get_i64(u32 offset)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x7);

	i64 v = *(i64 *)address;
	return v;
}

INLINE void
set_f32(u32 offset, float v)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x3);

	*(float *)address = v;
}

INLINE void
set_f64(u32 offset, double v)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x7);

	*(double *)address = v;
}

INLINE void
set_i8(u32 offset, i8 v)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x0);

	*(i8 *)address = v;
}

INLINE void
set_i16(u32 offset, i16 v)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x1);

	*(i16 *)address = v;
}

INLINE void
set_i32(u32 offset, i32 v)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x3);

	*(i32 *)address = v;
}

INLINE void
set_i64(u32 offset, i64 v)
{
	char *mem_as_chars = (char *)memory;
	void *address      = &mem_as_chars[offset];

	MPX_BC(address, 0x7);

	*(i64 *)address = v;
}

INLINE char *
get_function_from_table(u32 idx, u32 type_id)
{
	awsm_assert(idx < INDIRECT_TABLE_SIZE);

	struct indirect_table_entry f = indirect_table[idx];

	awsm_assert(f.type_id == type_id);
	awsm_assert(f.func_pointer);

	return f.func_pointer;
}
