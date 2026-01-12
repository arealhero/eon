#pragma once

#include <eon/build_info.h>
#include <eon/keywords.h>
#include <eon/types.h>

#ifdef ASAN_ENABLED
#    include <sanitizer/asan_interface.h>

#    ifdef ASAN_POISON_MEMORY_REGION
#        undef ASAN_POISON_MEMORY_REGION
#    endif

#    ifdef ASAN_UNPOISON_MEMORY_REGION
#        undef ASAN_UNPOISON_MEMORY_REGION
#    endif

internal inline void
ASAN_POISON_MEMORY_REGION(Byte* memory, const Size number_of_bytes)
{
    __asan_poison_memory_region(memory, (USize)number_of_bytes);
}

internal inline void
ASAN_UNPOISON_MEMORY_REGION(Byte* memory, const Size number_of_bytes)
{
    __asan_unpoison_memory_region(memory, (USize)number_of_bytes);
}

#else
#    define ASAN_POISON_MEMORY_REGION(address, size)
#    define ASAN_UNPOISON_MEMORY_REGION(address, size)
#endif
