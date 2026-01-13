#pragma once

#include <eon/common.h>
#include <eon/conversions.h> // NOTE(vlad): Exporting 'KiB', 'MiB', etc for convenience.

internal inline void copy_memory(Byte* restrict to,
                                 const Byte* restrict from,
                                 const Size number_of_bytes);

internal inline void move_memory(Byte* to,
                                 const Byte* from,
                                 const Size number_of_bytes);

internal inline void fill_memory_with_zeros(Byte* memory, const Size number_of_bytes);

typedef struct
{
    Size reserved_bytes_count;

    Index free_memory_offset;
    Index committed_memory_offset;
} Arena;

internal Arena* arena_create(Size number_of_bytes_to_reserve, Size number_of_bytes_to_commit);
internal void arena_destroy(Arena* arena);

internal Byte* arena_push(Arena* arena, Size number_of_bytes);
internal Byte* arena_push_uninitialized(Arena* arena, Size number_of_bytes);
// TODO(vlad): Change to 'arena_save_position' and 'arena_restore_position'.
internal void arena_pop_to_position(Arena* arena, Index position);
internal void arena_clear(Arena* arena);

// TODO(vlad): Support 'arena_reallocate_uninitialized'?
internal Byte* arena_reallocate(Arena* restrict arena,
                                Byte* restrict memory,
                                const Size memory_size_in_bytes,
                                const Size requested_size_in_bytes);

#define allocate(arena, Type) (Type*)(arena_push((arena), size_of(Type)))
#define allocate_uninitialized(arena, Type) (Type*)(arena_push_uninitialized((arena), size_of(Type)))

#define reallocate(arena, pointer, Type, old_count, new_count) \
    (Type*)(arena_reallocate(arena, as_bytes(pointer), size_of(Type) * old_count, size_of(Type) * new_count))

#define allocate_array(arena, number_of_elements, Type)                 \
    (Type*)(arena_push((arena), size_of(Type) * (number_of_elements)))
#define allocate_uninitialized_array(arena, number_of_elements, Type)   \
    (Type*)(arena_push_uninitialized((arena), size_of(Type) * (number_of_elements)))
