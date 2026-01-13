#pragma once

#include <eon/build_info.h>
#include <eon/keywords.h>
#include <eon/types.h>

#if ASAN_ENABLED
#    include <sanitizer/asan_interface.h>

#    define ASAN_DEFAULT_OPTIONS "detect_stack_use_after_return=true:"  \
                                 "check_initialization_order=true:"     \
                                 "strict_init_order=true:"              \
                                 "strict_string_checks=true:"           \
                                 "poison_history_size=8192"

#    if !defined(ASAN_SET_DEFAULT_OPTIONS)
#        define ASAN_SET_DEFAULT_OPTIONS 1
#    endif

#    if ASAN_SET_DEFAULT_OPTIONS
external const char*
__asan_default_options(void)
{
    return ASAN_DEFAULT_OPTIONS;
}
#    endif

// FIXME(vlad): Change '#ifdef's to '#if defined()' and '#ifndef's to '#if !defined()'.
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
