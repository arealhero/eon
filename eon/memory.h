#pragma once

#include <eon/common.h>

typedef struct
{
    ssize reserved_bytes_count;

    ssize free_memory_offset;
    ssize committed_memory_offset;
} Arena;

internal Arena* arena_create(ssize number_of_bytes_to_reserve, ssize number_of_bytes_to_commit);
internal void arena_destroy(Arena* arena);

internal void* arena_push(Arena* arena, ssize number_of_bytes);
internal void* arena_push_uninitialized(Arena* arena, ssize number_of_bytes);
// TODO(vlad): Change to 'arena_save_position' and 'arena_restore_position'.
internal void arena_pop_to_position(Arena* arena, ssize position);
internal void arena_clear(Arena* arena);

internal void* arena_reallocate(Arena* restrict arena,
                                byte* restrict memory,
                                const ssize memory_size_in_bytes,
                                const ssize requested_size_in_bytes);

#define allocate(arena, Type) (Type*)(arena_push((arena), size_of(Type)))
#define allocate_uninitialized(arena, Type) (Type*)(arena_push_uninitialized((arena), size_of(Type)))

#define allocate_array(arena, number_of_elements, Type)                 \
    (Type*)(arena_push((arena), size_of(Type) * (number_of_elements)))
#define allocate_uninitialized_array(arena, number_of_elements, Type)   \
    (Type*)(arena_push_uninitialized((arena), size_of(Type) * (number_of_elements)))
