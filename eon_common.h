#pragma once

#include <stdint.h>
#include <stddef.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef ptrdiff_t ssize;
typedef ptrdiff_t ssize;

typedef s8  bool8;
typedef s32 bool32;

#define true 1
#define false 0

#define internal static

#define ASSERT(expression)                              \
    do                                                  \
    {                                                   \
        if ((expression))                               \
        {                                               \
        }                                               \
        else                                            \
        {                                               \
            puts("Assertion failed: " #expression);     \
            __builtin_debugtrap();                      \
        }                                               \
    } while (0)

#define MAX(lhs, rhs) (((lhs) > (rhs)) ? (lhs) : (rhs))

#define size_of(Type) (ssize)sizeof(Type)
