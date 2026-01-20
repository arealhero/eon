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
}

internal void
test_variable_definitions_parsing(Test_Context* context)
{
    // NOTE(vlad): Variable without initialisation.
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
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
        ASSERT_STRINGS_ARE_EQUAL(statement->variable_definition.name.token.lexeme, "variable");
        ASSERT_EQUAL(statement->variable_definition.type->type, AST_TYPE_INT_32);
        ASSERT_EQUAL(statement->variable_definition.initialisation_type, AST_INITIALISATION_DEFAULT);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Variable with initialisation.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    variable: Int32 = 123;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        ASSERT_EQUAL(function_definition->statements.statements_count, 1);

        const Ast_Statement* statement = &function_definition->statements.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "variable");
        ASSERT_EQUAL(definition->type->type, AST_TYPE_INT_32);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Variable with deduced type.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    variable := 123;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        ASSERT_EQUAL(function_definition->statements.statements_count, 1);

        const Ast_Statement* statement = &function_definition->statements.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "variable");
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Multiple variable definitions.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 123;"
                                              "    var2: String_View;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        ASSERT_EQUAL(function_definition->statements.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->statements.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var1");
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");
        }

        {
            const Ast_Statement* statement = &function_definition->statements.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var2");
            ASSERT_EQUAL(definition->type->type, AST_TYPE_USER_DEFINED);
            ASSERT_STRINGS_ARE_EQUAL(definition->type->name.token.lexeme, "String_View");
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_DEFAULT);
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    // NOTE(vlad): String_View initialisation.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var: String_View = \"Hello\";"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        ASSERT_EQUAL(function_definition->statements.statements_count, 1);

        const Ast_Statement* statement = &function_definition->statements.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* definition = &statement->variable_definition;
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
        ASSERT_EQUAL(definition->type->type, AST_TYPE_USER_DEFINED);
        ASSERT_STRINGS_ARE_EQUAL(definition->type->name.token.lexeme, "String_View");
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_STRING_LITERAL);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.string_literal.value, "Hello");
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.string_literal.token.lexeme, "\"Hello\"");

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Variable initialisation via other variable.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 123;"
                                              "    var2 := var1;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(context->arena, &parser, &lexer);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_VOID);

        ASSERT_EQUAL(function_definition->statements.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->statements.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var1");
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "123");
        }

        {
            const Ast_Statement* statement = &function_definition->statements.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* definition = &statement->variable_definition;
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var2");
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.identifier.token.lexeme, "var1");
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

internal void
test_return_statement_parsing(Test_Context* context)
{
    // NOTE(vlad): Empty return statement.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    return;"
                                              "}");

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
        ASSERT_EQUAL(statement->type, AST_STATEMENT_RETURN);
        ASSERT_TRUE(statement->return_statement.is_empty);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Simple return statement with a number constant.
    {
        const String_View input = string_view("foo: () -> Int32 = {"
                                              "    return 123;"
                                              "}");

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
        ASSERT_EQUAL(return_type->type, AST_TYPE_INT_32);

        ASSERT_EQUAL(definition->statements.statements_count, 1);

        const Ast_Statement* statement = &definition->statements.statements[0];
        ASSERT_EQUAL(statement->type, AST_STATEMENT_RETURN);
        ASSERT_FALSE(statement->return_statement.is_empty);
        ASSERT_EQUAL(statement->return_statement.expression.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(statement->return_statement.expression.number.token.lexeme,
                                 "123");

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

REGISTER_TESTS(
    test_function_definitions_parsing,
    test_variable_definitions_parsing,
    test_return_statement_parsing
)

#include "eon_parser.c"
#include "eon_lexer.c"
#include "eon_log.c"
