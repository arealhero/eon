#include <eon/unit_test.h>

#include "eon_ir.h"
#include "eon_semantics.h"

internal void
test_expressions(Test_Context* context)
{
    Errors errors = {0};
    errors_create(&errors, context->arena);

    // NOTE(vlad): Testing addition and subtraction.
    {
        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    return 1 + 2;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            IR ir = {0};
            ir_create(&ir, context->arena, context->arena);
            ASSERT_TRUE(convert_ast_to_ir(&ast, &ir));

            ASSERT_EQUAL(ir.instructions_count, 4);

            const IR_Instruction* instruction = &ir.instructions[0];

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_ASSIGNMENT);

                const SSA_Assignment* assignment = &instruction->assignment;

                const SSA_Variable* result = &assignment->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 0);

                const SSA_Operand* operand = &assignment->operand;
                ASSERT_EQUAL(operand->type, SSA_OPERAND_TYPE_INT32);
                ASSERT_EQUAL(operand->s32_value, 1);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_ASSIGNMENT);
                const SSA_Assignment* assignment = &instruction->assignment;

                const SSA_Variable* result = &assignment->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 1);

                const SSA_Operand* operand = &assignment->operand;
                ASSERT_EQUAL(operand->type, SSA_OPERAND_TYPE_INT32);
                ASSERT_EQUAL(operand->s32_value, 2);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_BINARY_OPERATION);

                const SSA_Binary_Operation* binary_operation = &instruction->binary_operation;
                ASSERT_EQUAL(binary_operation->operation_type, SSA_BINARY_OPERATION_TYPE_ADD);

                const SSA_Operand* lhs = &binary_operation->lhs;
                ASSERT_EQUAL(lhs->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&lhs->variable));
                ASSERT_EQUAL(lhs->variable.version, 0);

                const SSA_Operand* rhs = &binary_operation->rhs;
                ASSERT_EQUAL(rhs->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&rhs->variable));
                ASSERT_EQUAL(rhs->variable.version, 1);

                const SSA_Variable* result = &binary_operation->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 2);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_RETURN);

                const SSA_Return* return_instruction = &instruction->return_instruction;
                ASSERT_FALSE(return_instruction->is_empty);

                const SSA_Operand* return_value = &return_instruction->return_value;
                ASSERT_EQUAL(return_value->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&return_value->variable));
                ASSERT_EQUAL(return_value->variable.version, 2);
            }

            ir_destroy(&ir);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }

        {
            const String_View input = string_view("main: () -> Int32 = {"
                                                  "    return 1 + 2 * 3;"
                                                  "}");

            Lexer lexer = {0};
            Parser parser = {0};

            lexer_create(&lexer, string_view("<input>"), input);
            parser_create(&parser, context->arena, &lexer, &errors);

            Ast ast = {0};
            ASSERT_TRUE(parser_parse(context->arena, &parser, &ast));
            ASSERT_EQUAL(errors.errors_count, 0);

            ASSERT_TRUE(create_lexical_scopes_and_infer_types(context->arena, &ast));

            IR ir = {0};
            ir_create(&ir, context->arena, context->arena);
            ASSERT_TRUE(convert_ast_to_ir(&ast, &ir));

            ASSERT_EQUAL(ir.instructions_count, 6);

            const IR_Instruction* instruction = &ir.instructions[0];

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_ASSIGNMENT);

                const SSA_Assignment* assignment = &instruction->assignment;

                const SSA_Variable* result = &assignment->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 0);

                const SSA_Operand* operand = &assignment->operand;
                ASSERT_EQUAL(operand->type, SSA_OPERAND_TYPE_INT32);
                ASSERT_EQUAL(operand->s32_value, 1);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_ASSIGNMENT);

                const SSA_Assignment* assignment = &instruction->assignment;

                const SSA_Variable* result = &assignment->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 1);

                const SSA_Operand* operand = &assignment->operand;
                ASSERT_EQUAL(operand->type, SSA_OPERAND_TYPE_INT32);
                ASSERT_EQUAL(operand->s32_value, 2);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_ASSIGNMENT);

                const SSA_Assignment* assignment = &instruction->assignment;

                const SSA_Variable* result = &assignment->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 2);

                const SSA_Operand* operand = &assignment->operand;
                ASSERT_EQUAL(operand->type, SSA_OPERAND_TYPE_INT32);
                ASSERT_EQUAL(operand->s32_value, 3);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_BINARY_OPERATION);

                const SSA_Binary_Operation* binary_operation = &instruction->binary_operation;
                ASSERT_EQUAL(binary_operation->operation_type, SSA_BINARY_OPERATION_TYPE_MULTIPLY);

                const SSA_Operand* lhs = &binary_operation->lhs;
                ASSERT_EQUAL(lhs->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&lhs->variable));
                ASSERT_EQUAL(lhs->variable.version, 1);

                const SSA_Operand* rhs = &binary_operation->rhs;
                ASSERT_EQUAL(rhs->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&rhs->variable));
                ASSERT_EQUAL(rhs->variable.version, 2);

                const SSA_Variable* result = &binary_operation->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 3);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_BINARY_OPERATION);

                const SSA_Binary_Operation* binary_operation = &instruction->binary_operation;
                ASSERT_EQUAL(binary_operation->operation_type, SSA_BINARY_OPERATION_TYPE_ADD);

                const SSA_Operand* lhs = &binary_operation->lhs;
                ASSERT_EQUAL(lhs->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&lhs->variable));
                ASSERT_EQUAL(lhs->variable.version, 0);

                const SSA_Operand* rhs = &binary_operation->rhs;
                ASSERT_EQUAL(rhs->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&rhs->variable));
                ASSERT_EQUAL(rhs->variable.version, 3);

                const SSA_Variable* result = &binary_operation->result;
                ASSERT_TRUE(is_temporary_variable(result));
                ASSERT_EQUAL(result->version, 4);

                instruction += 1;
            }

            {
                ASSERT_EQUAL(instruction->type, SSA_TYPE_RETURN);

                const SSA_Return* return_instruction = &instruction->return_instruction;
                ASSERT_FALSE(return_instruction->is_empty);

                const SSA_Operand* return_value = &return_instruction->return_value;
                ASSERT_EQUAL(return_value->type, SSA_OPERAND_TYPE_VARIABLE);
                ASSERT_TRUE(is_temporary_variable(&return_value->variable));
                ASSERT_EQUAL(return_value->variable.version, 4);
            }

            ir_destroy(&ir);
            parser_destroy(&parser);
            lexer_destroy(&lexer);

            clear_errors(&errors);
        }
    }
}

REGISTER_TESTS(
    test_expressions
)

#include "eon_errors.c"
#include "eon_ir.c"
#include "eon_lexer.c"
#include "eon_parser.c"
#include "eon_semantics.c"
