#include <eon/unit_test.h>

#include "eon_parser.h"

internal void
test_function_definitions_parsing(Test_Context* context)
{
    {
        const String_View input = string_view("foo: () -> Bool = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_USER_DEFINED);
        ASSERT_STRINGS_ARE_EQUAL(return_type->name.token.lexeme, "Bool");

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("foo: () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("foo: (argument: Int32) -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 1);

        const Ast_Function_Argument* argument = &arguments->arguments[0];
        ASSERT_STRINGS_ARE_EQUAL(argument->name.token.lexeme, "argument");

        const Ast_Type* argument_type = argument->type;
        ASSERT_EQUAL(argument_type->type, AST_TYPE_INT_32);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("foo: () -> () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_FUNCTION);
        ASSERT_EQUAL(return_type->arguments.arguments_count, 0);
        ASSERT_EQUAL(return_type->return_type->type, AST_TYPE_VOID);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("foo: (first: Int32, second: Some_Type) -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 2);

        {
            const Ast_Function_Argument* first_argument = &arguments->arguments[0];
            ASSERT_STRINGS_ARE_EQUAL(first_argument->name.token.lexeme, "first");

            const Ast_Type* first_argument_type = first_argument->type;
            ASSERT_EQUAL(first_argument_type->type, AST_TYPE_INT_32);
        }

        {
            const Ast_Function_Argument* second_argument = &arguments->arguments[1];
            ASSERT_STRINGS_ARE_EQUAL(second_argument->name.token.lexeme, "second");

            const Ast_Type* second_argument_type = second_argument->type;
            ASSERT_EQUAL(second_argument_type->type, AST_TYPE_USER_DEFINED);
            ASSERT_STRINGS_ARE_EQUAL(second_argument_type->name.token.lexeme, "Some_Type");
        }

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Multiple function definitions.
    {
        const String_View input = string_view("foo: () -> void = {}\n"
                                              "bar: (arg: Type) -> Other_Type = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 2);

        {
            const Ast_Function_Definition* definition = &ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

            const Ast_Function_Arguments* arguments = &function_type->arguments;
            ASSERT_EQUAL(arguments->arguments_count, 0);

            const Ast_Type* return_type = function_type->return_type;
            ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);
        }

        {
            const Ast_Function_Definition* definition = &ast.function_definitions[1];
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "bar");

            const Ast_Type* function_type = definition->type;
            ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

            const Ast_Function_Arguments* arguments = &function_type->arguments;
            ASSERT_EQUAL(arguments->arguments_count, 1);
            ASSERT_STRINGS_ARE_EQUAL(arguments->arguments[0].name.token.lexeme, "arg");
            ASSERT_EQUAL(arguments->arguments[0].type->type, AST_TYPE_USER_DEFINED);
            ASSERT_STRINGS_ARE_EQUAL(arguments->arguments[0].type->name.token.lexeme, "Type");

            const Ast_Type* return_type = function_type->return_type;
            ASSERT_EQUAL(return_type->type, AST_TYPE_USER_DEFINED);
            ASSERT_STRINGS_ARE_EQUAL(return_type->name.token.lexeme, "Other_Type");
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("foo: () -> void = { variable: Int32; }");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        ASSERT_EQUAL(definition->statements.statements_count, 1);

        const Ast_Statement* statement = &definition->statements.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DECLARATION);
        ASSERT_STRINGS_ARE_EQUAL(statement->variable_declaration.name.token.lexeme, "variable");
        ASSERT_EQUAL(statement->variable_declaration.type->type, AST_TYPE_INT_32);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

REGISTER_TESTS(
    test_function_definitions_parsing
)

#include "eon_parser.c"
#include "eon_lexer.c"
#include "eon_log.c"
