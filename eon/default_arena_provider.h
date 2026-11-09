#pragma once

#include <eon/memory.h>
#include <eon/string_declarations.h>
#include <eon/types.h>

struct Arena_Provider
{
    s32 dummy_field;
};
typedef struct Arena_Provider Arena_Provider;

internal Arena*
acquire_arena_from_provider(Arena_Provider* provider,
                            const String_View arena_name,
                            const Size number_of_bytes_to_reserve,
                            const Size number_of_bytes_to_commit)
{
    UNUSED(provider);
    return create_arena(arena_name, number_of_bytes_to_reserve, number_of_bytes_to_commit);
}

internal void
request_arena_reset(Arena_Provider* provider, Arena* arena)
{
    UNUSED(provider);
    arena_clear(arena);
}

internal void
release_arena_to_provider(Arena_Provider* provider, Arena* arena)
{
    UNUSED(provider);
    destroy_arena(arena);
}
