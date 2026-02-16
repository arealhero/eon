#include <eon/unit_test.h>

#include "eon_parser.h"

internal void
test_function_definitions_parsing(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("foo: () -> Bool = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: (argument: Int32) -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> * () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_POINTER);
        ASSERT_EQUAL(return_type->pointed_to->type, AST_TYPE_FUNCTION);
        ASSERT_EQUAL(return_type->pointed_to->arguments.arguments_count, 0);
        ASSERT_EQUAL(return_type->pointed_to->return_type->type, AST_TYPE_VOID);

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: (first: Int32, second: Some_Type) -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): Multiple function definitions.
    {
        const String_View input = string_view("foo: () -> void = {}\n"
                                              "bar: (arg: Type) -> Other_Type = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> Float32 = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        const Ast_Function_Definition* definition = &ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = definition->type;
        ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

        const Ast_Function_Arguments* arguments = &function_type->arguments;
        ASSERT_EQUAL(arguments->arguments_count, 0);

        const Ast_Type* return_type = function_type->return_type;
        ASSERT_EQUAL(return_type->type, AST_TYPE_FLOAT_32);
        ASSERT_STRINGS_ARE_EQUAL(return_type->name.token.lexeme, "Float32");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_variable_definitions_parsing(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // NOTE(vlad): Variable without initialisation.
    {
        const String_View input = string_view("foo: () -> void = { variable: Int32; }");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): Variable with initialisation.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    variable: Int32 = 123;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): Variable with deduced type.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    variable := 123;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): Multiple variable definitions.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 123;"
                                              "    var2: String_View;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): String_View initialisation.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var: String_View = \"Hello\";"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): Variable initialisation via other variable.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 123;"
                                              "    var2 := var1;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): Pointer declaration.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    pointer: * Int32;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "pointer");
        ASSERT_EQUAL(definition->type->type, AST_TYPE_POINTER);
        ASSERT_EQUAL(definition->type->pointed_to->type, AST_TYPE_INT_32);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_DEFAULT);

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Pointer to a pointer declaration.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    pointer: ** Int32;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "pointer");
        ASSERT_EQUAL(definition->type->type, AST_TYPE_POINTER);
        ASSERT_EQUAL(definition->type->pointed_to->type, AST_TYPE_POINTER);
        ASSERT_EQUAL(definition->type->pointed_to->pointed_to->type, AST_TYPE_INT_32);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_DEFAULT);

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_return_statement_parsing(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // NOTE(vlad): Empty return statement.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    return;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }

    // NOTE(vlad): Simple return statement with a number constant.
    {
        const String_View input = string_view("foo: () -> Int32 = {"
                                              "    return 123;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        clear_errors(&errors);
    }
}

internal void
test_if_statement_parsing(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // NOTE(vlad): If without else.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    if true { return; }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);
        const Ast_If_Statement* if_statement = &statement->if_statement;
        ASSERT_EQUAL(if_statement->condition.type, AST_EXPRESSION_IDENTIFIER);
        ASSERT_EQUAL(if_statement->condition.identifier.token.type, TOKEN_TRUE);

        const Ast_Statements* true_statements = &if_statement->if_statements;
        ASSERT_EQUAL(true_statements->statements_count, 1);
        ASSERT_EQUAL(true_statements->statements[0].type, AST_STATEMENT_RETURN);
        ASSERT_TRUE(true_statements->statements[0].return_statement.is_empty);

        const Ast_Statements* false_statements = &if_statement->else_statements;
        ASSERT_EQUAL(false_statements->statements_count, 0);

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): If with else.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    if true { return; }"
                                              "    else { a := 1; }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);
        const Ast_If_Statement* if_statement = &statement->if_statement;
        ASSERT_EQUAL(if_statement->condition.type, AST_EXPRESSION_IDENTIFIER);
        ASSERT_EQUAL(if_statement->condition.identifier.token.type, TOKEN_TRUE);

        const Ast_Statements* true_statements = &if_statement->if_statements;
        ASSERT_EQUAL(true_statements->statements_count, 1);
        ASSERT_EQUAL(true_statements->statements[0].type, AST_STATEMENT_RETURN);
        ASSERT_TRUE(true_statements->statements[0].return_statement.is_empty);

        const Ast_Statements* false_statements = &if_statement->else_statements;
        ASSERT_EQUAL(false_statements->statements_count, 1);
        ASSERT_EQUAL(false_statements->statements[0].type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* variable_definition = &false_statements->statements[0].variable_definition;

        ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
        ASSERT_EQUAL(variable_definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(variable_definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(variable_definition->initial_value.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(variable_definition->initial_value.number.token.lexeme, "1");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing chained if statements.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    if 1 { return 1; }"
                                              "    else if 2 { return 2; }"
                                              "    else { return 3; }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);

        const Ast_If_Statement* if_statement = &statement->if_statement;
        ASSERT_EQUAL(if_statement->condition.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(if_statement->condition.number.token.lexeme, "1");

        const Ast_Statements* true_statements = &if_statement->if_statements;
        ASSERT_EQUAL(true_statements->statements_count, 1);
        ASSERT_EQUAL(true_statements->statements[0].type, AST_STATEMENT_RETURN);

        const Ast_Return_Statement* first_return = &true_statements->statements[0].return_statement;
        ASSERT_FALSE(first_return->is_empty);
        ASSERT_EQUAL(first_return->expression.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(first_return->expression.number.token.lexeme, "1");

        const Ast_Statements* false_statements = &if_statement->else_statements;
        ASSERT_EQUAL(false_statements->statements_count, 1);
        ASSERT_EQUAL(false_statements->statements[0].type, AST_STATEMENT_IF);

        const Ast_If_Statement* else_if_statement = &false_statements->statements[0].if_statement;
        ASSERT_EQUAL(else_if_statement->condition.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(else_if_statement->condition.number.token.lexeme, "2");

        const Ast_Statements* else_if_true_statements = &else_if_statement->if_statements;
        ASSERT_EQUAL(else_if_true_statements->statements_count, 1);
        ASSERT_EQUAL(else_if_true_statements->statements[0].type, AST_STATEMENT_RETURN);

        const Ast_Return_Statement* second_return = &else_if_true_statements->statements[0].return_statement;
        ASSERT_FALSE(second_return->is_empty);
        ASSERT_EQUAL(second_return->expression.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(second_return->expression.number.token.lexeme, "2");

        const Ast_Statements* else_if_false_statements = &else_if_statement->else_statements;
        ASSERT_EQUAL(else_if_false_statements->statements_count, 1);
        ASSERT_EQUAL(else_if_false_statements->statements[0].type, AST_STATEMENT_RETURN);

        const Ast_Return_Statement* third_return = &else_if_false_statements->statements[0].return_statement;
        ASSERT_FALSE(third_return->is_empty);
        ASSERT_EQUAL(third_return->expression.type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(third_return->expression.number.token.lexeme, "3");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_expressions(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var1 := 0;"
                                              "    var2 := (var1);"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "0");
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

        clear_errors(&errors);
    }

    // NOTE(vlad): Simple expression tests.

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 + 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_ADD);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "+");
        ASSERT_EQUAL(definition->initial_value.binary_expression.lhs->type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.lhs->number.token.lexeme, "2");
        ASSERT_EQUAL(definition->initial_value.binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.rhs->number.token.lexeme, "2");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 - 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_SUBTRACT);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "-");
        ASSERT_EQUAL(definition->initial_value.binary_expression.lhs->type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.lhs->number.token.lexeme, "2");
        ASSERT_EQUAL(definition->initial_value.binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.rhs->number.token.lexeme, "2");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 * 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_MULTIPLY);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "*");
        ASSERT_EQUAL(definition->initial_value.binary_expression.lhs->type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.lhs->number.token.lexeme, "2");
        ASSERT_EQUAL(definition->initial_value.binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.rhs->number.token.lexeme, "2");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 2 / 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);

        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_DIVIDE);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "/");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "2");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "2");
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing function call without arguments.
    {
        const String_View input = string_view("foo: () -> Int32 = { return 123; }\n"
                                              "bar: () -> void = {"
                                              "    var := foo();"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(ast.function_definitions_count, 2);

        {
            const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

            const Ast_Type* function_type = function_definition->type;
            ASSERT_EQUAL(function_type->type, AST_TYPE_FUNCTION);

            const Ast_Function_Arguments* arguments = &function_type->arguments;
            ASSERT_EQUAL(arguments->arguments_count, 0);

            const Ast_Type* return_type = function_type->return_type;
            ASSERT_EQUAL(return_type->type, AST_TYPE_INT_32);

            ASSERT_EQUAL(function_definition->statements.statements_count, 1);

            const Ast_Statement* statement = &function_definition->statements.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_RETURN);
            ASSERT_FALSE(statement->return_statement.is_empty);
            ASSERT_EQUAL(statement->return_statement.expression.type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(statement->return_statement.expression.number.token.lexeme,
                                     "123");
        }

        {
            const Ast_Function_Definition* function_definition = &ast.function_definitions[1];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "bar");

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_CALL);

            const Ast_Call* call = &definition->initial_value.call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "foo");
            ASSERT_EQUAL(call->arguments_count, 0);
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing function call with simple arguments.
    {
        const String_View input = string_view("foo: (first: Int32, second: Int32) -> Int32 = { return first + second; }\n"
                                              "bar: () -> void = {"
                                              "    var := foo(10, 20);"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(ast.function_definitions_count, 2);

        {
            const Ast_Function_Definition* function_definition = &ast.function_definitions[1];
            ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "bar");

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_CALL);

            const Ast_Call* call = &definition->initial_value.call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "foo");
            ASSERT_EQUAL(call->arguments_count, 2);

            {
                const Ast_Expression* first_argument = call->arguments[0];
                ASSERT_EQUAL(first_argument->type, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(first_argument->number.token.lexeme, "10");
            }

            {
                const Ast_Expression* second_argument = call->arguments[1];
                ASSERT_EQUAL(second_argument->type, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(second_argument->number.token.lexeme, "20");
            }
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing function call with non-trivial arguments.
    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := bar(10 + something * 30, baz(10, 20));"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        {
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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_CALL);

            const Ast_Call* call = &definition->initial_value.call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "bar");
            ASSERT_EQUAL(call->arguments_count, 2);

            {
                const Ast_Expression* first_argument = call->arguments[0];
                ASSERT_EQUAL(first_argument->type, AST_EXPRESSION_ADD);

                ASSERT_STRINGS_ARE_EQUAL(first_argument->binary_expression.operator.lexeme, "+");

                {
                    const Ast_Expression* lhs = first_argument->binary_expression.lhs;
                    ASSERT_EQUAL(lhs->type, AST_EXPRESSION_NUMBER);
                    ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "10");
                }

                {
                    const Ast_Expression* rhs = first_argument->binary_expression.rhs;
                    ASSERT_EQUAL(rhs->type, AST_EXPRESSION_MULTIPLY);
                    ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.operator.lexeme, "*");

                    ASSERT_EQUAL(rhs->binary_expression.lhs->type, AST_EXPRESSION_IDENTIFIER);
                    ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.lhs->identifier.token.lexeme, "something");

                    ASSERT_EQUAL(rhs->binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
                    ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.rhs->number.token.lexeme, "30");
                }
            }

            {
                const Ast_Expression* second_argument = call->arguments[1];
                ASSERT_EQUAL(second_argument->type, AST_EXPRESSION_CALL);

                const Ast_Call* nested_call = &second_argument->call;
                const Ast_Expression* nested_called_expression = nested_call->called_expression;
                ASSERT_EQUAL(nested_called_expression->type, AST_EXPRESSION_IDENTIFIER);
                ASSERT_STRINGS_ARE_EQUAL(nested_called_expression->identifier.token.lexeme, "baz");

                ASSERT_EQUAL(nested_call->arguments_count, 2);

                ASSERT_EQUAL(nested_call->arguments[0]->type, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(nested_call->arguments[0]->number.token.lexeme, "10");

                ASSERT_EQUAL(nested_call->arguments[1]->type, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(nested_call->arguments[1]->number.token.lexeme, "20");
            }
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    // NOTE(vlad): Testing comparison expressions.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 == 1;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "==");

            ASSERT_EQUAL(binary_expression->lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 != 1;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NOT_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "!=");

            ASSERT_EQUAL(binary_expression->lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 < 1;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_LESS);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "<");

            ASSERT_EQUAL(binary_expression->lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 <= 1;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_LESS_OR_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, "<=");

            ASSERT_EQUAL(binary_expression->lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 > 1;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_GREATER);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, ">");

            ASSERT_EQUAL(binary_expression->lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    var := 1 >= 1;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_GREATER_OR_EQUAL);

            const Ast_Binary_Expression* binary_expression = &definition->initial_value.binary_expression;

            ASSERT_STRINGS_ARE_EQUAL(binary_expression->operator.lexeme, ">=");

            ASSERT_EQUAL(binary_expression->lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->lhs->number.token.lexeme, "1");

            ASSERT_EQUAL(binary_expression->rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(binary_expression->rhs->number.token.lexeme, "1");

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }
}

internal void
test_operator_precedence(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 1 + 2 * 3;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_ADD);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "+");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "1");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->type, AST_EXPRESSION_MULTIPLY);
            ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.operator.lexeme, "*");
            ASSERT_EQUAL(rhs->binary_expression.lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.lhs->number.token.lexeme, "2");
            ASSERT_EQUAL(rhs->binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->binary_expression.rhs->number.token.lexeme, "3");
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := (1 + 2) * 3;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);

        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_MULTIPLY);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "*");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->type, AST_EXPRESSION_ADD);
            ASSERT_STRINGS_ARE_EQUAL(lhs->binary_expression.operator.lexeme, "+");
            ASSERT_EQUAL(lhs->binary_expression.lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->binary_expression.lhs->number.token.lexeme, "1");
            ASSERT_EQUAL(lhs->binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->binary_expression.rhs->number.token.lexeme, "2");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "3");
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 3 + bar();"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
        ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
        ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
        ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_ADD);
        ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.binary_expression.operator.lexeme, "+");

        {
            const Ast_Expression* lhs = definition->initial_value.binary_expression.lhs;
            ASSERT_EQUAL(lhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(lhs->number.token.lexeme, "3");
        }

        {
            const Ast_Expression* rhs = definition->initial_value.binary_expression.rhs;
            ASSERT_EQUAL(rhs->type, AST_EXPRESSION_CALL);

            const Ast_Call* call = &rhs->call;
            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "bar");
            ASSERT_EQUAL(call->arguments_count, 0);
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_assignments(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 1;"
                                              "    var = 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "1");
        }

        {
            const Ast_Statement* statement = &function_definition->statements.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_ASSIGNMENT);

            const Ast_Assignment* assignment = &statement->assignment;
            ASSERT_STRINGS_ARE_EQUAL(assignment->name.token.lexeme, "var");
            ASSERT_EQUAL(assignment->expression.type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.number.token.lexeme, "2");
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 1;"
                                              "    var = var + 1;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "1");
        }

        {
            const Ast_Statement* statement = &function_definition->statements.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_ASSIGNMENT);

            const Ast_Assignment* assignment = &statement->assignment;
            ASSERT_STRINGS_ARE_EQUAL(assignment->name.token.lexeme, "var");
            ASSERT_EQUAL(assignment->expression.type, AST_EXPRESSION_ADD);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.binary_expression.operator.lexeme, "+");

            {
                const Ast_Expression* lhs = assignment->expression.binary_expression.lhs;
                ASSERT_EQUAL(lhs->type, AST_EXPRESSION_IDENTIFIER);
                ASSERT_STRINGS_ARE_EQUAL(lhs->identifier.token.lexeme, "var");
            }

            {
                const Ast_Expression* rhs = assignment->expression.binary_expression.rhs;
                ASSERT_EQUAL(rhs->type, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(rhs->number.token.lexeme, "1");
            }
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_while_statements(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    var := 10;"
                                              "    while var != 0"
                                              "    {"
                                              "        var = var - 1;"
                                              "    }"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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
            ASSERT_STRINGS_ARE_EQUAL(definition->name.token.lexeme, "var");
            ASSERT_EQUAL(definition->type->type, AST_TYPE_DEDUCED);
            ASSERT_EQUAL(definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);
            ASSERT_EQUAL(definition->initial_value.type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(definition->initial_value.number.token.lexeme, "10");
        }

        {
            const Ast_Statement* statement = &function_definition->statements.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            const Ast_Expression* condition = &while_statement->condition;
            ASSERT_EQUAL(condition->type, AST_EXPRESSION_NOT_EQUAL);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.operator.lexeme, "!=");

            ASSERT_EQUAL(condition->binary_expression.lhs->type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.lhs->identifier.token.lexeme, "var");

            ASSERT_EQUAL(condition->binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.rhs->number.token.lexeme, "0");

            ASSERT_EQUAL(while_statement->statements.statements_count, 1);

            const Ast_Statement* inner_statement = &while_statement->statements.statements[0];
            ASSERT_EQUAL(inner_statement->type, AST_STATEMENT_ASSIGNMENT);

            const Ast_Assignment* assignment = &inner_statement->assignment;
            ASSERT_STRINGS_ARE_EQUAL(assignment->name.token.lexeme, "var");
            ASSERT_EQUAL(assignment->expression.type, AST_EXPRESSION_SUBTRACT);
            ASSERT_EQUAL(assignment->expression.binary_expression.lhs->type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.binary_expression.lhs->identifier.token.lexeme, "var");

            ASSERT_EQUAL(assignment->expression.binary_expression.rhs->type, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(assignment->expression.binary_expression.rhs->number.token.lexeme, "1");
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_call_statements(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    bar(10);"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

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

        {
            const Ast_Statement* statement = &function_definition->statements.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_CALL);

            const Ast_Call_Statement* call_statement = &statement->call_statement;
            const Ast_Call* call = &call_statement->call;

            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_EQUAL(called_expression->type, AST_EXPRESSION_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(called_expression->identifier.token.lexeme, "bar");
            ASSERT_EQUAL(call->arguments_count, 1);

            {
                const Ast_Expression* argument = call->arguments[0];
                ASSERT_EQUAL(argument->type, AST_EXPRESSION_NUMBER);
                ASSERT_STRINGS_ARE_EQUAL(argument->number.token.lexeme, "10");
            }
        }

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }
}

internal void
test_syntax_errors(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    {
        const String_View input = string_view("");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_FALSE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(errors.errors_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(errors.errors[0].message, "Expected identifier, found end of file");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, string_view("<input>"), input);
        parser_create(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_FALSE(parser_parse(context->arena, &parser, &ast));

        ASSERT_EQUAL(errors.errors_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(errors.errors[0].message, "Expected :, found end of file");

        parser_destroy(&parser);
        lexer_destroy(&lexer);

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

#include "eon_errors.c"
#include "eon_lexer.c"
#include "eon_parser.c"
