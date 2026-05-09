#include <eon/unit_test.h>

internal void
test_string_formatting(Test_Context* test_context)
{
    {
        const String_View format_string = string_view("Formatting without arguments");
        const String result = format_string(test_context->arena, format_string);
        ASSERT_STRINGS_ARE_EQUAL(result, format_string);
    }

    {
        const String result = format_string(test_context->arena, "Hello, {}!", "world");
        ASSERT_STRINGS_ARE_EQUAL(result, "Hello, world!");
    }
}

internal void
test_signed_integers_formatting(Test_Context* test_context)
{
    // NOTE(vlad): Signed integers.

    {
        const String result = format_string(test_context->arena, "{}", (s8)123);
        ASSERT_STRINGS_ARE_EQUAL(result, "123");
    }

    {
        const String result = format_string(test_context->arena, "{}", (s8)(-1));
        ASSERT_STRINGS_ARE_EQUAL(result, "-1");
    }

    {
        const String result = format_string(test_context->arena, "{}", (s16)(255));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    // NOTE(vlad): Max values of signed integers.

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(s8));
        ASSERT_STRINGS_ARE_EQUAL(result, "127");
    }

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(s16));
        ASSERT_STRINGS_ARE_EQUAL(result, "32767");
    }

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(s32));
        ASSERT_STRINGS_ARE_EQUAL(result, "2147483647");
    }

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(s64));
        ASSERT_STRINGS_ARE_EQUAL(result, "9223372036854775807");
    }

    // NOTE(vlad): Min values of signed integers.

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(s8));
        ASSERT_STRINGS_ARE_EQUAL(result, "-128");
    }

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(s16));
        ASSERT_STRINGS_ARE_EQUAL(result, "-32768");
    }

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(s32));
        ASSERT_STRINGS_ARE_EQUAL(result, "-2147483648");
    }

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(s64));
        ASSERT_STRINGS_ARE_EQUAL(result, "-9223372036854775808");
    }
}

internal void
test_unsigned_integers_formatting(Test_Context* test_context)
{
    // NOTE(vlad): Signed integers.

    {
        const String result = format_string(test_context->arena, "{}", (u8)123);
        ASSERT_STRINGS_ARE_EQUAL(result, "123");
    }

    {
        const String result = format_string(test_context->arena, "{}", (u8)(255));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    {
        const String result = format_string(test_context->arena, "{}", (u8)(0));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(test_context->arena, "{}", (u16)(255));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    // NOTE(vlad): Max values of signed integers.

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(u8));
        ASSERT_STRINGS_ARE_EQUAL(result, "255");
    }

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(u16));
        ASSERT_STRINGS_ARE_EQUAL(result, "65535");
    }

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(u32));
        ASSERT_STRINGS_ARE_EQUAL(result, "4294967295");
    }

    {
        const String result = format_string(test_context->arena, "{}", MAX_VALUE(u64));
        ASSERT_STRINGS_ARE_EQUAL(result, "18446744073709551615");
    }

    // NOTE(vlad): Min values of signed integers.

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(u8));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(u16));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(u32));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(test_context->arena, "{}", MIN_VALUE(u64));
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    // NOTE(vlad): Print integers in hex.
    {
        const String result = format_string(test_context->arena, "{base:16}", 123);
        ASSERT_STRINGS_ARE_EQUAL(result, "7b");
    }

    // NOTE(vlad): Padding tests.
    {
        const String result = format_string(test_context->arena,
                                            "{left-pad-count: 10}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "         0");
    }

    {
        const String result = format_string(test_context->arena,
                                            "{left-pad-count: 0}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(test_context->arena,
                                            "{left-pad-count: 1}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "0");
    }

    {
        const String result = format_string(test_context->arena,
                                            "{left-pad-count: 10, left-pad-char: x}",
                                            0);
        ASSERT_STRINGS_ARE_EQUAL(result, "xxxxxxxxx0");
    }

    {
        const String result = format_string(test_context->arena,
                                            "{base: 16, left-pad-count: 4}",
                                            123);
        ASSERT_STRINGS_ARE_EQUAL(result, "  7b");
    }

    {
        const String result = format_string(test_context->arena,
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
    //     const String result = format_string(test_context->arena,
    //                                         "{"
    //                                         "    base: 16,"
    //                                         "    uppercase-hexadecimal: true,"
    //                                         "}", 123);
    //     ASSERT_STRINGS_ARE_EQUAL(result, "7B");
    // }
}

internal void
test_formatting_corner_cases(Test_Context* test_context)
{
    // NOTE(vlad): Format-like string without closing brace.
    {
        const String result = format_string(test_context->arena, "{base:16");
        ASSERT_STRINGS_ARE_EQUAL(result, "{base:16");
    }

    {
        const String result = format_string(test_context->arena, "{{");
        ASSERT_STRINGS_ARE_EQUAL(result, "{{");
    }
}

internal void
test_floating_point_numbers_formatting(Test_Context* test_context)
{
    // NOTE(vlad): Testing f32 formatting.
    {
        {
            const String result = format_string(test_context->arena, "{}", 1.0f);
            ASSERT_STRINGS_ARE_EQUAL(result, "1.00");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 2}", 1.0f);
            ASSERT_STRINGS_ARE_EQUAL(result, "1.00");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 1}", 1.2f);
            ASSERT_STRINGS_ARE_EQUAL(result, "1.2");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 2}", 1.2f);
            ASSERT_STRINGS_ARE_EQUAL(result, "1.20");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 1}", 1.45f);
            ASSERT_STRINGS_ARE_EQUAL(result, "1.5");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 1}", -1.45f);
            ASSERT_STRINGS_ARE_EQUAL(result, "-1.5");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 1}", -0.05f);
            ASSERT_STRINGS_ARE_EQUAL(result, "-0.1");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 6}", 0.1f + 0.2f);
            ASSERT_STRINGS_ARE_EQUAL(result, "0.300000");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 17}", 0.1f + 0.2f);
            ASSERT_STRINGS_ARE_EQUAL(result, "0.30000000817692672");
        }
    }

    // NOTE(vlad): Testing f64 formatting.
    {
        {
            const String result = format_string(test_context->arena, "{precision: 1}", -0.05);
            ASSERT_STRINGS_ARE_EQUAL(result, "-0.1");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 17}", 0.1 + 0.2);
            ASSERT_STRINGS_ARE_EQUAL(result, "0.30000000000000004");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 6}", 0.63486);
            ASSERT_STRINGS_ARE_EQUAL(result, "0.634860");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 6}", -0.529565257);
            ASSERT_STRINGS_ARE_EQUAL(result, "-0.529565");
        }
    }

    // NOTE(vlad): Testing rounding.
    {
        {
            const String result = format_string(test_context->arena, "{precision: 6}", -4.9999989999999991);
            ASSERT_STRINGS_ARE_EQUAL(result, "-4.999999");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 1}", 4.95);
            ASSERT_STRINGS_ARE_EQUAL(result, "5.0");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 1}", -4.95);
            ASSERT_STRINGS_ARE_EQUAL(result, "-5.0");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 6}", -4.9999999999999991);
            ASSERT_STRINGS_ARE_EQUAL(result, "-5.000000");
        }

        {
            const String result = format_string(test_context->arena, "{precision: 6}", 2.0799999999999992);
            ASSERT_STRINGS_ARE_EQUAL(result, "2.080000");
        }
    }
}

internal void
test_integer_parsing(Test_Context* test_context)
{
    // NOTE(vlad): Basic tests.
    {
        {
            s8 result;
            ASSERT_TRUE(parse_integer(string_view("123"), &result));
            ASSERT_EQUAL(result, 123);
        }

        {
            s32 result;
            ASSERT_TRUE(parse_integer(string_view("124534"), &result));
            ASSERT_EQUAL(result, 124534);
        }

        {
            s32 result;
            ASSERT_TRUE(parse_integer(string_view("+1"), &result));
            ASSERT_EQUAL(result, 1);
        }

        {
            s32 result;
            ASSERT_TRUE(parse_integer(string_view("-1"), &result));
            ASSERT_EQUAL(result, -1);
        }
    }

    // NOTE(vlad): Max values of signed integers.
    {
        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(s8));

            s8 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(s8));
        }

        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(s16));

            s16 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(s16));
        }

        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(s32));

            s32 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(s32));
        }

        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(s64));

            s64 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(s64));
        }
    }

    // NOTE(vlad): Min values of signed integers.

    {
        const String input = format_string(test_context->arena, "{}", MIN_VALUE(s8));

        s8 result;
        ASSERT_TRUE(parse_integer(string_view(input), &result));
        ASSERT_EQUAL(result, MIN_VALUE(s8));
    }

    {
        const String input = format_string(test_context->arena, "{}", MIN_VALUE(s16));

        s16 result;
        ASSERT_TRUE(parse_integer(string_view(input), &result));
        ASSERT_EQUAL(result, MIN_VALUE(s16));
    }

    {
        const String input = format_string(test_context->arena, "{}", MIN_VALUE(s32));

        s32 result;
        ASSERT_TRUE(parse_integer(string_view(input), &result));
        ASSERT_EQUAL(result, MIN_VALUE(s32));
    }

    {
        const String input = format_string(test_context->arena, "{}", MIN_VALUE(s64));

        s64 result;
        ASSERT_TRUE(parse_integer(string_view(input), &result));
        ASSERT_EQUAL(result, MIN_VALUE(s64));
    }

    // NOTE(vlad): Max values of unsigned integers.
    {
        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(u8));

            u8 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(u8));
        }

        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(u16));

            u16 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(u16));
        }

        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(u32));

            u32 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(u32));
        }

        {
            const String input = format_string(test_context->arena, "{}", MAX_VALUE(u64));

            u64 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MAX_VALUE(u64));
        }
    }

    // NOTE(vlad): Min values of unsigned integers.
    {
        {
            const String input = format_string(test_context->arena, "{}", MIN_VALUE(u8));

            u8 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MIN_VALUE(u8));
        }

        {
            const String input = format_string(test_context->arena, "{}", MIN_VALUE(u16));

            u16 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MIN_VALUE(u16));
        }

        {
            const String input = format_string(test_context->arena, "{}", MIN_VALUE(u32));

            u32 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MIN_VALUE(u32));
        }

        {
            const String input = format_string(test_context->arena, "{}", MIN_VALUE(u64));

            u64 result;
            ASSERT_TRUE(parse_integer(string_view(input), &result));
            ASSERT_EQUAL(result, MIN_VALUE(u64));
        }
    }

    // NOTE(vlad): Testing signed integer overflows.
    {
        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(s8) / 10,
                                               (MAX_VALUE(s8) % 10) + 1);

            s8 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(s16) / 10,
                                               (MAX_VALUE(s16) % 10) + 1);

            s16 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(s32) / 10,
                                               (MAX_VALUE(s32) % 10) + 1);

            s32 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(s64) / 10,
                                               (MAX_VALUE(s64) % 10) + 1);

            s64 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }
    }

    // NOTE(vlad): Testing signed integer underflows.
    {
        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MIN_VALUE(s8) / 10,
                                               ABS(MIN_VALUE(s8) % 10) + 1);

            s8 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MIN_VALUE(s16) / 10,
                                               ABS(MIN_VALUE(s16) % 10) + 1);

            s16 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MIN_VALUE(s32) / 10,
                                               ABS(MIN_VALUE(s32) % 10) + 1);

            s32 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MIN_VALUE(s64) / 10,
                                               ABS(MIN_VALUE(s64) % 10) + 1);

            s64 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }
    }

    // NOTE(vlad): Testing unsigned integer overflows.
    {
        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(u8) / 10,
                                               (MAX_VALUE(u8) % 10) + 1);

            u8 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(u16) / 10,
                                               (MAX_VALUE(u16) % 10) + 1);

            u16 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(u32) / 10,
                                               (MAX_VALUE(u32) % 10) + 1);

            u32 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }

        {
            const String input = format_string(test_context->arena,
                                               "{}{}",
                                               MAX_VALUE(u64) / 10,
                                               (MAX_VALUE(u64) % 10) + 1);

            u64 result;
            ASSERT_FALSE(parse_integer(string_view(input), &result));
        }
    }

    // NOTE(vlad): Testing unsigned integer underflows.
    {
        {
            u8 result;
            ASSERT_FALSE(parse_integer(string_view("-1"), &result));
        }

        {
            u16 result;
            ASSERT_FALSE(parse_integer(string_view("-1"), &result));
        }

        {
            u32 result;
            ASSERT_FALSE(parse_integer(string_view("-1"), &result));
        }

        {
            u64 result;
            ASSERT_FALSE(parse_integer(string_view("-1"), &result));
        }
    }

    // NOTE(vlad): Testing various failure cases.
    {
        {
            s8 result;
            ASSERT_FALSE(parse_integer(string_view("abc"), &result));
        }

        {
            s8 result;
            ASSERT_FALSE(parse_integer(string_view("--1"), &result));
        }

        {
            s8 result;
            ASSERT_FALSE(parse_integer(string_view("-+1"), &result));
        }

        {
            s8 result;
            ASSERT_FALSE(parse_integer(string_view("-1-"), &result));
        }

        {
            s8 result;
            ASSERT_FALSE(parse_integer(string_view("+"), &result));
        }

        {
            s8 result;
            ASSERT_FALSE(parse_integer(string_view("-"), &result));
        }
    }
}

internal void
test_floating_point_numbers_parsing(Test_Context* test_context)
{
    // NOTE(vlad): Basic tests.
    {
        {
            f32 result;
            ASSERT_TRUE(parse_float(string_view("123"), &result));
            ASSERT_FLOATS_ARE_EQUAL(result, 123.0f);
        }

        {
            f32 result;
            ASSERT_TRUE(parse_float(string_view("1.5"), &result));
            ASSERT_FLOATS_ARE_EQUAL(result, 1.5f);
        }

        {
            f32 result;
            ASSERT_TRUE(parse_float(string_view("0.2"), &result));
            ASSERT_FLOATS_ARE_EQUAL(result, 0.2f);
        }

        {
            f32 result;
            ASSERT_TRUE(parse_float(string_view("-1.234567"), &result));
            ASSERT_FLOATS_ARE_EQUAL(result, -1.234567f);
        }

        {
            f64 result;
            ASSERT_TRUE(parse_float(string_view("-1.234567"), &result));
            ASSERT_FLOATS_ARE_EQUAL(result, -1.234567);
        }
    }
}

REGISTER_TESTS(
    test_string_formatting,
    test_signed_integers_formatting,
    test_unsigned_integers_formatting,
    test_formatting_corner_cases,
    test_floating_point_numbers_formatting,
    test_integer_parsing,
    test_floating_point_numbers_parsing
)
