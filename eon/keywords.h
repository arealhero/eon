#pragma once

#include <eon/build_info.h>

#define local_persist static
#define global_variable static
#define internal static

#define external extern

// NOTE(vlad): This assumes that C11 is enabled.
#define thread_local _Thread_local

#if COMPILER_CLANG || COMPILER_GCC
#    define maybe_unused __attribute__((unused))
#    define noreturn __attribute__((noreturn))
#elif COMPILER_MSVC
// TODO(vlad): Implement this.
#    define maybe_unused
#    define noreturn __declspec(noreturn)
#else COMPILER_MSVC
#    error Failed to define 'maybe_unused' and 'noreturn'.
#endif
