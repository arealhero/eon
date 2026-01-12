#pragma once

#include <eon/common.h>

// NOTE(vlad): Defining unsigned integer types.

#if defined(__UINT8_TYPE__)
typedef __UINT8_TYPE__ u8;
#else
#    error __UINT8_TYPE__ is not defined.
#endif

#if defined(__UINT16_TYPE__)
typedef __UINT16_TYPE__ u16;
#else
#    error __UINT16_TYPE__ is not defined.
#endif

#if defined(__UINT32_TYPE__)
typedef __UINT32_TYPE__ u32;
#else
#    error __UINT32_TYPE__ is not defined.
#endif

#if defined(__UINT64_TYPE__)
typedef __UINT64_TYPE__ u64;
#else
#    error __UINT64_TYPE__ is not defined.
#endif

// NOTE(vlad): Defining signed integer types.

#if defined(__INT8_TYPE__)
typedef __INT8_TYPE__ s8;
#else
#    error __INT8_TYPE__ is not defined.
#endif

#if defined(__INT16_TYPE__)
typedef __INT16_TYPE__ s16;
#else
#    error __INT16_TYPE__ is not defined.
#endif

#if defined(__INT32_TYPE__)
typedef __INT32_TYPE__ s32;
#else
#    error __INT32_TYPE__ is not defined.
#endif

#if defined(__INT64_TYPE__)
typedef __INT64_TYPE__ s64;
#else
#    error __INT64_TYPE__ is not defined.
#endif

// NOTE(vlad): Defining floating-point types.

// FIXME(vlad): Ensure that f32 and f64 conforms to IEEE 754.
#if defined(__SIZEOF_FLOAT__)
#    if __SIZEOF_FLOAT__ == 4
typedef float  f32;
#    else
#        error Failed to define "f32": __SIZEOF_FLOAT__ != 4.
#    endif
#else
#    error Failed to define "f32": __SIZEOF_FLOAT__ is not defined.
#endif

#if defined(__SIZEOF_DOUBLE__)
#    if __SIZEOF_DOUBLE__ == 8
typedef double f64;
#    else
#        error Failed to define "f64": __SIZEOF_DOUBLE__ != 8.
#    endif
#else
#    error Failed to define "f64": __SIZEOF_DOUBLE__ is not defined.
#endif

// NOTE(vlad): Defining memory-related types.

#if defined(__PTRDIFF_TYPE__)
typedef __PTRDIFF_TYPE__ Size;
#else
// TODO(vlad): Try to use __PTRDIFF_WIDTH__ and use the appropriate integer type.
//             Something like this: 'typedef CONCATENATE(s, __PTRDIFF_WIDTH__) Size;'
#    error Failed to define "Size": __PTRDIFF_TYPE__ is not defined.
#endif

#if defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ USize;
#else
#    error Failed to define "USize": __SIZE_TYPE__ is not defined.
#endif

typedef Size Index;

// FIXME(vlad): Change to 's8'. Or rather 'byte' should be 'u8' by default
//              and 'sbyte' should be 's8' (if we really need 'sbyte' in the first place).
typedef char Byte;
typedef u8   UByte;

// NOTE(vlad): Defining boolean types.

typedef s8  Bool8;
typedef s32 Bool;

#define true 1
#define false 0

#define size_of(Type) (Size)(sizeof(Type))
#define as_bytes(expression) (Byte*)(expression)

// NOTE(vlad): Compile-time tests.

#include <eon/static_assert.h>

STATIC_ASSERT(size_of(s8) == 1);
STATIC_ASSERT(size_of(s16) == 2);
STATIC_ASSERT(size_of(s32) == 4);
STATIC_ASSERT(size_of(s64) == 8);

STATIC_ASSERT(size_of(u8) == 1);
STATIC_ASSERT(size_of(u16) == 2);
STATIC_ASSERT(size_of(u32) == 4);
STATIC_ASSERT(size_of(u64) == 8);
