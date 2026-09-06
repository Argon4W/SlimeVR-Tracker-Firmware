#ifndef SLIME_COMMON_H
#define SLIME_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

// Utilities macros.
#define ref(value)										(&value)																// Get the pointer of a value.
#define val(reference)									(*reference)															// Get the value of a pointer.
#define ptr(type)										type*																	// Pointer type of the type.
#define arr(type)										ptr(type)																// Array type of the type.
#define func(function_name, return_type, arguments...)	return_type (*function_name)(arguments)									// Function pointer type.
#define cast_to(type, value)							((type)(value))															// Type cast.
#define cast_ptr(type, value)							(cast_to(ptr(type), value))												// Pointer cast to value.
#define alloc_ptr(type)									cast_to(ptr(type), calloc(1, sizeof(type)))								// Allocate pointer.
#define alloc_arr(type, count)							cast_to(ptr(type), calloc((count), sizeof(type)))						// Allocate array pointer.
#define alloc_dma_arr(type, count)						cast_to(ptr(type), heap_caps_calloc((count), sizeof(type), DMA_CAPS))	// Allocate DMA array pointer.
#define padding16(pointer_)								((((pointer_) + 15U) & (~cast_to(pointer, 15U))) - pointer_)			// Get the padding in bytes to reach the next least 16-byte aligned address.

// Unified primitive types.
typedef const char*	const_string;	// Constant string,
typedef char*		string;			// String.
typedef ptr(void)	opaque;			// Opaque pointer.
typedef uintptr_t	pointer;		// Pointer value.
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

// Common bit operations
inline u32 ctz(const u32 val) {
	return val == 0 ? 32 : __builtin_ctz(val);
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_COMMON_H
