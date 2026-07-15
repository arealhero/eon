#include "eon_unit_test.h"

#include "eon_ast.h"

#include "eon_parser.h"

internal void
test_break_and_continue_statements_validation(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    while 1 != 2\n"
                                                 "    {\n"
                                                 "        break;\n"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&function_type->location,
                                          "() -> void");

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&return_type->location, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->start_location.line, 1);
            ASSERT_EQUAL(statement->start_location.column, 4);

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            const Ast_Expression* condition = &while_statement->condition;
            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->kind, AST_EXPRESSION_NOT_EQUAL);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.operator.lexeme, "!=");

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.lhs->number.token.lexeme, "1");
            ASSERT_FALSE(condition->binary_expression.lhs->number.is_a_floating_point_number);

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.rhs->number.token.lexeme, "2");
            ASSERT_FALSE(condition->binary_expression.rhs->number.is_a_floating_point_number);

            ASSERT_EQUAL(while_statement->body.statements_count, 1);

            const Ast_Statement* inner_statement = &while_statement->body.statements[0];
            ASSERT_EQUAL(inner_statement->start_location.line, 3);
            ASSERT_EQUAL(inner_statement->start_location.column, 8);

            ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
            ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
            ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    while 1 != 2\n"
                                                 "    {\n"
                                                 "        continue;\n"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&function_type->location,
                                          "() -> void");

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&return_type->location, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->start_location.line, 1);
            ASSERT_EQUAL(statement->start_location.column, 4);

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            const Ast_Expression* condition = &while_statement->condition;
            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->kind, AST_EXPRESSION_NOT_EQUAL);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.operator.lexeme, "!=");

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.lhs->number.token.lexeme, "1");
            ASSERT_FALSE(condition->binary_expression.lhs->number.is_a_floating_point_number);

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.rhs->number.token.lexeme, "2");
            ASSERT_FALSE(condition->binary_expression.rhs->number.is_a_floating_point_number);

            ASSERT_EQUAL(while_statement->body.statements_count, 1);

            const Ast_Statement* inner_statement = &while_statement->body.statements[0];
            ASSERT_EQUAL(inner_statement->start_location.line, 3);
            ASSERT_EQUAL(inner_statement->start_location.column, 8);

            ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_CONTINUE);
            ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "continue");
            ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    while 1 != 2\n"
                                                 "    {\n"
                                                 "        break;\n"
                                                 "        break;\n"
                                                 "        break;\n"
                                                 "        break;\n"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&function_type->location,
                                          "() -> void");

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "void");
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&return_type->location, "void");

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->start_location.line, 1);
            ASSERT_EQUAL(statement->start_location.column, 4);

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            const Ast_Expression* condition = &while_statement->condition;
            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->kind, AST_EXPRESSION_NOT_EQUAL);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.operator.lexeme, "!=");

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.lhs->number.token.lexeme, "1");
            ASSERT_FALSE(condition->binary_expression.lhs->number.is_a_floating_point_number);

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.rhs->number.token.lexeme, "2");
            ASSERT_FALSE(condition->binary_expression.rhs->number.is_a_floating_point_number);

            ASSERT_EQUAL(while_statement->body.statements_count, 4);

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[0];
                ASSERT_EQUAL(inner_statement->start_location.line, 3);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[1];
                ASSERT_EQUAL(inner_statement->start_location.line, 4);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[2];
                ASSERT_EQUAL(inner_statement->start_location.line, 5);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[3];
                ASSERT_EQUAL(inner_statement->start_location.line, 6);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
    {

        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    while 1 != 2\n"
                                                 "    {\n"
                                                 "        break;\n"
                                                 "        break;\n"
                                                 "        break;\n"
                                                 "        break;\n"
                                                 "    }\n"
                                                 "    return 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_STRINGS_ARE_EQUAL(function_definition->name.token.lexeme, "foo");

        const Ast_Type* function_type = function_definition->type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, AST_TYPE_FUNCTION);
        ASSERT_FALSE(function_type->is_mutable);
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&function_type->location, "() -> s32");

        ASSERT_EQUAL(function_type->function.parameters_count, 0);

        const Ast_Type* return_type = function_type->function.return_type;
        ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, AST_TYPE_NAME);
        ASSERT_FALSE(return_type->is_mutable);
        ASSERT_STRINGS_ARE_EQUAL(return_type->named_type.token.lexeme, "s32");
        ASSERT_LOCATION_STRINGS_ARE_EQUAL(&return_type->location, "s32");

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->start_location.line, 1);
            ASSERT_EQUAL(statement->start_location.column, 4);

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            const Ast_Expression* condition = &while_statement->condition;
            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->kind, AST_EXPRESSION_NOT_EQUAL);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.operator.lexeme, "!=");

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.lhs->number.token.lexeme, "1");
            ASSERT_FALSE(condition->binary_expression.lhs->number.is_a_floating_point_number);

            ASSERT_ENUM_VALUES_ARE_EQUAL(condition->binary_expression.rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(condition->binary_expression.rhs->number.token.lexeme, "2");
            ASSERT_FALSE(condition->binary_expression.rhs->number.is_a_floating_point_number);

            ASSERT_EQUAL(while_statement->body.statements_count, 4);

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[0];
                ASSERT_EQUAL(inner_statement->start_location.line, 3);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[1];
                ASSERT_EQUAL(inner_statement->start_location.line, 4);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[2];
                ASSERT_EQUAL(inner_statement->start_location.line, 5);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }

            {
                const Ast_Statement* inner_statement = &while_statement->body.statements[3];
                ASSERT_EQUAL(inner_statement->start_location.line, 6);
                ASSERT_EQUAL(inner_statement->start_location.column, 8);

                ASSERT_ENUM_VALUES_ARE_EQUAL(inner_statement->kind, AST_STATEMENT_BREAK);
                ASSERT_STRINGS_ARE_EQUAL(inner_statement->jump.token.lexeme, "break");
                ASSERT_POINTERS_ARE_EQUAL(inner_statement->jump.destination, statement);
            }
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->start_location.line, 8);
            ASSERT_EQUAL(statement->start_location.column, 4);

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
            ASSERT_FALSE(statement->return_statement.is_empty);
            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->return_statement.expression.kind, AST_EXPRESSION_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(statement->return_statement.expression.number.token.lexeme, "10");
            ASSERT_FALSE(statement->return_statement.expression.number.is_a_floating_point_number);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_invalid_break_and_continue_statements(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    break;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_TRUE(has_compilation_errors(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:5: error: 'break' statement outside of loop statements is not allowed\n"
                                                        "  2 |     break;\n"
                                                        "    |     ^~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    continue;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_TRUE(has_compilation_errors(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:5: error: 'continue' statement outside of loop statements is not allowed\n"
                                                        "  2 |     continue;\n"
                                                        "    |     ^~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    if 1 == 2\n"
                                                 "    {\n"
                                                 "        continue;\n"
                                                 "    }\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_TRUE(has_compilation_errors(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:9: error: 'continue' statement outside of loop statements is not allowed\n"
                                                        "  4 |         continue;\n"
                                                        "    |         ^~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    while 1 == 2 {}\n"
                                                 "    if 1 == 2\n"
                                                 "    {\n"
                                                 "        continue;\n"
                                                 "    }\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_TRUE(has_compilation_errors(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:5:9: error: 'continue' statement outside of loop statements is not allowed\n"
                                                        "  5 |         continue;\n"
                                                        "    |         ^~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_break_and_continue_statements_validation,
    test_invalid_break_and_continue_statements
)

#include "eon_ast.c"
#include "eon_cfg.c"
#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_tac.c"
#include "eon_types.c"
