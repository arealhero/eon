#pragma once

#define local_persist static
#define global_variable static
#define internal static

// NOTE(vlad): This assumes that C11 is enabled.
#define thread_local _Thread_local

// TODO(vlad): Don't use attributes directly. We should determine what attribute to use
//             in '<eon/build_info.h>'.
#define maybe_unused __attribute__((unused))
#define noreturn __attribute__((noreturn)) _Noreturn
