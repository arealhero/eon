#pragma once

#include <eon/keywords.h>
#include <eon/types.h>

#include <eon/assert.h>
#include <eon/macros.h>

internal inline void
unused_impl(void* dummy_parameter_for_varargs, ...)
{
    (void) dummy_parameter_for_varargs;
}
#define UNUSED(...) unused_impl(NULL __VA_OPT__(,) __VA_ARGS__)

#define MAX(lhs, rhs) (((lhs) > (rhs)) ? (lhs) : (rhs))
#define MIN(lhs, rhs) (((lhs) < (rhs)) ? (lhs) : (rhs))
#define ABS(number) (((number) > 0) ? (number) : -(number))

#define KiB(count) ((Size)(count) << 10)
#define MiB(count) ((Size)(count) << 20)
#define GiB(count) ((Size)(count) << 30)

internal inline Bool
is_power_of_two(const USize number)
{
    return number != 0 && (number & (number - 1)) == 0;
}

#define ALIGN_UP_TO_POW2(number, power) \
    (((Size)(number) + ((Size)(power) - 1)) & (~((Size)(power) - 1)))

// NOTE(vlad): Compile-time tests.

STATIC_ASSERT(KiB(1) == (Size) 1*1024);
STATIC_ASSERT(MiB(2) == (Size) 2*1024*1024);
STATIC_ASSERT(GiB(3) == (Size) 3*1024*1024*1024);

STATIC_ASSERT(ALIGN_UP_TO_POW2(0x12, 0x20) == 0x20);
STATIC_ASSERT(ALIGN_UP_TO_POW2(0x123, 0x10) == 0x130);
STATIC_ASSERT(ALIGN_UP_TO_POW2(0x123, 1 << 1) == 0x124);
STATIC_ASSERT(ALIGN_UP_TO_POW2(0x123, 1 << 0) == 0x123);
