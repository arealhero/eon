#pragma once
#define EON_PLATFORM_ENTRY_POINT_INCLUDED 1

#include <eon/build_info.h>

#if OS_WINDOWS
#    include "win32_entry_point.c"
#else
#    error This OS is not supported yet.
#endif

