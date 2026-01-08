#if !EON_PLATFORM_MEMORY_INCLUDED
#error Do not use this file directly. Include '<eon/platform/memory.h>' instead.
#endif

// FIXME(vlad): Test this -- I just stole this functions without testing.

#include <windows.h>

internal size
platform_get_page_size(void)
{
    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    return info.dwPageSize;
}

internal void*
platform_reserve_memory(const size number_of_bytes)
{
    return VirtualAlloc(NULL, number_of_bytes, MEM_RESERVE, PAGE_READWRITE);
}

internal bool32
platform_commit_memory(void* pointer, size number_of_bytes)
{
    const void* result = VirtualAlloc(pointer, number_of_bytes, MEM_COMMIT, PAGE_READWRITE);
    return result != NULL;
}

internal bool32
platform_decommit_memory(void* pointer, size number_of_bytes)
{
    return VirtualFree(pointer, number_of_bytes, MEM_DECOMMIT);
}

internal bool32
platform_release_memory(void* pointer, size number_of_bytes)
{
    return VirtualFree(pointer, number_of_bytes, MEM_RELEASE);
}
