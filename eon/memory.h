#pragma once

#include <eon/common.h>
#include <eon/containers.h>
#include <eon/conversions.h> // NOTE(vlad): Exporting 'KiB', 'MiB', etc for convenience.
#include <eon/macros.h>
#include <eon/string.h>

internal inline void copy_memory(Byte* restrict to,
                                 const Byte* restrict from,
                                 const Size number_of_bytes);

internal inline void move_memory(Byte* to,
                                 const Byte* from,
                                 const Size number_of_bytes);

internal inline void fill_memory_with_zeros(Byte* memory, const Size number_of_bytes);

internal inline Bool memory_chunks_are_equal(const Byte* lhs, const Byte* rhs, const Size number_of_bytes);

internal inline Size count_trailing_zero_bits(const u64 number);

#define fill_with_zeros(pointer, number_of_elements, Type)              \
    fill_memory_with_zeros(as_bytes(pointer), size_of(Type) * number_of_elements)

struct Bytes_View
{
    const Byte* data;
    Size data_size_in_bytes;
};
typedef struct Bytes_View Bytes_View;

struct Buffer
{
    array(Byte, bytes);
};
typedef struct Buffer Buffer;

internal inline Bytes_View
INTERNAL_bytes_view_as_bytes_view(const Byte* data, const Size size)
{
    UNUSED(size);
    return *(Bytes_View*)(data);
}

internal inline Bytes_View
INTERNAL_buffer_as_bytes_view(const Byte* data, const Size size)
{
    UNUSED(size);
    Buffer* buffer = (Buffer*)(data);

    Bytes_View result = {0};
    result.data = buffer->bytes;
    result.data_size_in_bytes = buffer->bytes_count;
    return result;
}

internal inline Bytes_View
INTERNAL_default_as_bytes_view(const Byte* data, const Size size)
{
    Bytes_View result = {0};
    result.data = data;
    result.data_size_in_bytes = size;
    return result;
}

#define as_bytes_view(object)                                   \
    _Generic((object),                                          \
                 Bytes_View: INTERNAL_bytes_view_as_bytes_view, \
                 Buffer: INTERNAL_buffer_as_bytes_view,         \
                 default: INTERNAL_default_as_bytes_view        \
             )(as_bytes(&object), size_of(object))

struct Arena
{
    String_View name;

    Size reserved_bytes_count;

    Index free_memory_offset;
    Index committed_memory_offset;
};
typedef struct Arena Arena;

internal Arena* INTERNAL_create_arena(const String_View name,
                                      Size number_of_bytes_to_reserve,
                                      Size number_of_bytes_to_commit);

#define create_arena(name, number_of_bytes_to_reserve, number_of_bytes_to_commit) \
    INTERNAL_create_arena(string_view(name), number_of_bytes_to_reserve, number_of_bytes_to_commit)
internal void destroy_arena(Arena* arena);

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

internal inline void append_byte_to_buffer(Arena* arena,
                                           Buffer* buffer,
                                           const Byte byte);

internal inline void append_bytes_to_buffer(Arena* arena,
                                            Buffer* buffer,
                                            const Bytes_View bytes);

internal inline void append_string_to_buffer(Arena* arena,
                                             Buffer* buffer,
                                             const String_View string_view);

internal inline u64
align_up(const u64 value, const u64 alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}
