#include <eon/unit_test.h>

#include "eon_interpreter.h"
#include "eon_semantics.h"

internal Bool
assert_that_there_are_no_errors(Test_Context* context,
                                const Run_Result* run_result)
{
    if (run_result->status != INTERPRETER_RUN_OK)
    {
        const String_View error_type =
            run_result->status == INTERPRETER_RUN_COMPILE_ERROR
            ? string_view("compile-time")
            : string_view("runtime");
        const String comment = format_string(context->arena,
                                             "    Interpreter returned a {} error:\n"
                                             "        {}",
                                             error_type,
                                             run_result->error);
        MARK_UNIT_TEST_AS_FAILED(comment);
        return false;
    }

    return true;
}

internal void
test_simple_programs(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    return 0;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 0);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    return 1;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 1);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    return 2 + 2 * 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 6);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    variable := 10;"
                                              "    return 2 * variable + 5;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 25);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
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
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 20);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> Int32 = { return 10; }\n"
                                              "main: () -> Int32 = {"
                                              "    return foo();"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 10);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: (argument: Int32) -> Int32 = {"
                                              "    return argument;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        call_info.arguments = allocate_array(context->arena,
                                             1,
                                             Interpreter_Expression_Result);
        call_info.arguments[0].type = AST_TYPE_INT_32;
        call_info.arguments[0].s32_value = 123;
        call_info.arguments_count = 1;
        call_info.arguments_capacity = 1;
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 123);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    if 2 + 2 == 4 { return 1; }"
                                              "    else { return 2; }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 1);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    if 2 + 2 == 4 { return 1; }"
                                              "    else          { return 2; }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 1);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    if 2 + 2 != 4 { return 1; }"
                                              "    else          { return 2; }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 2);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing nested lexical scopes.
    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    a := 10;"
                                              "    if 2 + 2 == 4 {"
                                              "        a := 20;"
                                              "        return a;"
                                              "    }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 20);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    first: mutable _ = 10;"
                                              "    second: mutable _ = 20;"
                                              "    first = 1;"
                                              "    second = 2;"
                                              "    return first + second;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 3);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: () -> Int32 = {"
                                              "    var: mutable _ = 10;"
                                              "    while var != 0"
                                              "    {"
                                              "        var = var - 1;"
                                              "    }"
                                              "    return var;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);

        if (result.status != INTERPRETER_RUN_OK)
        {
            println("Interpreter error: {}", result.error);
        }

        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 0);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing call statement.
    {
        const String_View input = string_view("foo: () -> Int32 = { return 10; }\n"
                                              "main: () -> Int32 = {"
                                              "    foo();"
                                              "    return 0;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);

        if (result.status != INTERPRETER_RUN_OK)
        {
            println("Interpreter error: {}", result.error);
        }

        ASSERT_EQUAL(result.status, INTERPRETER_RUN_OK);
        ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
        ASSERT_EQUAL(result.result.s32_value, 0);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing unary expressions.
    {
        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    a := -10;"
                                                  "    return a;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, -10);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    a := -(10 + 3);"
                                                  "    return a;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, -13);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    a := -10 + 3;"
                                                  "    return a;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, -7);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    a := --10;"
                                                  "    return a;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 10);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }
}

// FIXME(vlad): Move this to something like 'eon_type_system_ut.c'.
internal void
test_type_system(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("main: () -> Float32 = {"
                                              "    return 1.2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        if (!assert_that_there_are_no_errors(context, &result))
        {
            return;
        }
        ASSERT_EQUAL(result.result.type, AST_TYPE_FLOAT_32);
        ASSERT_FLOATS_ARE_EQUAL(result.result.f32_value, 1.2f);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("main: (argument: Float32) -> Float32 = {"
                                              "    return argument + 0.2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

        Call_Info call_info = {0};
        call_info.arguments = allocate_array(context->arena,
                                             1,
                                             Interpreter_Expression_Result);
        call_info.arguments[0].type = AST_TYPE_FLOAT_32;
        call_info.arguments[0].f32_value = 0.1f;
        call_info.arguments_count = 1;
        call_info.arguments_capacity = 1;
        const Run_Result result = interpreter_execute_function(context->arena,
                                                               context->arena,
                                                               &interpreter,
                                                               &ast,
                                                               string_view("main"),
                                                               &call_info);
        if (!assert_that_there_are_no_errors(context, &result))
        {
            return;
        }
        ASSERT_EQUAL(result.result.type, AST_TYPE_FLOAT_32);
        ASSERT_FLOATS_ARE_EQUAL(result.result.f32_value, 0.3f);

        interpreter_destroy(&interpreter);
        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_comparisons(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // NOTE(vlad): Testing basic cases.
    {
        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    if 0 == 0 { return 1; } else { return 0; }"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 1);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    if 0 != 0 { return 1; } else { return 0; }"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 0);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    if 0 > 0 { return 1; } else { return 0; }"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 0);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    if 0 >= 0 { return 1; } else { return 0; }"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 1);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    if 0 < 0 { return 1; } else { return 0; }"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 0);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    if 0 <= 0 { return 1; } else { return 0; }"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 1);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing various other cases.
    {
        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    if 0 != 1 { return 1; } else { return 0; }"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            Interpreter interpreter = {0};
            interpreter_create(&interpreter, context->arena);

            Call_Info call_info = {0};
            const Run_Result result = interpreter_execute_function(context->arena,
                                                                   context->arena,
                                                                   &interpreter,
                                                                   &ast,
                                                                   string_view("main"),
                                                                   &call_info);
            if (!assert_that_there_are_no_errors(context, &result))
            {
                return;
            }
            ASSERT_EQUAL(result.result.type, AST_TYPE_INT_32);
            ASSERT_FLOATS_ARE_EQUAL(result.result.s32_value, 1);

            interpreter_destroy(&interpreter);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }
}

internal void
test_compile_time_errors(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // TODO(vlad): Test that there are only one function definition. Maybe we should test this
    //             after parsing and before executing.

    {
        const String_View input = string_view("foo: () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        Interpreter interpreter = {0};
        interpreter_create(&interpreter, context->arena);

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

        clear_errors(&errors);
    }
}

REGISTER_TESTS(
    test_simple_programs,
    test_type_system,
    test_comparisons,
    test_compile_time_errors
)

#include "eon_errors.c"
#include "eon_interpreter.c"
#include "eon_lexer.c"
#include "eon_parser.c"
#include "eon_semantics.c"
