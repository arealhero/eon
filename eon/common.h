#pragma once

#include <eon/build-info.h>
#include <eon/macros.h>

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h> // NOTE(vlad): for 'abort()'.

// FIXME(vlad): Remove this header.
#include <string.h> // NOTE(vlad): For 'memset'.

#if !defined(EON_DISABLE_ASSERTS)
    #define EON_DISABLE_ASSERTS 0
#endif

#define local_persist static
#define global_variable static
#define internal static

// NOTE(vlad): This assumes that C11 is enabled.
#define thread_local _Thread_local

internal inline void
unused_impl(void* dummy_parameter_for_varargs, ...)
{
    (void) dummy_parameter_for_varargs;
}
#define UNUSED(...) unused_impl(NULL __VA_OPT__(,) __VA_ARGS__)

#define maybe_unused __attribute__((unused))
#define noreturn __attribute__((noreturn))

noreturn internal inline void
FAIL(void)
{
    // TODO(vlad): Print backtrace?
    // TODO(vlad): Use trap in debug builds only and 'quick_exit()' in release builds?
    __builtin_debugtrap();

    // NOTE(vlad): This line is needed to get rid of this compile error:
    //             "function declared 'noreturn' should not return".
    abort();
}

#if EON_DISABLE_ASSERTS
    #define ASSERT(expression) (void)((expression))
#else
    #define ASSERT(expression) ASSERT_IMPL(expression, __func__, __FILE__, __LINE__)
    #define ASSERT_IMPL(expression, function, file, line)                   \
        do                                                                  \
        {                                                                   \
            if (!(expression))                                              \
            {                                                               \
                printf("%s:%d: Assertion failed in function %s: %s\n",      \
                       file, line, function, #expression);                  \
                /* FIXME(vlad): Print backtrace (use libunwind?). */        \
                FAIL();                                                     \
            }                                                               \
        } while (0)
#endif

// TODO(vlad): Introduce 'STATIC_ASSERT_C(expression, comment)'
//             with CONCATENATE(comment, __COUNTER__)?

// TODO(vlad): Test if __COUNTER__ is available, otherwise use __LINE__.
#define STATIC_ASSERT(expression)                                       \
    typedef int CONCATENATE(Static_Assert_, __COUNTER__)[(expression) ? 1 : -1]

// // TODO(vlad): Use '_Static_assert' from C11?
// #define STATIC_ASSERT(expression) _Static_assert(expression, "Static assertion failed");

#define REQUIRE_SEMICOLON void CONCATENATE(Semicolon_Required_, __COUNTER__)(void)

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef ptrdiff_t ssize;
typedef size_t    usize;

typedef s32 ssize32;
typedef s64 ssize64;

typedef u32 usize32;
typedef u64 usize64;

typedef s8  bool8;
typedef s32 bool32;

typedef char byte;
typedef u8   ubyte;

#define true 1
#define false 0

#define size_of(Type) (ssize)(sizeof(Type))
#define as_bytes(expression) (byte*)(expression)

#define MAX(lhs, rhs) (((lhs) > (rhs)) ? (lhs) : (rhs))
#define MIN(lhs, rhs) (((lhs) < (rhs)) ? (lhs) : (rhs))

internal inline bool32
is_power_of_two(const usize number)
{
    return number != 0 && (number & (number - 1)) == 0;
}

#define ALIGN_UP_TO_POW2(number, power) \
    (((ssize)(number) + ((ssize)(power) - 1)) & (~((ssize)(power) - 1)))

#define KiB(count) ((ssize)(count) << 10)
#define MiB(count) ((ssize)(count) << 20)
#define GiB(count) ((ssize)(count) << 30)

internal inline void
copy_memory(      byte* restrict to,
            const byte* restrict from,
            const ssize number_of_bytes)
{
    memcpy(to, from, (usize)number_of_bytes);
}

// NOTE(vlad): Compile-time tests.

STATIC_ASSERT(KiB(1) == (ssize) 1*1024);
STATIC_ASSERT(MiB(2) == (ssize) 2*1024*1024);
STATIC_ASSERT(GiB(3) == (ssize) 3*1024*1024*1024);

STATIC_ASSERT(ALIGN_UP_TO_POW2(0x12, 0x20) == 0x20);
STATIC_ASSERT(ALIGN_UP_TO_POW2(0x123, 0x10) == 0x130);
STATIC_ASSERT(ALIGN_UP_TO_POW2(0x123, 1 << 1) == 0x124);
STATIC_ASSERT(ALIGN_UP_TO_POW2(0x123, 1 << 0) == 0x123);
