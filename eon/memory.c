#include "memory.h"

#include <eon/platform/memory.h>

#define ARENA_HEADER_SIZE (size_of(Arena))
#define ARENA_ALIGNMENT (size_of(void*))

internal inline void
copy_memory(      byte* restrict to,
            const byte* restrict from,
            const ssize number_of_bytes)
{
    for (ssize i = 0;
         i < number_of_bytes;
         ++i)
    {
        to[i] = from[i];
    }
}

internal inline void
move_memory(      byte* to,
            const byte* from,
            const ssize number_of_bytes)
{
    if (ABS(to - from) < number_of_bytes)
    {
        // NOTE(vlad): Regions overlap.

        if (to < from)
        {
            copy_memory(to, from, number_of_bytes);
        }
        else
        {
            for (ssize i = number_of_bytes - 1;
                 i >= 0;
                 --i)
            {
                to[i] = from[i];
            }
        }
    }
    else
    {
        copy_memory(to, from, number_of_bytes);
    }
}

internal inline void
fill_memory_with_zeros(byte* memory, const ssize number_of_bytes)
{
    for (ssize i = 0;
         i < number_of_bytes;
         ++i)
    {
        memory[i] = (byte)0;
    }
}

maybe_unused internal Arena*
arena_create(ssize number_of_bytes_to_reserve, ssize number_of_bytes_to_commit)
{
    // XXX(vlad): Reserve 'number_of_bytes_to_commit' instead?
    ASSERT(number_of_bytes_to_reserve >= number_of_bytes_to_commit);

    const ssize page_size = platform_get_page_size();

    number_of_bytes_to_reserve = ALIGN_UP_TO_POW2(number_of_bytes_to_reserve + ARENA_HEADER_SIZE, page_size);
    number_of_bytes_to_commit = ALIGN_UP_TO_POW2(number_of_bytes_to_commit + ARENA_HEADER_SIZE, page_size);

    Arena* arena = (Arena*) platform_reserve_memory(number_of_bytes_to_reserve);
    if (arena == NULL)
    {
        ASSERT(0 && "Failed to reserve memory");
        return NULL;
    }

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
    platform_release_memory(arena, arena->reserved_bytes_count);
}

maybe_unused internal void*
arena_push(Arena* arena, const ssize number_of_bytes)
{
    void* memory = arena_push_uninitialized(arena, number_of_bytes);
    fill_memory_with_zeros(memory, number_of_bytes);
    return memory;
}

maybe_unused internal void*
arena_push_uninitialized(Arena* arena, const ssize number_of_bytes)
{
    const ssize aligned_memory_offset = ALIGN_UP_TO_POW2(arena->free_memory_offset, ARENA_ALIGNMENT);

    const ssize free_memory_offset_after_allocation = aligned_memory_offset + number_of_bytes;
    if (free_memory_offset_after_allocation > arena->reserved_bytes_count)
    {
        ASSERT(0 && "Failed to allocate memory");
        return NULL;
    }

    if (free_memory_offset_after_allocation > arena->committed_memory_offset)
    {
        const ssize page_size = platform_get_page_size();
        // NOTE(vlad): We assume that the page size is a power of 2.
        const ssize new_committed_memory_offset = ALIGN_UP_TO_POW2(free_memory_offset_after_allocation,
                                                                   page_size);

        // NOTE(vlad): This assert should never fire, but just in case I will leave it here.
        ASSERT(new_committed_memory_offset <= arena->reserved_bytes_count);

        // NOTE(vlad): Assert that we don't overcommit here.
        ASSERT(new_committed_memory_offset <= free_memory_offset_after_allocation + page_size);

        byte* memory_to_commit = as_bytes(arena) + arena->committed_memory_offset;
        const ssize number_of_bytes_to_commit = new_committed_memory_offset - arena->committed_memory_offset;

        if (!platform_commit_memory(memory_to_commit, number_of_bytes_to_commit))
        {
            ASSERT(0 && "Failed to commit memory");
            return NULL;
        }

        arena->committed_memory_offset = new_committed_memory_offset;
    }

    byte* memory = as_bytes(arena) + aligned_memory_offset;
    arena->free_memory_offset = free_memory_offset_after_allocation;

    return memory;
}

maybe_unused internal void
arena_pop_to_position(Arena* arena, const ssize position)
{
    if (position < arena->free_memory_offset)
    {
        arena->free_memory_offset = MAX(position, ARENA_HEADER_SIZE);
    }

    // TODO(vlad): Decommit memory? Probably not, but maybe we should add
    //             something like 'arena_pop_and_decommit()' for some rare use cases.
}

maybe_unused internal void
arena_clear(Arena* arena)
{
    const ssize page_size = platform_get_page_size();

    ASSERT(arena->committed_memory_offset % page_size == 0);
    ASSERT(ARENA_HEADER_SIZE < page_size);

    void* memory_to_decommit = as_bytes(arena) + page_size;
    const ssize number_of_bytes_to_decommit = arena->committed_memory_offset - page_size;

    platform_decommit_memory(memory_to_decommit, number_of_bytes_to_decommit);

    arena->committed_memory_offset = page_size;
    arena->free_memory_offset = ARENA_HEADER_SIZE;
}

maybe_unused internal void*
arena_reallocate(Arena* restrict arena,
                 byte* restrict memory,
                 const ssize memory_size_in_bytes,
                 const ssize requested_size_in_bytes)
{
    if (memory == NULL)
    {
        return arena_push(arena, requested_size_in_bytes);
    }

    // TODO(vlad): Add these assertions to all arena-related functions?
    ASSERT(memory_size_in_bytes > 0 && requested_size_in_bytes >= 0);

    if (memory + memory_size_in_bytes == as_bytes(arena) + arena->free_memory_offset)
    {
        // NOTE(vlad): This memory is at the end of the arena, so we can just
        //             change its capacity without copying anything.

        if (requested_size_in_bytes < memory_size_in_bytes)
        {
            const ssize difference = memory_size_in_bytes - requested_size_in_bytes;
            arena->free_memory_offset = MAX(arena->free_memory_offset - difference,
                                            ARENA_HEADER_SIZE);
        }
        else
        {
            const ssize difference = requested_size_in_bytes - memory_size_in_bytes;
            arena_push(arena, difference);
        }

        return memory;
    }

    // FIXME(vlad): Poison old memory.
    // NOTE(vlad): Something else was allocated thus we must reallocate and copy memory.
    void* new_memory = arena_push(arena, requested_size_in_bytes);
    copy_memory(as_bytes(new_memory), memory,
                MIN(requested_size_in_bytes, memory_size_in_bytes));
    return new_memory;
}
