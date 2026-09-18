#pragma once
#define EON_PLATFORM_TIME_INCLUDED 1

#include <eon/common.h>
#include <eon/keywords.h>

typedef s64 Timestamp; // NOTE(vlad): In microseconds.

internal Timestamp platform_get_current_monotonic_timestamp(void);

#if OS_LINUX
#    include "linux/time.c"
#elif OS_MAC
#    include "macos/time.c"
#elif OS_WINDOWS
#    include "win32/time.c"
#else
#    error This OS is not supported yet.
#endif
