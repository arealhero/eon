#pragma once
#define EON_PLATFORM_ENTRY_POINT_INCLUDED 1

#include <eon/build_info.h>
#include <eon/sanitizers/asan.h>

internal int eon_main(const String_View* arguments, const Size arguments_count);

#if EON_WITHOUT_CRT
#    if ASAN_ENABLED
#        error ASAN requires C runtime.
#    endif

#    if OS_WINDOWS
#        include "win32_entry_point.c"
#    else
#        error This OS is not supported yet.
#    endif
#else
#    include "crt_entry_point.c"
#endif

