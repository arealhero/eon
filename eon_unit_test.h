#pragma once

#include <eon/unit_test.h>

#include "eon_compilation_context.h"

struct Arena_Provider
{
    Test_Context* test_context;
};

internal inline Arena*
acquire_arena_from_provider(struct Arena_Provider* provider,
                            const String_View arena_name,
                            const Size number_of_bytes_to_reserve,
                            const Size number_of_bytes_to_commit)
{
    UNUSED(arena_name, number_of_bytes_to_reserve, number_of_bytes_to_commit);
    return provider->test_context->arena;
}

internal inline void
request_arena_reset(struct Arena_Provider* provider, Arena* arena)
{
    UNUSED(provider, arena);
}

internal inline void
release_arena_to_provider(struct Arena_Provider* provider, Arena* arena)
{
    UNUSED(provider, arena);
}

#define CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE(source_code)           \
    struct Arena_Provider test_arena_provider = {0};                    \
    test_arena_provider.test_context = test_context;                    \
                                                                        \
    Compilation_Context context = {0};                                  \
    do {                                                                \
        Source_File source_file = {0};                                  \
        source_file.filename = string_view("<test-input>");             \
        source_file.code = string_view(source_code);                    \
        create_compilation_context(&context, &test_arena_provider, &source_file); \
    }                                                                   \
    while (0)

#define ASSERT_LOCATION_STRINGS_ARE_EQUAL(location, expected)           \
    ASSERT_STRINGS_ARE_EQUAL(source_location_to_string(&context, location), expected)

#define ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES()                  \
    do                                                                  \
    {                                                                   \
        if (has_diagnostic_messages(&context))                          \
        {                                                               \
            const String dumped_messages = dump_diagnostic_messages(test_context->arena, \
                                                                    &context, \
                                                                    MAX_MESSAGE_LEVEL); \
            ASSERT_STRINGS_ARE_EQUAL(dumped_messages, "");              \
        }                                                               \
    } while (0)

#define ASSERT_TYPE_IS_VALID(type_id)           \
    ASSERT_TRUE(type_id_is_valid(&context, type_id))

// XXX(vlad): Test that type strings are equal?
#define ASSERT_TYPE_IDS_ARE_EQUAL(lhs, rhs) \
    ASSERT_TRUE(type_ids_are_equal(&context, (lhs), (rhs)))

#define ASSERT_TYPE_STRINGS_ARE_EQUAL(type_id, expected)                \
    do                                                                  \
    {                                                                   \
        ASSERT_TYPE_IS_VALID(type_id);                                  \
        ASSERT_STRINGS_ARE_EQUAL(convert_type_to_string(test_context->arena, \
                                                        &context,       \
                                                        type_id),       \
                                 expected);                             \
    }                                                                   \
    while (0)

// FIXME(vlad): Implement 'ASSERT_DIAGNOSTIC_MESSAGES_ARE_EQUAL("<diagnostic-messages>")'.
