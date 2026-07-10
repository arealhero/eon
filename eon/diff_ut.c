#include <eon/unit_test.h>

#include "diff.h"

internal void
test_simple_line_diffs(Test_Context* test_context)
{
    {
        const String_View lhs = string_view("abc");
        const String_View rhs = string_view("abc");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 0);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff), "");
    }

    {
        const String_View lhs = string_view("abc");
        const String_View rhs = string_view("123");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 1);
        ASSERT_EQUAL(diff.lines[0], 0);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff),
                                 "1\n"
                                 "< abc\n"
                                 "> 123\n");
    }

    {
        const String_View lhs = string_view("");
        const String_View rhs = string_view("123");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 1);
        ASSERT_EQUAL(diff.lines[0], 0);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff),
                                 "1\n"
                                 "< \n"
                                 "> 123\n");
    }

    {
        const String_View lhs = string_view("abc");
        const String_View rhs = string_view("");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 1);
        ASSERT_EQUAL(diff.lines[0], 0);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff),
                                 "1\n"
                                 "< abc\n"
                                 "> \n");
    }

    {
        const String_View lhs = string_view("hello\nworld");
        const String_View rhs = string_view("hello\nWORLD");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 1);
        ASSERT_EQUAL(diff.lines[0], 1);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff),
                                 "2\n"
                                 "< world\n"
                                 "> WORLD\n");
    }

    {
        const String_View lhs = string_view("hello\nworld");
        const String_View rhs = string_view("HELLO\nworld");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 1);
        ASSERT_EQUAL(diff.lines[0], 0);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff),
                                 "1\n"
                                 "< hello\n"
                                 "> HELLO\n");
    }

    {
        const String_View lhs = string_view("hello");
        const String_View rhs = string_view("hello\nworld");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 1);
        ASSERT_EQUAL(diff.lines[0], 1);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff),
                                 "2\n"
                                 "< \n"
                                 "> world\n");
    }
}

internal void
test_simple_regressions(Test_Context* test_context)
{
    {
        const String_View lhs = string_view("hello\nhello\nworld\n\nhello");
        const String_View rhs = string_view("HELLO\nHELLO\nworld\n\nHELLO");

        const Diff diff = calculate_line_diff(test_context->arena, lhs, rhs);

        ASSERT_EQUAL(diff.lines_count, 3);
        ASSERT_EQUAL(diff.lines[0], 0);
        ASSERT_EQUAL(diff.lines[1], 1);
        ASSERT_EQUAL(diff.lines[2], 4);
        ASSERT_STRINGS_ARE_EQUAL(line_diff_to_string(test_context->arena, &diff),
                                 "1\n"
                                 "< hello\n"
                                 "> HELLO\n"
                                 "2\n"
                                 "< hello\n"
                                 "> HELLO\n"
                                 "5\n"
                                 "< hello\n"
                                 "> HELLO\n");
    }
}

REGISTER_TESTS(
    test_simple_line_diffs,
    test_simple_regressions
)

#include "diff.c"
