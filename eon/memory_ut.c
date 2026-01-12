#include <eon/unit_test.h>

#define TEST_STRING_LENGTH 10

internal String
create_source_string(Arena* arena)
{
    String result = {0};
    result.data = allocate_uninitialized_array(arena, TEST_STRING_LENGTH, char);
    result.length = TEST_STRING_LENGTH;

    for (Index i = 0;
         i < result.length;
         ++i)
    {
        result.data[i] = '0' + (char)i;
    }

    return result;
}

internal String
create_destination_string(Arena* arena)
{
    String result = {0};
    result.data = allocate_array(arena, TEST_STRING_LENGTH, char);
    result.length = TEST_STRING_LENGTH;

    for (Index i = 0;
         i < result.length;
         ++i)
    {
        result.data[i] = 'x';
    }

    return result;
}

internal void
test_copy_memory(Test_Context* context)
{
    // NOTE(vlad): Full copy.
    {
        const String source_string = create_source_string(context->arena);
        String destination_string = create_destination_string(context->arena);

        copy_memory(as_bytes(destination_string.data),
                    as_bytes(source_string.data),
                    source_string.length);

        ASSERT_STRINGS_ARE_EQUAL(destination_string, source_string);
    }

    // NOTE(vlad): Partial copy.
    {
        const String source_string = create_source_string(context->arena);
        String destination_string = create_destination_string(context->arena);

        copy_memory(as_bytes(destination_string.data),
                    as_bytes(source_string.data),
                    5);

        ASSERT_STRINGS_ARE_EQUAL(destination_string, "01234xxxxx");
    }

    // NOTE(vlad): Destination's suffix is the same as a source's prefix.
    {
        String string = create_source_string(context->arena);

        copy_memory(as_bytes(string.data),
                    as_bytes(string.data + 2),
                    5);

        ASSERT_STRINGS_ARE_EQUAL(string, "2345656789");
    }

    // NOTE(vlad): Destination's prefix is the same as a source's suffix.
    {
        String string = create_source_string(context->arena);

        copy_memory(as_bytes(string.data + 2),
                    as_bytes(string.data),
                    5);

        ASSERT_STRINGS_ARE_EQUAL(string, "0101010789");
    }

    // NOTE(vlad): Source and destination are the same.
    {
        String string = create_source_string(context->arena);

        copy_memory(as_bytes(string.data),
                    as_bytes(string.data),
                    string.length);

        ASSERT_STRINGS_ARE_EQUAL(string, "0123456789");
    }
}

internal void
test_move_memory(Test_Context* context)
{
    // NOTE(vlad): Full copy.
    {
        const String source_string = create_source_string(context->arena);
        String destination_string = create_destination_string(context->arena);

        move_memory(as_bytes(destination_string.data),
                    as_bytes(source_string.data),
                    source_string.length);

        ASSERT_STRINGS_ARE_EQUAL(destination_string, source_string);
    }

    // NOTE(vlad): Partial copy.
    {
        const String source_string = create_source_string(context->arena);
        String destination_string = create_destination_string(context->arena);

        move_memory(as_bytes(destination_string.data),
                    as_bytes(source_string.data),
                    5);

        ASSERT_STRINGS_ARE_EQUAL(destination_string, "01234xxxxx");
    }

    // NOTE(vlad): Destination's suffix is the same as a source's prefix.
    {
        String string = create_source_string(context->arena);

        move_memory(as_bytes(string.data),
                    as_bytes(string.data + 2),
                    5);

        ASSERT_STRINGS_ARE_EQUAL(string, "2345656789");
    }

    // NOTE(vlad): Destination's prefix is the same as a source's suffix.
    {
        String string = create_source_string(context->arena);

        move_memory(as_bytes(string.data + 2),
                    as_bytes(string.data),
                    5);

        ASSERT_STRINGS_ARE_EQUAL(string, "0101234789");
    }

    // NOTE(vlad): Source and destination are the same.
    {
        String string = create_source_string(context->arena);

        move_memory(as_bytes(string.data),
                    as_bytes(string.data),
                    string.length);

        ASSERT_STRINGS_ARE_EQUAL(string, "0123456789");
    }
}

internal void
test_fill_memory_with_zeros(Test_Context* context)
{
    {
        String string = create_source_string(context->arena);

        fill_memory_with_zeros(as_bytes(string.data + 5), 5);

        String_View expected_string = {0};
        expected_string.data = "01234\0\0\0\0\0";
        expected_string.length = string.length;

        ASSERT_STRINGS_ARE_EQUAL(string, expected_string);
    }

    {
        String string = create_source_string(context->arena);

        fill_memory_with_zeros(as_bytes(string.data), 5);

        String_View expected_string = {0};
        expected_string.data = "\0\0\0\0\0""56789"; // NOTE(vlad): String concatenation is needed
                                                    //             to prevent C compiler from treating
                                                    //             "\056" as an octal escape sequence.
        expected_string.length = string.length;

        ASSERT_STRINGS_ARE_EQUAL(string, expected_string);
    }
}

REGISTER_TESTS(
    test_copy_memory,
    test_move_memory,
    test_fill_memory_with_zeros
)
