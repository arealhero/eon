#pragma once

#include <eon/build_info.h>

#if COMPILER_MSVC
typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;
#else

#    if defined(__UINT8_TYPE__)
typedef __UINT8_TYPE__ u8;
#    else
#        error __UINT8_TYPE__ is not defined.
#    endif

#    if defined(__UINT16_TYPE__)
typedef __UINT16_TYPE__ u16;
#    else
#        error __UINT16_TYPE__ is not defined.
#    endif

#    if defined(__UINT32_TYPE__)
typedef __UINT32_TYPE__ u32;
#    else
#        error __UINT32_TYPE__ is not defined.
#    endif

#    if defined(__UINT64_TYPE__)
typedef __UINT64_TYPE__ u64;
#    else
#        error __UINT64_TYPE__ is not defined.
#    endif

// NOTE(vlad): Defining signed integer types.

#    if defined(__INT8_TYPE__)
typedef __INT8_TYPE__ s8;
#    else
#        error __INT8_TYPE__ is not defined.
#    endif

#    if defined(__INT16_TYPE__)
typedef __INT16_TYPE__ s16;
#    else
#        error __INT16_TYPE__ is not defined.
#    endif

#    if defined(__INT32_TYPE__)
typedef __INT32_TYPE__ s32;
#    else
#        error __INT32_TYPE__ is not defined.
#    endif

#    if defined(__INT64_TYPE__)
typedef __INT64_TYPE__ s64;
#    else
#        error __INT64_TYPE__ is not defined.
#    endif
#endif

// NOTE(vlad): Defining floating-point types.

// FIXME(vlad): Ensure that f32 and f64 conforms to IEEE 754.
#if defined(__SIZEOF_FLOAT__)
#    if __SIZEOF_FLOAT__ == 4
typedef float f32;
#    else
#        error Failed to define "f32": __SIZEOF_FLOAT__ != 4.
#    endif
#else
#    if COMPILER_MSVC
typedef float f32;
#    else
#        error Failed to define "f32": __SIZEOF_FLOAT__ is not defined.
#    endif
#endif

#if defined(__SIZEOF_DOUBLE__)
#    if __SIZEOF_DOUBLE__ == 8
typedef double f64;
#    else
#        error Failed to define "f64": __SIZEOF_DOUBLE__ != 8.
#    endif
#else
#    if COMPILER_MSVC
typedef double f64;
#    else
#        error Failed to define "f64": __SIZEOF_DOUBLE__ is not defined.
#    endif
#endif

// NOTE(vlad): Defining memory-related types.

#if ARCH_32BIT
typedef s32 Size;
typedef u32 USize;
#elif ARCH_64BIT
typedef s64 Size;
typedef u64 USize;
#else
#    error Failed to define "Size" and "USize": unknown target architecture bitness.
#endif

// NOTE(vlad): Integer limits.

#define MAX_VALUE(Integer_Type)                 \
    _Generic((Integer_Type)0,                   \
                 s8: (s8)0x7F,                  \
                 s16: (s16)0x7FFF,              \
                 s32: (s32)0x7FFFFFFF,          \
                 s64: (s64)0x7FFFFFFFFFFFFFFF,  \
                                                \
                 u8: (u8)0xFF,                  \
                 u16: (u16)0xFFFF,              \
                 u32: (u32)0xFFFFFFFF,          \
                 u64: (u64)0xFFFFFFFFFFFFFFFF   \
             )

#define MIN_VALUE(Integer_Type)                 \
    _Generic((Integer_Type)0,                   \
                 s8: (s8)0x80,                  \
                 s16: (s16)0x8000,              \
                 s32: (s32)0x80000000,          \
                 s64: (s64)0x8000000000000000,  \
                                                \
                 u8: (u8)0,                     \
                 u16: (u16)0,                   \
                 u32: (u32)0,                   \
                 u64: (u64)0                    \
             )

typedef Size Index;

typedef u8 Byte;
typedef s8 Signed_Byte;

// NOTE(vlad): Defining boolean types.

typedef s8  Bool8;
typedef s32 Bool;

#define true ((Bool)1)
#define false ((Bool)0)

#define true8 ((Bool8)1)
#define false8 ((Bool8)0)

#define size_of(Type) (Size)(sizeof(Type))
#define as_bytes(expression) (Byte*)(expression)

#define WIDTH_IN_BITS(number)                   \
    _Generic((number),                          \
                 s8: 8,                         \
                 s16: 16,                       \
                 s32: 32,                       \
                 s64: 64,                       \
                                                \
                 u8: 8,                         \
                 u16: 16,                       \
                 u32: 32,                       \
                 u64: 64                        \
             )

#define IS_SIGNED(number)                       \
    _Generic((number),                          \
                 s8: true,                      \
                 s16: true,                     \
                 s32: true,                     \
                 s64: true,                     \
                                                \
                 u8: false,                     \
                 u16: false,                    \
                 u32: false,                    \
                 u64: false                     \
             )

#define TYPE_WIDTH_IN_BITS(Integer_Type) WIDTH_IN_BITS((Integer_Type)0)
#define TYPE_IS_SIGNED(Integer_Type) IS_SIGNED((Integer_Type)0)

#define FOR_EACH_SIGNED_INTEGER_TYPE(DO, ...)   \
    DO(s8 __VA_OPT__(,) __VA_ARGS__)            \
    DO(s16 __VA_OPT__(,) __VA_ARGS__)           \
    DO(s32 __VA_OPT__(,) __VA_ARGS__)           \
    DO(s64 __VA_OPT__(,) __VA_ARGS__)           \

#define FOR_EACH_UNSIGNED_INTEGER_TYPE(DO, ...)   \
    DO(u8 __VA_OPT__(,) __VA_ARGS__)              \
    DO(u16 __VA_OPT__(,) __VA_ARGS__)             \
    DO(u32 __VA_OPT__(,) __VA_ARGS__)             \
    DO(u64 __VA_OPT__(,) __VA_ARGS__)             \

#define FOR_EACH_INTEGER_TYPE(DO, ...)                                  \
    FOR_EACH_SIGNED_INTEGER_TYPE(DO __VA_OPT__(,) __VA_ARGS__)          \
    FOR_EACH_UNSIGNED_INTEGER_TYPE(DO __VA_OPT__(,) __VA_ARGS__)

#define FOR_EACH_FLOAT_TYPE(DO, ...)            \
    DO(f32 __VA_OPT__(,) __VA_ARGS__)           \
    DO(f64 __VA_OPT__(,) __VA_ARGS__)

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

STATIC_ASSERT(size_of(f32) == 4);
STATIC_ASSERT(size_of(f64) == 8);

STATIC_ASSERT(MAX_VALUE(Index) == MAX_VALUE(Size));

STATIC_ASSERT(TYPE_WIDTH_IN_BITS(s8) == 8);
STATIC_ASSERT(TYPE_WIDTH_IN_BITS(s16) == 16);
STATIC_ASSERT(TYPE_WIDTH_IN_BITS(s32) == 32);
STATIC_ASSERT(TYPE_WIDTH_IN_BITS(s64) == 64);

STATIC_ASSERT(TYPE_WIDTH_IN_BITS(u8) == 8);
STATIC_ASSERT(TYPE_WIDTH_IN_BITS(u16) == 16);
STATIC_ASSERT(TYPE_WIDTH_IN_BITS(u32) == 32);
STATIC_ASSERT(TYPE_WIDTH_IN_BITS(u64) == 64);

STATIC_ASSERT(WIDTH_IN_BITS((s8) 123) == 8);
STATIC_ASSERT(WIDTH_IN_BITS((s16) 123) == 16);
STATIC_ASSERT(WIDTH_IN_BITS((s32) 123) == 32);
STATIC_ASSERT(WIDTH_IN_BITS((s64) 123) == 64);

STATIC_ASSERT(WIDTH_IN_BITS((u8) 123) == 8);
STATIC_ASSERT(WIDTH_IN_BITS((u16) 123) == 16);
STATIC_ASSERT(WIDTH_IN_BITS((u32) 123) == 32);
STATIC_ASSERT(WIDTH_IN_BITS((u64) 123) == 64);

STATIC_ASSERT(TYPE_IS_SIGNED(s8) == true);
STATIC_ASSERT(TYPE_IS_SIGNED(s16) == true);
STATIC_ASSERT(TYPE_IS_SIGNED(s32) == true);
STATIC_ASSERT(TYPE_IS_SIGNED(s64) == true);

STATIC_ASSERT(TYPE_IS_SIGNED(u8) == false);
STATIC_ASSERT(TYPE_IS_SIGNED(u16) == false);
STATIC_ASSERT(TYPE_IS_SIGNED(u32) == false);
STATIC_ASSERT(TYPE_IS_SIGNED(u64) == false);

STATIC_ASSERT(IS_SIGNED((s8) 123) == true);
STATIC_ASSERT(IS_SIGNED((s16) 123) == true);
STATIC_ASSERT(IS_SIGNED((s32) 123) == true);
STATIC_ASSERT(IS_SIGNED((s64) 123) == true);

STATIC_ASSERT(IS_SIGNED((u8) 123) == false);
STATIC_ASSERT(IS_SIGNED((u16) 123) == false);
STATIC_ASSERT(IS_SIGNED((u32) 123) == false);
STATIC_ASSERT(IS_SIGNED((u64) 123) == false);
