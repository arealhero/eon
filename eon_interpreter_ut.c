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

        const Run_Result result = interpreter_run(context->arena,
                                                  context->arena,
                                                  &interpreter,
                                                  &ast,
                                                  string_view("main"));
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

        const Run_Result result = interpreter_run(context->arena,
                                                  context->arena,
                                                  &interpreter,
                                                  &ast,
                                                  string_view("main"));
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

        const Run_Result result = interpreter_run(context->arena,
                                                  context->arena,
                                                  &interpreter,
                                                  &ast,
                                                  string_view("main"));
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

        const Run_Result result = interpreter_run(context->arena,
                                                  context->arena,
                                                  &interpreter,
                                                  &ast,
                                                  string_view("main"));
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

        const Run_Result result = interpreter_run(context->arena,
                                                  context->arena,
                                                  &interpreter,
                                                  &ast,
                                                  string_view("main"));
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.return_value, 20);

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
        const String_View input = string_view("main: (argument: Int32) -> Int32 = {"
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

        const Run_Result result = interpreter_run(context->arena,
                                                  context->arena,
                                                  &interpreter,
                                                  &ast,
                                                  string_view("main"));
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_COMPILE_ERROR);
        ASSERT_STRINGS_ARE_EQUAL(result.error,
                                 "Execution of functions with arguments is not yet supported"
                                 " (function 'main' requires 1 argument)");

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

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

        const Run_Result result = interpreter_run(context->arena,
                                                  context->arena,
                                                  &interpreter,
                                                  &ast,
                                                  string_view("main"));
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
