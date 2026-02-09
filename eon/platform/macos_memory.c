#if !EON_PLATFORM_MEMORY_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/memory.h>" instead.
#endif

#include <eon/io.h>

#include <unistd.h>
#include <sys/mman.h>

// TODO(vlad): Poison memory for address sanitizer?
// TODO(vlad): Use calloc/free if asan is enabled because it does not track 'mmap'ed memory.

internal Size
platform_get_page_size(void)
{
    // STUDY(vlad): Should we cache page size here? If the answer is yes then we should make
    //              this function thread-safe so we won't need to call it upon entering every
    //              'main()' in every program just to initialize its local_persist variable.
    const Size page_size = sysconf(_SC_PAGESIZE);
    ASSERT(page_size != -1);
    ASSERT(is_power_of_two((USize)page_size));
    return page_size;
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
    println("Decommitting {} bytes ({} pages)",
            number_of_bytes,
            number_of_bytes / platform_get_page_size());

    // NOTE(vlad): 'posix_madvise' and 'mprotect' may fail if address is not a multiple
    //             of the page size as returned by 'sysconf()'.
    ASSERT((Size)pointer % platform_get_page_size() == 0);

    int result = posix_madvise(pointer, (USize)number_of_bytes, POSIX_MADV_DONTNEED);
    if (result != 0)
    {
        return false;
    }

    result = mprotect(pointer, (USize)number_of_bytes, PROT_NONE);
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
