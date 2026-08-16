#ifndef COMMON_H
#define COMMON_H

#include <inttypes.h>

// Utilities macros.
#define ref(value)										(&value)									// Get the pointer of a value.
#define val(reference)									(*reference)								// Get the value of a pointer.
#define ptr(type)										type*										// Pointer type of the type.
#define arr(type)										ptr(type)									// Array type of the type.
#define const_ptr(type)									const type*									// Constant pointer type of the type.
#define func(function_name, return_type, arguments...)	return_type (*function_name)(arguments)		// Function pointer type.
#define cast_to(type, value)							((type)(value))								// Type cast.
#define cast_ptr_to_val(type, value)					(val(cast_to(ptr(type), value)))			// Pointer cast to value.
#define alloc_ptr(type)									cast_to(ptr(type), calloc(1, sizeof(type)))	// Allocate pointer.

// Unified primitive types.
typedef const char*	const_string;	// Constant string,
typedef char*		string;			// String.
typedef ptr(void)	opaque;			// Opaque pointer.
typedef uint8_t		b8;				// Unsigned 8-bit boolean.
typedef uint8_t		u8;				// Unsigned 8-bit integer.
typedef uint16_t	u16;			// Unsigned 16-bit integer.
typedef uint32_t	u32;			// Unsigned 32-bit integer.
typedef uint64_t	u64;			// Unsigned 64-bit integer.
typedef int8_t		s8;				// Signed 8-bit integer.
typedef int16_t		s16;			// Signed 16-bit integer.
typedef int32_t		s32;			// Signed 32-bit integer.
typedef int64_t		s64;			// Signed 64-bit integer.
typedef float		f32;			// 32-bit floating point.
typedef double		f64;			// 64-bit floating point.

#endif //COMMON_H
