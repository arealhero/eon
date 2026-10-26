#if !EON_PLATFORM_MEMORY_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/memory.h>" instead.
#endif

#include <eon/assert.h>
#include <eon/io.h>

#include <eon/platform/linux/linux_interface.h>

// TODO(vlad): Poison memory for address sanitizer?
// TODO(vlad): Use calloc/free if asan is enabled because it does not track 'mmap'ed memory.

internal inline Size
platform_get_page_size(void)
{
    return LINUX_PAGE_SIZE;
}

internal Byte*
platform_reserve_memory(const Size number_of_bytes)
{
    Byte* result = mmap(NULL, (USize)number_of_bytes, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED)
    {
        ASSERT(0 && "Failed to reserve memory");
        result = NULL;
    }
    return result;
}

internal Bool
platform_commit_memory(Byte* pointer, Size number_of_bytes)
{
    const int result = mprotect(pointer, (USize)number_of_bytes, PROT_READ|PROT_WRITE);
    return result == 0;
}

internal Bool
platform_decommit_memory(Byte* pointer, Size number_of_bytes)
{
    // NOTE(vlad): 'posix_madvise' and 'mprotect' may fail if address is not a multiple
    //             of the page size as returned by 'sysconf()'.
    ASSERT((Size)(pointer) % LINUX_PAGE_SIZE == 0);

    const int result = mprotect(pointer, (USize)number_of_bytes, PROT_NONE);
    if (result != 0)
    {
        // TODO(vlad): Ignore 'ENOTSUP'.
        return false;
    }

    return true;
}

internal Bool
platform_release_memory(Byte* pointer, Size number_of_bytes)
{
    const int result = munmap(pointer, (USize)number_of_bytes);
    return !result;
}
