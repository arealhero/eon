#include <eon/unit_test.h>

internal void
test_string_formatting(Test_Context* context)
{
    {
        const String_View format_string = string_view("Formatting without arguments");
        const String result = format_string(context->arena, format_string);
        ASSERT_STRINGS_ARE_EQUAL(result, format_string);
    }

    {
        const String result = format_string(context->arena, "Hello, {}!", "world");
        ASSERT_STRINGS_ARE_EQUAL(result, "Hello, world!");
    }
}

internal void
test_signed_integers_formatting(Test_Context* context)
{
    // NOTE(vlad): Signed integers.

    {
        const String result = format_string(context->arena, "{}", (s8)123);
        ASSERT_STRINGS_ARE_EQUAL(result, "123");
    }

    {
        const String result = format_string(context->arena, "{}", (s8)(255));
        ASSERT_STRINGS_ARE_EQUAL(result, "-1");
    }

    {
        const String result = format_string(context->arena, "{}", (s16)(255));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    // NOTE(vlad): Max values of signed integers.

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(s8));
        ASSERT_STRINGS_ARE_EQUAL(result, "127");
    }

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(s16));
        ASSERT_STRINGS_ARE_EQUAL(result, "32767");
    }

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(s32));
        ASSERT_STRINGS_ARE_EQUAL(result, "2147483647");
    }

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(s64));
        ASSERT_STRINGS_ARE_EQUAL(result, "9223372036854775807");
    }

    // NOTE(vlad): Min values of signed integers.

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(s8));
        ASSERT_STRINGS_ARE_EQUAL(result, "-128");
    }

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(s16));
        ASSERT_STRINGS_ARE_EQUAL(result, "-32768");
    }

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(s32));
        ASSERT_STRINGS_ARE_EQUAL(result, "-2147483648");
    }

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(s64));
        ASSERT_STRINGS_ARE_EQUAL(result, "-9223372036854775808");
    }
}

internal void
test_unsigned_integers_formatting(Test_Context* context)
{
    // NOTE(vlad): Signed integers.

    {
        const String result = format_string(context->arena, "{}", (u8)123);
        ASSERT_STRINGS_ARE_EQUAL(result, "123");
    }

    {
        const String result = format_string(context->arena, "{}", (u8)(255));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    {
        const String result = format_string(context->arena, "{}", (u8)(256));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(context->arena, "{}", (u16)(255));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    // NOTE(vlad): Max values of signed integers.

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(u8));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(u16));
        ASSERT_STRINGS_ARE_EQUAL(result, "65535");
    }

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(u32));
        ASSERT_STRINGS_ARE_EQUAL(result, "4294967295");
    }

    {
        const String result = format_string(context->arena, "{}", MAX_VALUE(u64));
        ASSERT_STRINGS_ARE_EQUAL(result, "18446744073709551615");
    }

    // NOTE(vlad): Min values of signed integers.

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(u8));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(u16));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(u32));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(context->arena, "{}", MIN_VALUE(u64));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    // NOTE(vlad): Print integers in hex.
    {
        const String result = format_string(context->arena, "{base:16}", 123);
        ASSERT_STRINGS_ARE_EQUAL(result, "7b");
    }

    // NOTE(vlad): Padding tests.
    {
        const String result = format_string(context->arena,
                                            "{left-pad-count: 10}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "         0");
    }

    {
        const String result = format_string(context->arena,
                                            "{left-pad-count: 0}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(context->arena,
                                            "{left-pad-count: 1}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(context->arena,
                                            "{left-pad-count: 10, left-pad-char: x}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "xxxxxxxxx0");
    }

    {
        const String result = format_string(context->arena,
                                            "{base: 16, left-pad-count: 4}",
                                            123);
        ASSERT_STRINGS_ARE_EQUAL(result, "  7b");
    }

    {
        const String result = format_string(context->arena,
                                            "{"
                                            "  base: 16,"
                                            "  left-pad-count: 4,"
                                            "  left-pad-char: 0,"
                                            "}",
                                            123);
        ASSERT_STRINGS_ARE_EQUAL(result, "007b");
    }

    // TODO(vlad): Support something like this?
    // {
    //     const String result = format_string(context->arena,
    //                                         "{"
    //                                         "    base: 16,"
    //                                         "    uppercase-hexadecimal: true,"
    //                                         "}", 123);
    //     ASSERT_STRINGS_ARE_EQUAL(result, "7B");
    // }
}

internal void
test_corner_cases(Test_Context* context)
{
    // NOTE(vlad): Format-like string without closing brace.
    {
        const String result = format_string(context->arena, "{base:16");
        ASSERT_STRINGS_ARE_EQUAL(result, "{base:16");
    }

    {
        const String result = format_string(context->arena, "{{");
        ASSERT_STRINGS_ARE_EQUAL(result, "{{");
    }
}

REGISTER_TESTS(
    test_string_formatting,
    test_signed_integers_formatting,
    test_unsigned_integers_formatting,
    test_corner_cases
)
