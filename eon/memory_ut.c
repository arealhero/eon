#include <eon/unit_test.h>

#include <eon/sanitizers/asan.h>

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

    // NOTE(vlad): I initially wrote two tests here in which the destination overlaps
    //             with the source. Turns out the compiler is smart enough to see that
    //             we are copying overlapping memory and therefore rewriting the source
    //             data. Despite the 'restrict' keyword in the 'copy_memory' definition
    //             it would emit a code that takes those overlaps into account, thus
    //             the behaviour of 'copy_memory' is unintentionally correct (as though
    //             we'd called 'move_memory') when the test was compiled with '-O2', and
    //             intentionally incorrect with '-O0'.
    //
    //             Futhermore, this triggered the 'memcpy-param-overlap' asan check, which
    //             is fair enough, albeit somewhat unexpected.
    //
    //             We could in theory try to detect the current optimisation level and
    //             optionally enable these tests, but that seems unreasonable to me.
    //             Another option that we could try is removing the 'restrict' keywords
    //             from the definition of 'copy_memory', but I really don't want to do it:
    //             'memcpy' marks its arguments as restricted, because one must use
    //             'memmove' for overlapping regions.
    //
    //             That said, I removed the tests.
    //
    //             Overall, I see this as a win because it showed that
    //               1. the compiler will detect at least some of these incorrect usages
    //                  and emit correct instructions to restore the expected behaviour;
    //               2. asan can detect these erroneous usages and report them.

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

    // NOTE(vlad): Read the comment in 'test_copy_memory' for an explanation of
    //             why we need to disable these tests when asan is enabled.
#if ASAN_ENABLED == 0
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
#endif

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
