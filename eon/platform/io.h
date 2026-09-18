#pragma once
#define EON_PLATFORM_IO_INCLUDED 1

#include <eon/common.h>
#include <eon/keywords.h>

internal void write_data_to_stdout(const Byte* data, const Size data_size);

#if OS_LINUX
#    include "linux/io.c"
#elif OS_MAC
#    include "macos/io.c"
#elif OS_WINDOWS
#    include "win32/io.c"
#else
#    error This OS is not supported yet.
#endif
