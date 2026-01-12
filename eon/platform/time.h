#pragma once
#define EON_PLATFORM_TIME_INCLUDED 1

#include <eon/common.h>
#include <eon/keywords.h>

// XXX(vlad): Change 'Size' to 's64'? How would it work on 32-bit platforms?
typedef Size Timestamp; // NOTE(vlad): In microseconds.

internal Timestamp platform_timestamp_now(void);

#if OS_LINUX
#    include "linux_time.c"
#elif OS_MAC
#    include "macos_time.c"
#else
#    error This OS is not supported yet.
#endif
