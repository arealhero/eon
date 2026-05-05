#include <eon/unit_test.h>

#include "eon_parser.h"

internal void
test_function_definitions_parsing(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    {
        const String_View input = string_view("foo: () -> Bool = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "Bool");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: (argument: Int32) -> void = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 1);

        const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
        ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "argument");

        const Ast_Type* parameter_type = parameter->type;
        ASSERT_EQUAL(parameter_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(parameter_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(parameter_type->named_type.token.lexeme, "Int32");

        ASSERT_FALSE(parameter->has_default_value);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> () -> void = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_EQUAL(return_type->function.parameters_count, 0);
        ASSERT_EQUAL(return_type->function.return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->function.return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->function.return_type->named_type.token.lexeme, "void");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> * () -> void = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_POINTER);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_EQUAL(return_type->pointer.pointed_to->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(return_type->pointer.pointed_to->is_mutable);
        ASSERT_EQUAL(return_type->pointer.pointed_to->function.parameters_count, 0);
        ASSERT_EQUAL(return_type->pointer.pointed_to->function.return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->pointer.pointed_to->function.return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->pointer.pointed_to->function.return_type->named_type.token.lexeme,
                                 "void");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: (first: Int32, second: Some_Type) -> void = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 2);

        {
            const Ast_Function_Parameter* first_parameter = &function_type->function.parameters[0];
            ASSERT_STRINGS_ARE_EQUAL(first_parameter->name.token.lexeme, "first");

            const Ast_Type* first_parameter_type = first_parameter->type;
            ASSERT_EQUAL(first_parameter_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(first_parameter_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(first_parameter_type->named_type.token.lexeme, "Int32");

            ASSERT_FALSE(first_parameter->has_default_value);
        }

        {
            const Ast_Function_Parameter* second_parameter = &function_type->function.parameters[1];
            ASSERT_STRINGS_ARE_EQUAL(second_parameter->name.token.lexeme, "second");

            const Ast_Type* second_parameter_type = second_parameter->type;
            ASSERT_EQUAL(second_parameter_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(second_parameter_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(second_parameter_type->named_type.token.lexeme, "Some_Type");

            ASSERT_FALSE(second_parameter->has_default_value);
        }

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Multiple function definitions.
    {
        const String_View input = string_view("foo: () -> void = {}\n"
                                              "bar: (arg: Type) -> Other_Type = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 2);

        {
            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");
        }

        {
            const Ast_Function_Definition* definition = &context.ast.function_definitions[1];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "bar");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 1);

            {
                const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "arg");

                ASSERT_EQUAL(parameter->type->kind, AST_TYPE_NAME);
                ASSERT_FALSE(parameter->type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(parameter->type->named_type.token.lexeme, "Type");

                ASSERT_FALSE(parameter->has_default_value);
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "Other_Type");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> Float32 = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "Float32");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing mutable arguments and return types.
    {
        {
            const String_View input = string_view("foo: (first: mutable Int32, second: Some_Type) -> void = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 2);

            {
                const Ast_Function_Parameter* first_parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(first_parameter->name.token.lexeme, "first");

                const Ast_Type* first_parameter_type = first_parameter->type;
                ASSERT_EQUAL(first_parameter_type->kind, AST_TYPE_NAME);
                ASSERT_TRUE(first_parameter_type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(first_parameter_type->named_type.token.lexeme, "Int32");

                ASSERT_FALSE(first_parameter->has_default_value);
            }

            {
                const Ast_Function_Parameter* second_parameter = &function_type->function.parameters[1];
                ASSERT_STRINGS_ARE_EQUAL(second_parameter->name.token.lexeme, "second");

                const Ast_Type* second_parameter_type = second_parameter->type;
                ASSERT_EQUAL(second_parameter_type->kind, AST_TYPE_NAME);
                ASSERT_FALSE(second_parameter_type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(second_parameter_type->named_type.token.lexeme, "Some_Type");

                ASSERT_FALSE(second_parameter->has_default_value);
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (first: Int32, second: mutable Some_Type) -> void = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 2);

            {
                const Ast_Function_Parameter* first_parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(first_parameter->name.token.lexeme, "first");

                const Ast_Type* first_parameter_type = first_parameter->type;
                ASSERT_EQUAL(first_parameter_type->kind, AST_TYPE_NAME);
                ASSERT_FALSE(first_parameter_type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(first_parameter_type->named_type.token.lexeme, "Int32");

                ASSERT_FALSE(first_parameter->has_default_value);
            }

            {
                const Ast_Function_Parameter* second_parameter = &function_type->function.parameters[1];
                ASSERT_STRINGS_ARE_EQUAL(second_parameter->name.token.lexeme, "second");

                const Ast_Type* second_parameter_type = second_parameter->type;
                ASSERT_EQUAL(second_parameter_type->kind, AST_TYPE_NAME);
                ASSERT_TRUE(second_parameter_type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(second_parameter_type->named_type.token.lexeme, "Some_Type");

                ASSERT_FALSE(second_parameter->has_default_value);
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (first: mutable Int32, second: mutable Some_Type) -> void = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 2);

            {
                const Ast_Function_Parameter* first_parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(first_parameter->name.token.lexeme, "first");

                const Ast_Type* first_parameter_type = first_parameter->type;
                ASSERT_EQUAL(first_parameter_type->kind, AST_TYPE_NAME);
                ASSERT_TRUE(first_parameter_type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(first_parameter_type->named_type.token.lexeme, "Int32");

                ASSERT_FALSE(first_parameter->has_default_value);
            }

            {
                const Ast_Function_Parameter* second_parameter = &function_type->function.parameters[1];
                ASSERT_STRINGS_ARE_EQUAL(second_parameter->name.token.lexeme, "second");

                const Ast_Type* second_parameter_type = second_parameter->type;
                ASSERT_EQUAL(second_parameter_type->kind, AST_TYPE_NAME);
                ASSERT_TRUE(second_parameter_type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(second_parameter_type->named_type.token.lexeme, "Some_Type");

                ASSERT_FALSE(second_parameter->has_default_value);
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> mutable Float32 = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_TRUE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "Float32");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> mutable * Float32 = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_POINTER);
            ASSERT_TRUE(return_type->is_mutable);
            ASSERT_EQUAL(return_type->pointer.pointed_to->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->pointer.pointed_to->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->pointer.pointed_to->named_type.token.lexeme, "Float32");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> * mutable Float32 = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_POINTER);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_EQUAL(return_type->pointer.pointed_to->kind, AST_TYPE_NAME);
            ASSERT_TRUE(return_type->pointer.pointed_to->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->pointer.pointed_to->named_type.token.lexeme, "Float32");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> mutable * mutable Float32 = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_POINTER);
            ASSERT_TRUE(return_type->is_mutable);
            ASSERT_EQUAL(return_type->pointer.pointed_to->kind, AST_TYPE_NAME);
            ASSERT_TRUE(return_type->pointer.pointed_to->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->pointer.pointed_to->named_type.token.lexeme, "Float32");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> mutable * () -> void = {}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_POINTER);
            ASSERT_TRUE(return_type->is_mutable);
            ASSERT_EQUAL(return_type->pointer.pointed_to->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(return_type->pointer.pointed_to->is_mutable);
            ASSERT_EQUAL(return_type->pointer.pointed_to->function.parameters_count, 0);
            ASSERT_EQUAL(return_type->pointer.pointed_to->function.return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->pointer.pointed_to->function.return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->pointer.pointed_to->function.return_type->named_type.token.lexeme, "void");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }
    }
}

internal void
test_variable_definitions_parsing(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    // NOTE(vlad): Variable without initialisation.
    {
        const String_View input = string_view("foo: () -> void = { variable: Int32; }");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(definition->body.statements_count, 1);

        const Ast_Statement* statement = &definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
        ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.name.token.lexeme, "variable");
        ASSERT_EQUAL(statement->variable_definition.type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(statement->variable_definition.type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.type->named_type.token.lexeme, "Int32");
        ASSERT_FALSE(statement->variable_definition.has_initial_value);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Variable with initialisation.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    variable: Int32 = 123;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "variable");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(definition->type->named_type.token.lexeme, "Int32");

        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Variable with inferred type.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    variable := 123;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "variable");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);

        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Multiple variable definitions.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 123;"
                                              "    var2: String_View;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var1");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var2");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(definition->type->named_type.token.lexeme, "String_View");
            ASSERT_FALSE(definition->has_initial_value);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): String_View initialisation.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var: String_View = \"Hello\";"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(definition->type->named_type.token.lexeme, "String_View");
        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_STRING_LITERAL);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.string_literal.value, "Hello");
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.string_literal.token.lexeme, "\"Hello\"");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Variable initialisation via other variable.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 123;"
                                              "    var2 := var1;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var1");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var2");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.identifier.token.lexeme, "var1");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Pointer declaration.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    pointer: * Int32;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "pointer");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_POINTER);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_EQUAL(definition->type->pointer.pointed_to->kind, AST_TYPE_NAME);
        ASSERT_FALSE(definition->type->pointer.pointed_to->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(definition->type->pointer.pointed_to->named_type.token.lexeme, "Int32");
        ASSERT_FALSE(definition->has_initial_value);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Pointer to a pointer declaration.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    pointer: ** Int32;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "pointer");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_POINTER);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_EQUAL(definition->type->pointer.pointed_to->kind, AST_TYPE_POINTER);
        ASSERT_FALSE(definition->type->pointer.pointed_to->is_mutable);
        ASSERT_EQUAL(definition->type->pointer.pointed_to->pointer.pointed_to->kind, AST_TYPE_NAME);
        ASSERT_FALSE(definition->type->pointer.pointed_to->pointer.pointed_to->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(definition->type->pointer.pointed_to->pointer.pointed_to->named_type.token.lexeme, "Int32");
        ASSERT_FALSE(definition->has_initial_value);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing mutable variables.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    variable: mutable Int32;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(definition->body.statements_count, 1);

            const Ast_Statement* statement = &definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
            ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.name.token.lexeme, "variable");
            ASSERT_EQUAL(statement->variable_definition.type->kind, AST_TYPE_NAME);
            ASSERT_TRUE(statement->variable_definition.type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.type->named_type.token.lexeme, "Int32");
            ASSERT_FALSE(statement->variable_definition.has_initial_value);

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    pointer: mutable * Int32;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "pointer");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_POINTER);
            ASSERT_TRUE(definition->type->is_mutable);
            ASSERT_EQUAL(definition->type->pointer.pointed_to->kind, AST_TYPE_NAME);
            ASSERT_FALSE(definition->type->pointer.pointed_to->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(definition->type->pointer.pointed_to->named_type.token.lexeme, "Int32");
            ASSERT_FALSE(definition->has_initial_value);

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    pointer: mutable * mutable Int32;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "pointer");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_POINTER);
            ASSERT_TRUE(definition->type->is_mutable);
            ASSERT_EQUAL(definition->type->pointer.pointed_to->kind, AST_TYPE_NAME);
            ASSERT_TRUE(definition->type->pointer.pointed_to->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(definition->type->pointer.pointed_to->named_type.token.lexeme, "Int32");
            ASSERT_FALSE(definition->has_initial_value);

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    variable: mutable _ = 123;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(definition->body.statements_count, 1);

            const Ast_Statement* statement = &definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
            ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.name.token.lexeme, "variable");
            ASSERT_EQUAL(statement->variable_definition.type->kind, AST_TYPE_NAME);
            ASSERT_TRUE(statement->variable_definition.type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.type->named_type.token.lexeme, "_");
            ASSERT_TRUE(statement->variable_definition.has_initial_value);
            ASSERT_EQUAL(statement->variable_definition.initial_value.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.initial_value.number.token.lexeme, "123");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }
    }
}

internal void
test_return_statement_parsing(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    // NOTE(vlad): Empty return statement.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    return;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(definition->body.statements_count, 1);

        const Ast_Statement* statement = &definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_RETURN);
        ASSERT_TRUE(statement->return_statement.is_empty);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Simple return statement with a number constant.
    {
        const String_View input = string_view("foo: () -> Int32 = {"
                                              "    return 123;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "Int32");

        ASSERT_EQUAL(definition->body.statements_count, 1);

        const Ast_Statement* statement = &definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_RETURN);
        ASSERT_FALSE(statement->return_statement.is_empty);
        ASSERT_EQUAL(statement->return_statement.expression.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(statement->return_statement.expression.number.token.lexeme,
                                 "123");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }
}

internal void
test_if_statement_parsing(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    // NOTE(vlad): If without else.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    if true { return; }"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(definition->body.statements_count, 1);

        const Ast_Statement* statement = &definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);
        const Ast_If_Statement* if_statement = &statement->if_statement;
        ASSERT_EQUAL(if_statement->condition.kind, AST_EXPRESSION_IDENTIFIER);
        ASSERT_EQUAL(if_statement->condition.identifier.token.type, TOKEN_TRUE);

        const Ast_Code_Block* then_code_block = &if_statement->if_statements;
        ASSERT_EQUAL(then_code_block->statements_count, 1);
        ASSERT_EQUAL(then_code_block->statements[0].type, AST_STATEMENT_RETURN);
        ASSERT_TRUE(then_code_block->statements[0].return_statement.is_empty);

        const Ast_Code_Block* else_code_block = &if_statement->else_statements;
        ASSERT_EQUAL(else_code_block->statements_count, 0);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): If with else.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    if true { return; }"
                                              "    else { a := 1; }"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(definition->body.statements_count, 1);

        const Ast_Statement* statement = &definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);
        const Ast_If_Statement* if_statement = &statement->if_statement;
        ASSERT_EQUAL(if_statement->condition.kind, AST_EXPRESSION_IDENTIFIER);
        ASSERT_EQUAL(if_statement->condition.identifier.token.type, TOKEN_TRUE);

        const Ast_Code_Block* then_code_block = &if_statement->if_statements;
        ASSERT_EQUAL(then_code_block->statements_count, 1);
        ASSERT_EQUAL(then_code_block->statements[0].type, AST_STATEMENT_RETURN);
        ASSERT_TRUE(then_code_block->statements[0].return_statement.is_empty);

        const Ast_Code_Block* else_code_block = &if_statement->else_statements;
        ASSERT_EQUAL(else_code_block->statements_count, 1);
        ASSERT_EQUAL(else_code_block->statements[0].type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* variable_definition = &else_code_block->statements[0].variable_definition;

        ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
        ASSERT_EQUAL(variable_definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(variable_definition->type->is_mutable);
        ASSERT_TRUE(variable_definition->has_initial_value);
        ASSERT_EQUAL(variable_definition->initial_value.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(variable_definition->initial_value.number.token.lexeme, "1");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing chained if statements.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    if 1 { return 1; }"
                                              "    else if 2 { return 2; }"
                                              "    else { return 3; }"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(definition->body.statements_count, 1);

        const Ast_Statement* statement = &definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);

        const Ast_If_Statement* if_statement = &statement->if_statement;
        ASSERT_EQUAL(if_statement->condition.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(if_statement->condition.number.token.lexeme, "1");

        const Ast_Code_Block* then_code_block = &if_statement->if_statements;
        ASSERT_EQUAL(then_code_block->statements_count, 1);
        ASSERT_EQUAL(then_code_block->statements[0].type, AST_STATEMENT_RETURN);

        const Ast_Return_Statement* first_return = &then_code_block->statements[0].return_statement;
        ASSERT_FALSE(first_return->is_empty);
        ASSERT_EQUAL(first_return->expression.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(first_return->expression.number.token.lexeme, "1");

        const Ast_Code_Block* else_code_block = &if_statement->else_statements;
        ASSERT_EQUAL(else_code_block->statements_count, 1);
        ASSERT_EQUAL(else_code_block->statements[0].type, AST_STATEMENT_IF);

        const Ast_If_Statement* else_if_statement = &else_code_block->statements[0].if_statement;
        ASSERT_EQUAL(else_if_statement->condition.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(else_if_statement->condition.number.token.lexeme, "2");

        const Ast_Code_Block* else_if_true_code_block = &else_if_statement->if_statements;
        ASSERT_EQUAL(else_if_true_code_block->statements_count, 1);
        ASSERT_EQUAL(else_if_true_code_block->statements[0].type, AST_STATEMENT_RETURN);

        const Ast_Return_Statement* second_return = &else_if_true_code_block->statements[0].return_statement;
        ASSERT_FALSE(second_return->is_empty);
        ASSERT_EQUAL(second_return->expression.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(second_return->expression.number.token.lexeme, "2");

        const Ast_Code_Block* else_if_false_code_block = &else_if_statement->else_statements;
        ASSERT_EQUAL(else_if_false_code_block->statements_count, 1);
        ASSERT_EQUAL(else_if_false_code_block->statements[0].type, AST_STATEMENT_RETURN);

        const Ast_Return_Statement* third_return = &else_if_false_code_block->statements[0].return_statement;
        ASSERT_FALSE(third_return->is_empty);
        ASSERT_EQUAL(third_return->expression.kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(third_return->expression.number.token.lexeme, "3");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }
}

internal void
test_expressions(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 0;"
                                              "    var2 := (var1);"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var1");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "0");
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var2");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.identifier.token.lexeme, "var1");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Simple expression tests.

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 + 2;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_ADD);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "+");
        ASSERT_EQUAL(definition->initial_value.binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.lhs->number.token.lexeme, "2");
        ASSERT_EQUAL(definition->initial_value.binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.rhs->number.token.lexeme, "2");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 - 2;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_SUBTRACT);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "-");
        ASSERT_EQUAL(definition->initial_value.binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.lhs->number.token.lexeme, "2");
        ASSERT_EQUAL(definition->initial_value.binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.rhs->number.token.lexeme, "2");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 * 2;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_MULTIPLY);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "*");
        ASSERT_EQUAL(definition->initial_value.binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.lhs->number.token.lexeme, "2");
        ASSERT_EQUAL(definition->initial_value.binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.rhs->number.token.lexeme, "2");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 / 2;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_TRUE(definition->has_initial_value);

        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_DIVIDE);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "/");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "2");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "2");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing function call without arguments.
    {
        const String_View input = string_view("foo: () -> Int32 = { return 123; }\n"
                                              "bar: () -> void = {"
                                              "    var := foo();"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 2);

        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "Int32");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_RETURN);
            ASSERT_FALSE(statement->return_statement.is_empty);
            ASSERT_EQUAL(statement->return_statement.expression.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(statement->return_statement.expression.number.token.lexeme,
                                     "123");
        }

        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[1];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "bar");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_CALL);

            const Ast_Call* call = &definition->initial_value.call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "foo");
            ASSERT_EQUAL(call->arguments_count, 0);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing function call with simple arguments.
    {
        const String_View input = string_view("foo: (first: Int32, second: Int32) -> Int32 = { return first + second; }\n"
                                              "bar: () -> void = {"
                                              "    var := foo(10, 20);"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 2);

        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[1];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "bar");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_CALL);

            const Ast_Call* call = &definition->initial_value.call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "foo");
            ASSERT_EQUAL(call->arguments_count, 2);

            {
                const Ast_Expression* first_argument = call->arguments[0];
                ASSERT_EQUAL(first_argument->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(first_argument->number.token.lexeme, "10");
            }

            {
                const Ast_Expression* second_argument = call->arguments[1];
                ASSERT_EQUAL(second_argument->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(second_argument->number.token.lexeme, "20");
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing function call with non-trivial arguments.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := bar(10 + something * 30, baz(10, 20));"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_CALL);

            const Ast_Call* call = &definition->initial_value.call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "bar");
            ASSERT_EQUAL(call->arguments_count, 2);

            {
                const Ast_Expression* first_argument = call->arguments[0];
                ASSERT_EQUAL(first_argument->kind, AST_EXPRESSION_ADD);

                ASSERT_STRINGS_ARE_EQUAL(first_argument->binary_expression.operator.lexeme, "+");

                {
                    const Ast_Expression* lhs = first_argument->binary_expression.lhs;
                    ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_NUMBER);
                    ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "10");
                }

                {
                    const Ast_Expression* rhs = first_argument->binary_expression.rhs;
                    ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_MULTIPLY);
                    ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.operator.lexeme, "*");

                    ASSERT_EQUAL(rhs->binary_expression.lhs->kind, AST_EXPRESSION_IDENTIFIER);
                    ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.lhs->identifier.token.lexeme, "something");

                    ASSERT_EQUAL(rhs->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
                    ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.rhs->number.token.lexeme, "30");
                }
            }

            {
                const Ast_Expression* second_argument = call->arguments[1];
                ASSERT_EQUAL(second_argument->kind, AST_EXPRESSION_CALL);

                const Ast_Call* nested_call = &second_argument->call;
                const Ast_Expression* nested_called_expression = nested_call->called_expression;
                ASSERT_EQUAL(nested_called_expression->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_STRINGS_ARE_EQUAL(nested_called_expression->identifier.token.lexeme, "baz");

                ASSERT_EQUAL(nested_call->arguments_count, 2);

                ASSERT_EQUAL(nested_call->arguments[0]->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(nested_call->arguments[0]->number.token.lexeme, "10");

                ASSERT_EQUAL(nested_call->arguments[1]->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(nested_call->arguments[1]->number.token.lexeme, "20");
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing comparison expressions.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 == 1;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "==");

            ASSERT_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 != 1;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NOT_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "!=");

            ASSERT_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 < 1;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_LESS);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "<");

            ASSERT_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 <= 1;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_LESS_OR_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "<=");

            ASSERT_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 > 1;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_GREATER);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, ">");

            ASSERT_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 >= 1;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_GREATER_OR_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, ">=");

            ASSERT_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing unary expressions.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := -1;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 0);

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NEGATE);

            const Ast_Unary_Expression* unary_expression = &definition->initial_value.unary_expression;

            ASSERT_STRINGS_ARE_EQUAL(unary_expression->operator.lexeme, "-");

            ASSERT_EQUAL(unary_expression->operand->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(unary_expression->operand->number.token.lexeme, "1");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (arg: * Int32) -> void = {"
                                                  "    var := arg* * 2;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 1);

            {
                const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "arg");
                ASSERT_EQUAL(parameter->type->kind, AST_TYPE_POINTER);
                ASSERT_FALSE(parameter->type->is_mutable);
                ASSERT_EQUAL(parameter->type->pointer.pointed_to->kind, AST_TYPE_NAME);
                ASSERT_FALSE(parameter->type->pointer.pointed_to->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(parameter->type->pointer.pointed_to->named_type.token.lexeme, "Int32");
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_MULTIPLY);

            {
                const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

                ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "*");

                const Ast_Expression* lhs = binary_expression->lhs;
                ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_DEREFERENCE);
                ASSERT_STRINGS_ARE_EQUAL(lhs->unary_expression.operator.lexeme, "*");
                ASSERT_EQUAL(lhs->unary_expression.operand->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_STRINGS_ARE_EQUAL(lhs->unary_expression.operand->identifier.token.lexeme, "arg");

                const Ast_Expression* rhs = binary_expression->rhs;
                ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "2");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (arg: * Int32) -> void = {"
                                                  "    var := arg* + 2;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 1);

            {
                const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "arg");
                ASSERT_EQUAL(parameter->type->kind, AST_TYPE_POINTER);
                ASSERT_FALSE(parameter->type->is_mutable);
                ASSERT_EQUAL(parameter->type->pointer.pointed_to->kind, AST_TYPE_NAME);
                ASSERT_FALSE(parameter->type->pointer.pointed_to->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(parameter->type->pointer.pointed_to->named_type.token.lexeme, "Int32");
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_ADD);

            {
                const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

                ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "+");

                const Ast_Expression* lhs = binary_expression->lhs;
                ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_DEREFERENCE);
                ASSERT_STRINGS_ARE_EQUAL(lhs->unary_expression.operator.lexeme, "*");
                ASSERT_EQUAL(lhs->unary_expression.operand->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_STRINGS_ARE_EQUAL(lhs->unary_expression.operand->identifier.token.lexeme, "arg");

                const Ast_Expression* rhs = binary_expression->rhs;
                ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "2");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (arg: * * Int32) -> void = {"
                                                  "    var := arg** * 2;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 1);

            {
                const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "arg");
                ASSERT_EQUAL(parameter->type->kind, AST_TYPE_POINTER);
                ASSERT_FALSE(parameter->type->is_mutable);
                ASSERT_EQUAL(parameter->type->pointer.pointed_to->kind, AST_TYPE_POINTER);
                ASSERT_FALSE(parameter->type->pointer.pointed_to->is_mutable);
                ASSERT_EQUAL(parameter->type->pointer.pointed_to->pointer.pointed_to->kind, AST_TYPE_NAME);
                ASSERT_FALSE(parameter->type->pointer.pointed_to->pointer.pointed_to->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(parameter->type->pointer.pointed_to->pointer.pointed_to->named_type.token.lexeme, "Int32");
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_MULTIPLY);

            {
                const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

                ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "*");

                const Ast_Expression* lhs = binary_expression->lhs;
                ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_DEREFERENCE);
                {
                    const Ast_Unary_Expression* first_dereference = &lhs->unary_expression;
                    ASSERT_STRINGS_ARE_EQUAL(first_dereference->operator.lexeme, "*");
                    ASSERT_EQUAL(first_dereference->operand->kind, AST_EXPRESSION_DEREFERENCE);

                    const Ast_Unary_Expression* second_dereference = &first_dereference->operand->unary_expression;
                    ASSERT_STRINGS_ARE_EQUAL(second_dereference->operator.lexeme, "*");
                    ASSERT_EQUAL(second_dereference->operand->kind, AST_EXPRESSION_IDENTIFIER);
                    ASSERT_STRINGS_ARE_EQUAL(second_dereference->operand->identifier.token.lexeme, "arg");
                }

                const Ast_Expression* rhs = binary_expression->rhs;
                ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "2");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        // NOTE(vlad): Dereference and multiplication without delimiting spaces.
        {
            const String_View input = string_view("foo: (arg: * * Int32) -> void = {"
                                                  "    var := arg***2;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 1);

            {
                const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "arg");
                ASSERT_EQUAL(parameter->type->kind, AST_TYPE_POINTER);
                ASSERT_FALSE(parameter->type->is_mutable);
                ASSERT_EQUAL(parameter->type->pointer.pointed_to->kind, AST_TYPE_POINTER);
                ASSERT_FALSE(parameter->type->pointer.pointed_to->is_mutable);
                ASSERT_EQUAL(parameter->type->pointer.pointed_to->pointer.pointed_to->kind, AST_TYPE_NAME);
                ASSERT_FALSE(parameter->type->pointer.pointed_to->pointer.pointed_to->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(parameter->type->pointer.pointed_to->pointer.pointed_to->named_type.token.lexeme, "Int32");
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_MULTIPLY);

            {
                const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

                ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "*");

                const Ast_Expression* lhs = binary_expression->lhs;
                ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_DEREFERENCE);
                {
                    const Ast_Unary_Expression* first_dereference = &lhs->unary_expression;
                    ASSERT_STRINGS_ARE_EQUAL(first_dereference->operator.lexeme, "*");
                    ASSERT_EQUAL(first_dereference->operand->kind, AST_EXPRESSION_DEREFERENCE);

                    const Ast_Unary_Expression* second_dereference = &first_dereference->operand->unary_expression;
                    ASSERT_STRINGS_ARE_EQUAL(second_dereference->operator.lexeme, "*");
                    ASSERT_EQUAL(second_dereference->operand->kind, AST_EXPRESSION_IDENTIFIER);
                    ASSERT_STRINGS_ARE_EQUAL(second_dereference->operand->identifier.token.lexeme, "arg");
                }

                const Ast_Expression* rhs = binary_expression->rhs;
                ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "2");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (arg: Int32) -> void = {"
                                                  "    var := arg&;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 1);

            {
                const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "arg");
                ASSERT_EQUAL(parameter->type->kind, AST_TYPE_NAME);
                ASSERT_FALSE(parameter->type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(parameter->type->named_type.token.lexeme, "Int32");
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_ADDRESS_OF);

            const Ast_Unary_Expression* unary_expression = &definition->initial_value.unary_expression;
            ASSERT_STRINGS_ARE_EQUAL(unary_expression->operator.lexeme, "&");
            ASSERT_EQUAL(unary_expression->operand->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(unary_expression->operand->identifier.token.lexeme, "arg");

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (arg: Int32) -> void = {"
                                                  "    var := arg&* * 2;"
                                                  "}");

            Compilation_Context context = {0};
            Lexer lexer = {0};
            Parser parser = {0};

            create_compilation_context(&context);
            create_lexer(&lexer, string_view("<input>"), input);
            create_parser(&parser, &lexer, &context, &errors);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
            ASSERT_FALSE(function_type->is_mutable);

            ASSERT_EQUAL(function_type->function.parameters_count, 1);

            {
                const Ast_Function_Parameter* parameter = &function_type->function.parameters[0];
                ASSERT_STRINGS_ARE_EQUAL(parameter->name.token.lexeme, "arg");
                ASSERT_EQUAL(parameter->type->kind, AST_TYPE_NAME);
                ASSERT_FALSE(parameter->type->is_mutable);
                ASSERT_STRINGS_ARE_EQUAL(parameter->type->named_type.token.lexeme, "Int32");
            }

            const Ast_Type* return_type = function_type->function.return_type;
            ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
            ASSERT_FALSE(return_type->is_mutable);
            ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

            ASSERT_EQUAL(function_definition->body.statements_count, 1);

            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_MULTIPLY);

            {
                const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

                ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "*");

                const Ast_Expression* lhs = binary_expression->lhs;
                ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_DEREFERENCE);
                {
                    const Ast_Unary_Expression* dereference = &lhs->unary_expression;
                    ASSERT_STRINGS_ARE_EQUAL(dereference->operator.lexeme, "*");
                    ASSERT_EQUAL(dereference->operand->kind, AST_EXPRESSION_ADDRESS_OF);

                    const Ast_Unary_Expression* address_of = &dereference->operand->unary_expression;
                    ASSERT_STRINGS_ARE_EQUAL(address_of->operator.lexeme, "&");
                    ASSERT_EQUAL(address_of->operand->kind, AST_EXPRESSION_IDENTIFIER);
                    ASSERT_STRINGS_ARE_EQUAL(address_of->operand->identifier.token.lexeme, "arg");
                }

                const Ast_Expression* rhs = binary_expression->rhs;
                ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "2");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);

            clear_errors(&errors);
        }
    }
}

internal void
test_operator_precedence(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 1 + 2 * 3;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_ADD);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "+");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "1");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_MULTIPLY);
            ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.operator.lexeme, "*");
            ASSERT_EQUAL(rhs->binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.lhs->number.token.lexeme, "2");
            ASSERT_EQUAL(rhs->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.rhs->number.token.lexeme, "3");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := (1 + 2) * 3;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_TRUE(definition->has_initial_value);

        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_MULTIPLY);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "*");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_ADD);
            ASSERT_STRINGS_ARE_EQUAL(lhs->binary_expression.operator.lexeme, "+");
            ASSERT_EQUAL(lhs->binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->binary_expression.lhs->number.token.lexeme, "1");
            ASSERT_EQUAL(lhs->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->binary_expression.rhs->number.token.lexeme, "2");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "3");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 3 + bar();"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        const Ast_Statement* statement = &function_definition->body.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
        ASSERT_FALSE(definition->type->is_mutable);
        ASSERT_TRUE(definition->has_initial_value);
        ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_ADD);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "+");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "3");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_CALL);

            const Ast_Call* call = &rhs->call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "bar");
            ASSERT_EQUAL(call->arguments_count, 0);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }
}

internal void
test_assignments(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 1;"
                                              "    var = 2;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "1");
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_ASSIGNMENT);

            const Ast_Assignment* assignment = &statement->assignment;
            ASSERT_STRINGS_ARE_EQUAL(assignment->name.token.lexeme, "var");
            ASSERT_EQUAL(assignment->expression.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.number.token.lexeme, "2");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 1;"
                                              "    var = var + 1;"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "1");
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_ASSIGNMENT);

            const Ast_Assignment* assignment = &statement->assignment;
            ASSERT_STRINGS_ARE_EQUAL(assignment->name.token.lexeme, "var");
            ASSERT_EQUAL(assignment->expression.kind, AST_EXPRESSION_ADD);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.binary_expression.operator.lexeme, "+");

            {
                const Ast_Expression* lhs = assignment->expression.binary_expression.lhs;
                ASSERT_EQUAL(lhs->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_STRINGS_ARE_EQUAL(lhs->identifier.token.lexeme, "var");
            }

            {
                const Ast_Expression* rhs = assignment->expression.binary_expression.rhs;
                ASSERT_EQUAL(rhs->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "1");
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }
}

internal void
test_while_statements(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 10;"
                                              "    while var != 0"
                                              "    {"
                                              "        var = var - 1;"
                                              "    }"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->kind, AST_TYPE_OMITTED);
            ASSERT_FALSE(definition->type->is_mutable);
            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_EQUAL(definition->initial_value.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "10");
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            const Ast_Expression* condition = &while_statement->condition;
            ASSERT_EQUAL(condition->kind, AST_EXPRESSION_NOT_EQUAL);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.operator.lexeme, "!=");

            ASSERT_EQUAL(condition->binary_expression.lhs->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.lhs->identifier.token.lexeme, "var");

            ASSERT_EQUAL(condition->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.rhs->number.token.lexeme, "0");

            ASSERT_EQUAL(while_statement->body.statements_count, 1);

            const Ast_Statement* inner_statement = &while_statement->body.statements[0];
            ASSERT_EQUAL(inner_statement->type, AST_STATEMENT_ASSIGNMENT);

            const Ast_Assignment* assignment = &inner_statement->assignment;
            ASSERT_STRINGS_ARE_EQUAL(assignment->name.token.lexeme, "var");
            ASSERT_EQUAL(assignment->expression.kind, AST_EXPRESSION_SUBTRACT);
            ASSERT_EQUAL(assignment->expression.binary_expression.lhs->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.binary_expression.lhs->identifier.token.lexeme, "var");

            ASSERT_EQUAL(assignment->expression.binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.binary_expression.rhs->number.token.lexeme, "1");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }
}

internal void
test_call_statements(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    bar(10);"
                                              "}");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_CALL);

            const Ast_Call_Statement* call_statement = &statement->call_statement;
            const Ast_Call* call = &call_statement->call;

            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "bar");
            ASSERT_EQUAL(call->arguments_count, 1);

            {
                const Ast_Expression* argument = call->arguments[0];
                ASSERT_EQUAL(argument->kind, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(argument->number.token.lexeme, "10");
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }
}

internal void
test_syntax_errors(Test_Context* test_context)
{
    Errors errors = {0};
    create_errors(&errors, test_context->arena);

    {
        const String_View input = string_view("");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_FALSE(parse_ast(&parser));

        ASSERT_EQUAL(errors.errors_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(errors.errors[0].message, "Expected identifier, found end of file");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo");

        Compilation_Context context = {0};
        Lexer lexer = {0};
        Parser parser = {0};

        create_compilation_context(&context);
        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, &lexer, &context, &errors);

        ASSERT_FALSE(parse_ast(&parser));

        ASSERT_EQUAL(errors.errors_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(errors.errors[0].message, "Expected :, found end of file");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);

        clear_errors(&errors);
    }
}

REGISTER_TESTS(
    test_function_definitions_parsing,
    test_variable_definitions_parsing,
    test_return_statement_parsing,
    test_if_statement_parsing,
    test_expressions,
    test_operator_precedence,
    test_assignments,
    test_while_statements,
    test_call_statements,
    test_syntax_errors
)

#include "eon_compilation_context.c"
#include "eon_errors.c"
#include "eon_lexer.c"
#include "eon_parser.c"
