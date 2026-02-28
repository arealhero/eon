#include <eon/unit_test.h>

#include "eon_lexer.h"
#include "eon_parser.h"
#include "eon_semantics.h"

enum { GLOBAL_SCOPE_INDEX = 1, };

internal void
test_type_inference(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // NOTE(vlad): Testing type inference for literals.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := 10;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := 10.0;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_FLOAT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing type inference for variables (with and without qualifiers).
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a: Int32 = 10;"
                                                  "    b := a;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 2);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            {
                const Ast_Variable_Definition* variable = scope->variables[1];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "b");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a: Int32 = 10;"
                                                  "    b: mutable _ = a;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 2);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            {
                const Ast_Variable_Definition* variable = scope->variables[1];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "b");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_MUTABLE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a: mutable Int32 = 10;"
                                                  "    b := a;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 2);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_MUTABLE);
            }

            {
                const Ast_Variable_Definition* variable = scope->variables[1];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "b");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing type inference with arguments.
    {
        {
            const String_View input = string_view("foo: (argument: Int32) -> void = {"
                                                  "    a := argument;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 2);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "argument");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            {
                const Ast_Variable_Definition* variable = scope->variables[1];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (argument: mutable Int32) -> void = {"
                                                  "    a := argument;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 2);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "argument");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_MUTABLE);
            }

            {
                const Ast_Variable_Definition* variable = scope->variables[1];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: (argument: Int32) -> void = {"
                                                  "    a := 10 + argument * 2;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 2);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "argument");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            {
                const Ast_Variable_Definition* variable = scope->variables[1];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing type inference in return statements.
    {
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

            const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> Int32 = {"
                                                  "    return 10;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("clamp_below: (argument: Int32, threshold: Int32) -> Int32 = {"
                                                  "    if argument > threshold {"
                                                  "        return argument;"
                                                  "    } else {"
                                                  "        return threshold;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("clamp_below_zero: (argument: Int32) -> Int32 = {"
                                                  "    if argument > 0 {"
                                                  "        return argument;"
                                                  "    } else {"
                                                  "        return 0;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing type inference in assignments.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a: mutable _ = 10;"
                                                  "    a = 20;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_MUTABLE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := -10;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := -(10 + 3);"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := -10 + 3;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing type inference in call statements and expressions.
    {
        {
            const String_View input = string_view("foo: () -> Int32 = {"
                                                  "    return bar();"
                                                  "}"
                                                  "bar: () -> Int32 = { return 10; }");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(ast.function_definitions_count, 2);

            {
                const Ast_Function_Definition* function = &ast.function_definitions[0];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);
            }

            {
                const Ast_Function_Definition* function = &ast.function_definitions[1];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);
            }

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            {
                const Ast_Function_Definition* function = &ast.function_definitions[0];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_NOT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* scope = &ast.lexical_scopes[scope_index];
                ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
                ASSERT_EQUAL(scope->variables_count, 0);
            }

            {
                const Ast_Function_Definition* function = &ast.function_definitions[1];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_NOT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* scope = &ast.lexical_scopes[scope_index];
                ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
                ASSERT_EQUAL(scope->variables_count, 0);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    bar(20);"
                                                  "}"
                                                  "bar: (argument: Int32) -> void = {}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(ast.function_definitions_count, 2);

            {
                const Ast_Function_Definition* function = &ast.function_definitions[0];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);
            }

            {
                const Ast_Function_Definition* function = &ast.function_definitions[1];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);
            }

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            {
                const Ast_Function_Definition* function = &ast.function_definitions[0];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_NOT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* scope = &ast.lexical_scopes[scope_index];
                ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
                ASSERT_EQUAL(scope->variables_count, 0);
            }

            {
                const Ast_Function_Definition* function = &ast.function_definitions[1];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_NOT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* scope = &ast.lexical_scopes[scope_index];
                ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
                ASSERT_EQUAL(scope->variables_count, 1);

                {
                    const Ast_Variable_Definition* variable = scope->variables[0];
                    ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "argument");
                    ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                    ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
                }
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := bar(20);"
                                                  "}"
                                                  "bar: (argument: Int32) -> Float32 = { return 1.0; }");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_EQUAL(ast.function_definitions_count, 2);

            {
                const Ast_Function_Definition* function = &ast.function_definitions[0];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);
            }

            {
                const Ast_Function_Definition* function = &ast.function_definitions[1];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);
            }

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            {
                const Ast_Function_Definition* function = &ast.function_definitions[0];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_NOT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* scope = &ast.lexical_scopes[scope_index];
                ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
                ASSERT_EQUAL(scope->variables_count, 1);

                {
                    const Ast_Variable_Definition* variable = scope->variables[0];
                    ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                    ASSERT_EQUAL(variable->type->type, AST_TYPE_FLOAT_32);
                    ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
                }
            }

            {
                const Ast_Function_Definition* function = &ast.function_definitions[1];
                const Index scope_index = function->statements.lexical_scope_index;
                ASSERT_NOT_EQUAL(scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* scope = &ast.lexical_scopes[scope_index];
                ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
                ASSERT_EQUAL(scope->variables_count, 1);

                {
                    const Ast_Variable_Definition* variable = scope->variables[0];
                    ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "argument");
                    ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                    ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
                }
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }
}

internal void
test_lexical_scopes(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // NOTE(vlad): Testing if statements.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := 1;"
                                                  "    if 2 + 2 == 4 {"
                                                  "        a := 2.0;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            ASSERT_EQUAL(function_definition->statements.statements_count, 2);

            {
                const Ast_Statement* statement = &function_definition->statements.statements[1];
                ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);

                const Ast_If_Statement* if_statement = &statement->if_statement;

                {
                    const Ast_Statements* if_statements = &if_statement->if_statements;
                    ASSERT_NOT_EQUAL(if_statements->lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

                    const Lexical_Scope* if_scope = &ast.lexical_scopes[if_statements->lexical_scope_index];
                    ASSERT_EQUAL(if_scope->parent_scope_index, function_scope_index);
                    ASSERT_EQUAL(if_scope->variables_count, 1);

                    const Ast_Variable_Definition* variable = if_scope->variables[0];
                    ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                    ASSERT_EQUAL(variable->type->type, AST_TYPE_FLOAT_32);
                    ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
                }

                {
                    const Ast_Statements* else_statements = &if_statement->else_statements;
                    ASSERT_EQUAL(else_statements->lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);
                }
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := 1;"
                                                  "    if 2 + 2 == 4 {"
                                                  "    } else {"
                                                  "        a := 2.0;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            ASSERT_EQUAL(function_definition->statements.statements_count, 2);

            {
                const Ast_Statement* statement = &function_definition->statements.statements[1];
                ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);

                const Ast_If_Statement* if_statement = &statement->if_statement;

                {
                    const Ast_Statements* if_statements = &if_statement->if_statements;
                    ASSERT_EQUAL(if_statements->lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);
                }

                {
                    const Ast_Statements* else_statements = &if_statement->else_statements;
                    ASSERT_NOT_EQUAL(else_statements->lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

                    const Lexical_Scope* else_scope = &ast.lexical_scopes[else_statements->lexical_scope_index];
                    ASSERT_EQUAL(else_scope->parent_scope_index, function_scope_index);
                    ASSERT_EQUAL(else_scope->variables_count, 1);

                    const Ast_Variable_Definition* variable = else_scope->variables[0];
                    ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                    ASSERT_EQUAL(variable->type->type, AST_TYPE_FLOAT_32);
                    ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
                }
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }

    // NOTE(vlad): Testing while loops.
    {
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := 1;"
                                                  "    while a == 1 {"
                                                  "        a := 2.0;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            ASSERT_EQUAL(function_definition->statements.statements_count, 2);

            {
                const Ast_Statement* statement = &function_definition->statements.statements[1];
                ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

                const Ast_While_Statement* while_statement = &statement->while_statement;
                const Ast_Statements* while_statements = &while_statement->statements;
                ASSERT_NOT_EQUAL(while_statements->lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* while_scope = &ast.lexical_scopes[while_statements->lexical_scope_index];
                ASSERT_EQUAL(while_scope->parent_scope_index, function_scope_index);
                ASSERT_EQUAL(while_scope->variables_count, 1);

                const Ast_Variable_Definition* variable = while_scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_FLOAT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        // NOTE(vlad): Testing that we are not creating an empty lexical scope for while loops with empty body.
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := 1;"
                                                  "    while a == 1 {"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            ASSERT_EQUAL(function_definition->statements.statements_count, 2);

            {
                const Ast_Statement* statement = &function_definition->statements.statements[1];
                ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

                const Ast_While_Statement* while_statement = &statement->while_statement;
                const Ast_Statements* while_statements = &while_statement->statements;
                ASSERT_EQUAL(while_statements->lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        // NOTE(vlad): Yes, this looks bizzare, but this is valid in C, so I decided to leave it be.
        {
            const String_View input = string_view("foo: () -> void = {"
                                                  "    a := 1;"
                                                  "    while a == 1 {"
                                                  "        a := a;"
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
            ASSERT_EQUAL(function_definition->statements.lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            const Index function_scope_index = function_definition->statements.lexical_scope_index;
            ASSERT_NOT_EQUAL(function_scope_index, LAST_LEXICAL_SCOPE_INDEX);

            const Lexical_Scope* scope = &ast.lexical_scopes[function_scope_index];
            ASSERT_EQUAL(scope->parent_scope_index, GLOBAL_SCOPE_INDEX);
            ASSERT_EQUAL(scope->variables_count, 1);

            {
                const Ast_Variable_Definition* variable = scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            ASSERT_EQUAL(function_definition->statements.statements_count, 2);

            {
                const Ast_Statement* statement = &function_definition->statements.statements[1];
                ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

                const Ast_While_Statement* while_statement = &statement->while_statement;
                const Ast_Statements* while_statements = &while_statement->statements;
                ASSERT_NOT_EQUAL(while_statements->lexical_scope_index, LAST_LEXICAL_SCOPE_INDEX);

                const Lexical_Scope* while_scope = &ast.lexical_scopes[while_statements->lexical_scope_index];
                ASSERT_EQUAL(while_scope->parent_scope_index, function_scope_index);
                ASSERT_EQUAL(while_scope->variables_count, 1);

                const Ast_Variable_Definition* variable = while_scope->variables[0];
                ASSERT_STRINGS_ARE_EQUAL(variable->name.token.lexeme, "a");
                ASSERT_EQUAL(variable->type->type, AST_TYPE_INT_32);
                ASSERT_EQUAL(variable->type->qualifiers, AST_QUALIFIER_NONE);
            }

            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }
}

// FIXME(vlad): Test errors:
//                1. a: Int32 = 10.0;
//                2. a := 10; b: Float32 = a;
//                3. a := undefined_variable;
//                4. if <not a boolean> {}
//                5. foo: () -> Int32 = { return; } // Invalid return type.
//                6. foo: () -> Int32 = { return 2.0; } // Invalid return type.
//                7. a := 10; a := 10; // Redefinition of variable.
//                8. a: mutable _ = 10; a = 10.0;
//                etc.

REGISTER_TESTS(
    test_type_inference,
    test_lexical_scopes
)

#include "eon_errors.c"
#include "eon_lexer.c"
#include "eon_parser.c"
#include "eon_semantics.c"
