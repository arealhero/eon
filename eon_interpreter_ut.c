#include <eon/unit_test.h>

#include "eon_interpreter.h"

internal void
test_simple_programs(Test_Context* context)
{
    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    return 0;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 0);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    return 1;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 1);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    return 2 + 2 * 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 6);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    variable := 10;"
                                              "    return 2 * variable + 5;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 25);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    a := 10;"
                                              "    b := a / 2;"
                                              "    return a + b * (a + b + 3) / 9;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 20);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("foo: () -> Int32 = { return 10; }\n"
                                              "main: () -> Int32 = {"
                                              "    return foo();"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 10);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("main: (argument: Int32) -> Int32 = {"
                                              "    return argument;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        call_info.arguments = allocate_array(context->arena, 1, s32);
        call_info.arguments[0] = 123;
        call_info.arguments_count = 1;
        call_info.arguments_capacity = 1;
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 123);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

internal void
test_compile_time_errors(Test_Context* context)
{
    // TODO(vlad): Test that there are only one function definition. Maybe we should test this
    //             after parsing and before executing.

    {
        const String_View input = string_view("foo: () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(context->arena, context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(parser.errors.errors_count, 0);

        Interpreter interpreter = {0};
        interpreter_create(&interpreter);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_COMPILE_ERROR);
        ASSERT_STRINGS_ARE_EQUAL(result.error, "Function 'main' is not defined");

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

REGISTER_TESTS(
    test_simple_programs,
    test_compile_time_errors
)

#include "eon_interpreter.c"
#include "eon_errors.c"
#include "eon_lexer.c"
#include "eon_parser.c"
