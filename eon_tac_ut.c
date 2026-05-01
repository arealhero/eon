#include <eon/unit_test.h>

#include "eon_semantics.h"
#include "eon_tac.h"

internal void
test_expression_lowering(Test_Context* context)
{
    Errors errors = {0};
    create_errors(&errors, context->arena);

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    x := 1 + 2 * 3;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parse_ast(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
        const Ast_Statements* statements = &function_definition->statements;

        ASSERT_EQUAL(statements->statements_count, 1);
        const Ast_Statement* statement = &statements->statements[0];

        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

        ASSERT_EQUAL(variable_definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);

        const Ast_Expression* expression = &variable_definition->initial_value;

        Tac_Builder tac_builder = {0};
        create_tac_builder(&tac_builder, context->arena);

        const Tac_Temp expression_temp = lower_expression_to_tac(&tac_builder, expression);
        ASSERT_EQUAL(expression_temp.type, TAC_TYPE_INT32);
        ASSERT_EQUAL(expression_temp.id, 4);

        ASSERT_EQUAL(tac_builder.instructions_count, 5);

        const Tac_Instruction* instruction = tac_builder.instructions;
        {
            ASSERT_EQUAL(instruction->operation, TAC_MOVE);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->destination.temp.id, 0);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(instruction->first_argument.constant.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.constant.int32, 1);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_NONE);

            ++instruction;
        }

        {
            ASSERT_EQUAL(instruction->operation, TAC_MOVE);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->destination.temp.id, 1);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(instruction->first_argument.constant.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.constant.int32, 2);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_NONE);

            ++instruction;
        }

        {
            ASSERT_EQUAL(instruction->operation, TAC_MOVE);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->destination.temp.id, 2);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(instruction->first_argument.constant.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.constant.int32, 3);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_NONE);

            ++instruction;
        }

        {
            ASSERT_EQUAL(instruction->operation, TAC_MULTIPLY);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->destination.temp.id, 3);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->first_argument.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.temp.id, 1);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->second_argument.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->second_argument.temp.id, 2);

            ++instruction;
        }

        {
            ASSERT_EQUAL(instruction->operation, TAC_ADD);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->destination.temp.id, 4);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->first_argument.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.temp.id, 0);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->second_argument.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->second_argument.temp.id, 3);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);

        clear_errors(&errors);
    }

    {
        const String_View input = string_view("foo: () -> void = {"
                                              "    x := 1 != 2;"
                                              "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, string_view("<input>"), input);
        create_parser(&parser, context->arena, &lexer, &errors);

        Ast ast = {0};
        ASSERT_TRUE(parse_ast(context->arena, &parser, &ast));
        ASSERT_EQUAL(errors.errors_count, 0);

        ASSERT_EQUAL(ast.function_definitions_count, 1);

        ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

        const Ast_Function_Definition* function_definition = &ast.function_definitions[0];
        const Ast_Statements* statements = &function_definition->statements;

        ASSERT_EQUAL(statements->statements_count, 1);
        const Ast_Statement* statement = &statements->statements[0];

        ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

        const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

        ASSERT_EQUAL(variable_definition->initialisation_type, AST_INITIALISATION_WITH_VALUE);

        const Ast_Expression* expression = &variable_definition->initial_value;

        Tac_Builder tac_builder = {0};
        create_tac_builder(&tac_builder, context->arena);

        const Tac_Temp expression_temp = lower_expression_to_tac(&tac_builder, expression);
        ASSERT_EQUAL(expression_temp.type, TAC_TYPE_BOOL);
        ASSERT_EQUAL(expression_temp.id, 2);

        ASSERT_EQUAL(tac_builder.instructions_count, 3);

        const Tac_Instruction* instruction = tac_builder.instructions;
        {
            ASSERT_EQUAL(instruction->operation, TAC_MOVE);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->destination.temp.id, 0);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(instruction->first_argument.constant.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.constant.int32, 1);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_NONE);

            ++instruction;
        }

        {
            ASSERT_EQUAL(instruction->operation, TAC_MOVE);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->destination.temp.id, 1);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(instruction->first_argument.constant.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.constant.int32, 2);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_NONE);

            ++instruction;
        }

        {
            ASSERT_EQUAL(instruction->operation, TAC_NOT_EQUAL);

            ASSERT_EQUAL(instruction->destination.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->destination.temp.type, TAC_TYPE_BOOL);
            ASSERT_EQUAL(instruction->destination.temp.id, 2);

            ASSERT_EQUAL(instruction->first_argument.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->first_argument.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->first_argument.temp.id, 0);

            ASSERT_EQUAL(instruction->second_argument.kind, TAC_OPERAND_TEMP);
            ASSERT_EQUAL(instruction->second_argument.temp.type, TAC_TYPE_INT32);
            ASSERT_EQUAL(instruction->second_argument.temp.id, 1);

            ++instruction;
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);

        clear_errors(&errors);
    }
}

REGISTER_TESTS(
    test_expression_lowering
)

#include "eon_errors.c"
#include "eon_lexer.c"
#include "eon_parser.c"
#include "eon_semantics.c"
#include "eon_tac.c"
