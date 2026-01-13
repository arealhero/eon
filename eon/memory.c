#include "memory.h"

#include <eon/platform/memory.h>
#include <eon/sanitizers/asan.h>

#define ARENA_HEADER_SIZE (size_of(Arena))
#define ARENA_ALIGNMENT (size_of(void*))

internal inline void
copy_memory(Byte* restrict to,
            const Byte* restrict from,
            const Size number_of_bytes)
{
    for (Index i = 0;
         i < number_of_bytes;
         ++i)
    {
        to[i] = from[i];
    }
}

internal inline void
move_memory(Byte* to,
            const Byte* from,
            const Size number_of_bytes)
{
    if (to > from && to - from < number_of_bytes)
    {
        // NOTE(vlad): 'to's suffix overlaps with 'from's prefix,
        //             moving memory in reverse order.
        for (Index i = number_of_bytes - 1;
             i >= 0;
             --i)
        {
            to[i] = from[i];
        }
    }
    else
    {
        copy_memory(to, from, number_of_bytes);
    }
}

internal inline void
fill_memory_with_zeros(Byte* memory, const Size number_of_bytes)
{
    for (Size i = 0;
         i < number_of_bytes;
         ++i)
    {
        memory[i] = (Byte)0;
    }
}

#if ASAN_ENABLED
#    define ARENA_REDZONE_SIZE_IN_PAGES 1

internal void
ARENA_ADD_REDZONE(Arena* arena)
{
    const Size aligned_memory_offset = ALIGN_UP_TO_POW2(arena->free_memory_offset,
                                                        ARENA_ALIGNMENT);
    const Size redzone_size = ARENA_REDZONE_SIZE_IN_PAGES * platform_get_page_size();
    const Size free_memory_offset_after_allocation = aligned_memory_offset + redzone_size;

    if (free_memory_offset_after_allocation > arena->reserved_bytes_count)
    {
        // NOTE(vlad): We are using 'SILENT_ASSERT' here to prevent stack overflows
        //             if there are no memory left in the IO state's arena.
        SILENT_ASSERT(0 && "Failed to add redzone for ASAN.");
        FAIL();
    }

    // XXX(vlad): Do we need to commit the redzone's memory here?

    Byte* redzone = as_bytes(arena) + aligned_memory_offset;
    arena->free_memory_offset = free_memory_offset_after_allocation;

    ASAN_POISON_MEMORY_REGION(redzone, redzone_size);
}

#else
#    define ARENA_ADD_REDZONE(arena)
#endif

maybe_unused internal Arena*
arena_create(Size number_of_bytes_to_reserve, Size number_of_bytes_to_commit)
{
    // XXX(vlad): Reserve 'number_of_bytes_to_commit' instead?
    ASSERT(number_of_bytes_to_reserve >= number_of_bytes_to_commit);

    const Size page_size = platform_get_page_size();

    number_of_bytes_to_reserve = ALIGN_UP_TO_POW2(number_of_bytes_to_reserve + ARENA_HEADER_SIZE, page_size);
    number_of_bytes_to_commit = ALIGN_UP_TO_POW2(number_of_bytes_to_commit + ARENA_HEADER_SIZE, page_size);

    Arena* arena = (Arena*) platform_reserve_memory(number_of_bytes_to_reserve);
    if (arena == NULL)
    {
        ASSERT(0 && "Failed to reserve memory");
        return NULL;
    }

    ASAN_POISON_MEMORY_REGION(as_bytes(arena) + ARENA_HEADER_SIZE,
                              number_of_bytes_to_reserve - ARENA_HEADER_SIZE);

    if (!platform_commit_memory(arena, number_of_bytes_to_commit))
    {
        ASSERT(0 && "Failed to commit memory");
        return NULL;
    }

    arena->reserved_bytes_count = number_of_bytes_to_reserve;

    arena->free_memory_offset = ARENA_HEADER_SIZE;
    arena->committed_memory_offset = number_of_bytes_to_commit;

    return arena;
}

maybe_unused internal void
arena_destroy(Arena* arena)
{
    // TODO(vlad): Move this memory to a quarantine in asan is enabled?
    //             This would prevent EOF in situations when the OS will
    //             give us back released memory.
    //             @tag(asan)
    platform_release_memory(arena, arena->reserved_bytes_count);
}

maybe_unused internal void*
arena_push(Arena* arena, const Size number_of_bytes)
{
    void* memory = arena_push_uninitialized(arena, number_of_bytes);
    fill_memory_with_zeros(memory, number_of_bytes);
    return memory;
}

maybe_unused internal void*
arena_push_uninitialized(Arena* arena, const Size number_of_bytes)
{
    ARENA_ADD_REDZONE(arena);

    const Size aligned_memory_offset = ALIGN_UP_TO_POW2(arena->free_memory_offset, ARENA_ALIGNMENT);

    const Size free_memory_offset_after_allocation = aligned_memory_offset + number_of_bytes;
    if (free_memory_offset_after_allocation > arena->reserved_bytes_count)
    {
        // TODO(vlad): Implement growable arenas.
        //             @tag(growable-arena)
        ASSERT(0 && "Failed to allocate memory");
        return NULL;
    }

    if (free_memory_offset_after_allocation > arena->committed_memory_offset)
    {
        const Size page_size = platform_get_page_size();
        // NOTE(vlad): We assume that the page size is a power of 2.
        const Index new_committed_memory_offset = ALIGN_UP_TO_POW2(free_memory_offset_after_allocation,
                                                                   page_size);

        // NOTE(vlad): This assert should never fire, but just in case I will leave it here.
        ASSERT(new_committed_memory_offset <= arena->reserved_bytes_count);

        // NOTE(vlad): Assert that we don't overcommit here.
        ASSERT(new_committed_memory_offset <= free_memory_offset_after_allocation + page_size);

        Byte* memory_to_commit = as_bytes(arena) + arena->committed_memory_offset;
        const Size number_of_bytes_to_commit = new_committed_memory_offset - arena->committed_memory_offset;

        if (!platform_commit_memory(memory_to_commit, number_of_bytes_to_commit))
        {
            ASSERT(0 && "Failed to commit memory");
            return NULL;
        }

        arena->committed_memory_offset = new_committed_memory_offset;
    }

    Byte* memory = as_bytes(arena) + aligned_memory_offset;
    arena->free_memory_offset = free_memory_offset_after_allocation;

    ASAN_UNPOISON_MEMORY_REGION(memory, number_of_bytes);

    ARENA_ADD_REDZONE(arena);

    return memory;
}

maybe_unused internal void
arena_pop_to_position(Arena* arena, const Index position)
{
    if (position < arena->free_memory_offset)
    {
        const Index new_free_memory_offset = MAX(position, ARENA_HEADER_SIZE);

#if ASAN_ENABLED
        // NOTE(vlad): We do not actually reset 'free_memory_offset' to a 'new_free_memory_offset' here
        //             if asan is enabled. See NOTE in 'arena_clear'.
        ASAN_POISON_MEMORY_REGION(as_bytes(arena) + new_free_memory_offset,
                                  arena->free_memory_offset - new_free_memory_offset);
#else
        arena->free_memory_offset = new_free_memory_offset;
#endif
    }

    // TODO(vlad): Decommit memory? Probably not, but maybe we should add
    //             something like 'arena_pop_and_decommit()' for some rare use cases.
}

maybe_unused internal void
arena_clear(Arena* arena)
{
#if ASAN_ENABLED
    // NOTE(vlad): We do not actually reset 'free_memory_offset' here
    //             to be able to find use-after-free bugs like this:
    //
    //                 int* first_int = allocate(arena, int);
    //                 arena_clear(arena);
    //                 int* second_int = allocate(arena, int);
    //                 *first_int = 10; // Use after free.
    //                 *second_int = 10; // OK
    //
    ASAN_POISON_MEMORY_REGION(as_bytes(arena) + ARENA_HEADER_SIZE,
                              arena->free_memory_offset - ARENA_HEADER_SIZE);

    // TODO(vlad): We will quickly run out of memory if we don't clear
    //             long standing frequently reusable arenas (e.g. frame arenas).
    //             We need to track how many memory was used since the last clear
    //             and if there are less memory available in the arena than the max previous
    //             usage then we need to actually clear it. Yes, we will not find use-after-free
    //             bugs after clearing the arena, but we will get OOMs otherwise.
    //
    //             Another approach: track how many times we postponed clearing the arena
    //             and clear it if it exceeds some threshold (say, 10). But I like the
    //             max used memory approach more.
    //
    //             Also we need to think about this situation when we will implement growable arenas.
    //             @tag(asan) @tag(growable-arena)

    return;
#endif

    const Size page_size = platform_get_page_size();

    ASSERT(arena->committed_memory_offset % page_size == 0);
    ASSERT(ARENA_HEADER_SIZE < page_size);

    void* memory_to_decommit = as_bytes(arena) + page_size;
    const Size number_of_bytes_to_decommit = arena->committed_memory_offset - page_size;

    platform_decommit_memory(memory_to_decommit, number_of_bytes_to_decommit);

    arena->committed_memory_offset = page_size;
    arena->free_memory_offset = ARENA_HEADER_SIZE;
}

maybe_unused internal void*
arena_reallocate(Arena* restrict arena,
                 Byte* restrict memory,
                 const Size memory_size_in_bytes,
                 const Size requested_size_in_bytes)
{
    if (memory == NULL)
    {
        return arena_push(arena, requested_size_in_bytes);
    }

    // TODO(vlad): Add these assertions to all arena-related functions?
    ASSERT(memory_size_in_bytes > 0 && requested_size_in_bytes >= 0);

#if ASAN_ENABLED
    // NOTE(vlad): Always reallocate and copy memory if asan is enabled.
    //             This would be able to find stale pointers to the memory that could
    //             be potentially moved here.
#else
    if (memory + memory_size_in_bytes == as_bytes(arena) + arena->free_memory_offset)
    {
        // NOTE(vlad): This memory is at the end of the arena, so we can just
        //             change its capacity without copying anything.

        if (requested_size_in_bytes < memory_size_in_bytes)
        {
            const Size difference = memory_size_in_bytes - requested_size_in_bytes;
            arena->free_memory_offset = MAX(arena->free_memory_offset - difference,
                                            ARENA_HEADER_SIZE);
        }
        else
        {
            const Size difference = requested_size_in_bytes - memory_size_in_bytes;
            arena_push(arena, difference);
        }

        return memory;
    }
#endif

    // NOTE(vlad): Something else was allocated thus we must reallocate and copy memory.
    void* new_memory = arena_push(arena, requested_size_in_bytes);
    copy_memory(as_bytes(new_memory), memory,
                MIN(requested_size_in_bytes, memory_size_in_bytes));

    ASAN_POISON_MEMORY_REGION(memory, memory_size_in_bytes);

    return new_memory;
}
