#pragma once
#define EON_PLATFORM_MEMORY_INCLUDED 1

#include <eon/common.h>

internal Size platform_get_page_size(void);

// TODO(vlad): Change to 'byte*'?
internal void*  platform_reserve_memory(Size number_of_bytes);
internal Bool platform_commit_memory(void* pointer, Size number_of_bytes);
internal Bool platform_decommit_memory(void* pointer, Size number_of_bytes);
internal Bool platform_release_memory(void* pointer, Size number_of_bytes);

#if OS_WINDOWS
    #include "win32_memory.c"
#elif OS_LINUX
    // TODO(vlad): Change to 'unix_memory.c'? Probably not, but then we should use
    //             Linux-specific features like MADV_FREE.
    #include "linux_memory.c"
#elif OS_MAC
    // XXX(vlad): This was just copied from 'linux_memory.c'. We can merge them
    //            to 'unix_memory.c' if we don't use anything bsd- and linux-specific.
    #include "macos_memory.c"
#else
    #error This OS is not supported yet.
#endif
