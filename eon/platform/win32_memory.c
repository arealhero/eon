#if !EON_PLATFORM_MEMORY_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/memory.h>" instead.
#endif

// FIXME(vlad): Test this -- I just stole this functions without testing.

#include <windows.h>

internal Size
platform_get_page_size(void)
{
    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    return info.dwPageSize;
}

internal Byte*
platform_reserve_memory(const Size number_of_bytes)
{
    return VirtualAlloc(NULL, number_of_bytes, MEM_RESERVE, PAGE_READWRITE);
}

internal Bool
platform_commit_memory(Byte* pointer, Size number_of_bytes)
{
    const Byte* result = VirtualAlloc(pointer, number_of_bytes, MEM_COMMIT, PAGE_READWRITE);
    return result != NULL;
}

internal Bool
platform_decommit_memory(Byte* pointer, Size number_of_bytes)
{
    return VirtualFree(pointer, number_of_bytes, MEM_DECOMMIT);
}

internal Bool
platform_release_memory(Byte* pointer, Size number_of_bytes)
{
    return VirtualFree(pointer, number_of_bytes, MEM_RELEASE);
}
