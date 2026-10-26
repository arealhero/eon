#pragma once

#include <eon/build_info.h>

#define local_persist static
#define global_variable static
#define internal static

#define external extern

// NOTE(vlad): This assumes that C11 is enabled.
#define thread_local _Thread_local
