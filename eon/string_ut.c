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

REGISTER_TESTS(
    test_string_formatting
)
