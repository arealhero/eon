#include "eon_tac.h"

internal void
create_tac_builder(Tac_Builder* builder, Arena* instructions_arena)
{
    builder->instructions_arena = instructions_arena;
}

internal inline Index
create_temp_variable(Tac_Builder* builder)
{
    return builder->next_temp_id++;
}

internal void
emit_tac_instruction(Tac_Builder* builder,
                     const Tac_Instruction instruction)
{
    grow_array_if_needed(builder->instructions_arena, builder->instructions, Tac_Instruction);
    builder->instructions[builder->instructions_count++] = instruction;
}

internal Tac_Temp
lower_expression_to_tac(Tac_Builder* builder,
                        const Ast_Expression* expression)
{
    switch (expression->type)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        {
            // FIXME(vlad): Remove this ad-hoc handling.
            const String_View lexeme = expression->number.token.lexeme;

            Index fraction_start_index = -1;
            for (Index lexeme_index = 0;
                 lexeme_index < lexeme.length;
                 ++lexeme_index)
            {
                if (lexeme.data[lexeme_index] == '.')
                {
                    fraction_start_index = lexeme_index;
                    break;
                }
            }

            s32 int32;
            if (fraction_start_index == -1)
            {
                if (!parse_integer(lexeme, &int32))
                {
                    FAIL("Failed to parse integer");
                }
            }
            else
            {
                FAIL("[TAC] Floats are not supported yet");
            }

            Tac_Operand result = {0};
            result.kind = TAC_OPERAND_TEMP;
            result.temp.type = TAC_TYPE_INT32;
            result.temp.id = create_temp_variable(builder);

            Tac_Instruction instruction = {0};
            instruction.operation = TAC_MOVE;
            instruction.destination = result;
            instruction.first_argument.kind = TAC_OPERAND_CONSTANT;
            instruction.first_argument.constant.type = TAC_TYPE_INT32;
            instruction.first_argument.constant.int32 = int32;

            emit_tac_instruction(builder, instruction);

            return result.temp;
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("[TAC] String literals are not supported yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            FAIL("[TAC] Identifiers are not supported yet");
        } break;

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        case AST_EXPRESSION_EQUAL:
        case AST_EXPRESSION_NOT_EQUAL:
        case AST_EXPRESSION_LESS:
        case AST_EXPRESSION_LESS_OR_EQUAL:
        case AST_EXPRESSION_GREATER:
        case AST_EXPRESSION_GREATER_OR_EQUAL:
        {
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;

            const Tac_Temp lhs = lower_expression_to_tac(builder, binary_expression->lhs);
            const Tac_Temp rhs = lower_expression_to_tac(builder, binary_expression->rhs);

            if (lhs.type != rhs.type)
            {
                FAIL("[TAC]: LHS and RHS must be of the same type in binary expression");
            }

            Tac_Operand_Type type = lhs.type;

            Tac_Operand result = {0};
            result.kind = TAC_OPERAND_TEMP;
            result.temp.id = create_temp_variable(builder);

            Tac_Operation operation;

            switch (expression->type)
            {
                case AST_EXPRESSION_ADD:
                {
                    operation = TAC_ADD;
                    result.temp.type = type;
                } break;

                case AST_EXPRESSION_SUBTRACT:
                {
                    operation = TAC_SUBTRACT;
                    result.temp.type = type;
                } break;

                case AST_EXPRESSION_MULTIPLY:
                {
                    operation = TAC_MULTIPLY;
                    result.temp.type = type;
                } break;

                case AST_EXPRESSION_DIVIDE:
                {
                    operation = TAC_DIVIDE;
                    result.temp.type = type;
                } break;

                case AST_EXPRESSION_EQUAL:
                {
                    operation = TAC_EQUAL;
                    result.temp.type = TAC_TYPE_BOOL;
                } break;

                case AST_EXPRESSION_NOT_EQUAL:
                {
                    operation = TAC_NOT_EQUAL;
                    result.temp.type = TAC_TYPE_BOOL;
                } break;

                case AST_EXPRESSION_LESS:
                {
                    operation = TAC_LESS;
                    result.temp.type = TAC_TYPE_BOOL;
                } break;

                case AST_EXPRESSION_LESS_OR_EQUAL:
                {
                    operation = TAC_LESS_OR_EQUAL;
                    result.temp.type = TAC_TYPE_BOOL;
                } break;

                case AST_EXPRESSION_GREATER:
                {
                    operation = TAC_GREATER;
                    result.temp.type = TAC_TYPE_BOOL;
                } break;

                case AST_EXPRESSION_GREATER_OR_EQUAL:
                {
                    operation = TAC_GREATER_OR_EQUAL;
                    result.temp.type = TAC_TYPE_BOOL;
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            Tac_Instruction instruction = {0};
            instruction.operation = operation;
            instruction.destination = result;
            instruction.first_argument.kind = TAC_OPERAND_TEMP;
            instruction.first_argument.temp = lhs;
            instruction.second_argument.kind = TAC_OPERAND_TEMP;
            instruction.second_argument.temp = rhs;

            emit_tac_instruction(builder, instruction);

            return result.temp;
        } break;

        case AST_EXPRESSION_NEGATE:
        {
            FAIL("[TAC] Negation is not supported yet");
        } break;

        case AST_EXPRESSION_DEREFERENCE:
        {
            FAIL("[TAC] Dereference is not supported yet");
        } break;

        case AST_EXPRESSION_ADDRESS_OF:
        {
            FAIL("[TAC] Address-of is not supported yet");
        } break;

        case AST_EXPRESSION_CALL:
        {
            FAIL("[TAC] Calls are not supported yet");
        } break;
    }
}
