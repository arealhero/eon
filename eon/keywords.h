#pragma once

#define local_persist static
#define global_variable static
#define internal static

// NOTE(vlad): This assumes that C11 is enabled.
#define thread_local _Thread_local

#define maybe_unused __attribute__((unused))
#define noreturn __attribute__((noreturn)) _Noreturn
