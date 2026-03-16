#include "eon_ir.h"

internal void
ir_create(IR* ir,
          Arena* instructions_arena,
          Arena* labels_arena)
{
    ir->instructions_arena = instructions_arena;
    ir->labels_arena = labels_arena;
}

internal void
ir_destroy(IR* ir)
{
    UNUSED(ir);
}

internal void
add_label_for_function_with_current_ssa_index(IR* ir,
                                              const String_View function_name)
{
    if (ir->labels_count == ir->labels_capacity)
    {
        // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
        const Size new_capacity = MAX(1, 2 * ir->labels_capacity);
        ir->labels = reallocate(ir->labels_arena,
                                ir->labels,
                                IR_Label,
                                ir->labels_capacity,
                                new_capacity);
        ir->labels_capacity = new_capacity;
    }

    IR_Label* new_label = &ir->labels[ir->labels_count++];
    new_label->name = function_name;
    new_label->ssa_index = ir->instructions_count;
}

internal inline void
create_temporary_variable(IR* ir, SSA_Variable* variable)
{
    variable->version = ir->next_temporary_variable_index++;
}

internal IR_Instruction*
create_empty_instruction(IR* ir)
{
    if (ir->instructions_count == ir->instructions_capacity)
    {
        // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
        const Size new_capacity = MAX(1, 2 * ir->instructions_capacity);
        ir->instructions = reallocate(ir->instructions_arena,
                                      ir->instructions,
                                      IR_Instruction,
                                      ir->instructions_capacity,
                                      new_capacity);
        ir->instructions_capacity = new_capacity;
    }

    return &ir->instructions[ir->instructions_count++];
}

// TODO(vlad): Change to SSA_Operand and return values for numbers without creating separate variables?
internal SSA_Variable
convert_expression_to_ir(IR* ir, const Ast_Expression* expression)
{
    switch (expression->type)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        {
            IR_Instruction* instruction = create_empty_instruction(ir);
            instruction->type = SSA_TYPE_ASSIGNMENT;

            SSA_Assignment* assignment = &instruction->assignment;
            create_temporary_variable(ir, &assignment->result);

            assignment->operand.type = SSA_OPERAND_TYPE_INT32;

            // FIXME(vlad): Remove this ad-hoc handling.
            //              This MUST be done during semantic analysis.
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

            if (fraction_start_index == -1)
            {
                if (!parse_integer(lexeme, &assignment->operand.s32_value))
                {
                    FAIL("IR: Failed to parse integer");
                }
            }
            else
            {
                FAIL("IR: floating-point numbers are not supported yet");
            }

            return assignment->result;
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("IR: string literals are not supported yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            FAIL("IR: identifiers are not supported yet");
        } break;

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        {
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;

            const SSA_Variable lhs = convert_expression_to_ir(ir, binary_expression->lhs);
            const SSA_Variable rhs = convert_expression_to_ir(ir, binary_expression->rhs);

            IR_Instruction* instruction = create_empty_instruction(ir);
            instruction->type = SSA_TYPE_BINARY_OPERATION;

            create_temporary_variable(ir, &instruction->binary_operation.result);

            instruction->binary_operation.lhs.type = SSA_OPERAND_TYPE_VARIABLE;
            instruction->binary_operation.lhs.variable = lhs;

            instruction->binary_operation.rhs.type = SSA_OPERAND_TYPE_VARIABLE;
            instruction->binary_operation.rhs.variable = rhs;

            switch (expression->type)
            {
                case AST_EXPRESSION_ADD:
                {
                    instruction->binary_operation.operation_type = SSA_BINARY_OPERATION_TYPE_ADD;
                } break;

                case AST_EXPRESSION_SUBTRACT:
                {
                    instruction->binary_operation.operation_type = SSA_BINARY_OPERATION_TYPE_SUBTRACT;
                } break;

                case AST_EXPRESSION_MULTIPLY:
                {
                    instruction->binary_operation.operation_type = SSA_BINARY_OPERATION_TYPE_MULTIPLY;
                } break;

                case AST_EXPRESSION_DIVIDE:
                {
                    instruction->binary_operation.operation_type = SSA_BINARY_OPERATION_TYPE_DIVIDE;
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            return instruction->binary_operation.result;
        } break;

        case AST_EXPRESSION_EQUAL:
        {
            FAIL("IR: comparisons are not supported yet");
        } break;

        case AST_EXPRESSION_NOT_EQUAL:
        {
            FAIL("IR: comparisons are not supported yet");
        } break;

        case AST_EXPRESSION_LESS:
        {
            FAIL("IR: comparisons are not supported yet");
        } break;

        case AST_EXPRESSION_LESS_OR_EQUAL:
        {
            FAIL("IR: comparisons are not supported yet");
        } break;

        case AST_EXPRESSION_GREATER:
        {
            FAIL("IR: comparisons are not supported yet");
        } break;

        case AST_EXPRESSION_GREATER_OR_EQUAL:
        {
            FAIL("IR: comparisons are not supported yet");
        } break;

        case AST_EXPRESSION_NEGATE:
        {
            FAIL("IR: negate expressions are not supported yet");
        } break;

        case AST_EXPRESSION_DEREFERENCE:
        {
            FAIL("IR: dereference expressions are not supported yet");
        } break;

        case AST_EXPRESSION_ADDRESS_OF:
        {
            FAIL("IR: address-of expressions are not supported yet");
        } break;

        case AST_EXPRESSION_CALL:
        {
            FAIL("IR: call expressions are not supported yet");
        } break;
    }
}

internal Bool
convert_function_definition_to_ir(IR* ir, const Ast_Function_Definition* function_definition)
{
    // TODO(vlad): Support function overloading: provide argument types to the label
    //             or use name mangling here.
    add_label_for_function_with_current_ssa_index(ir, function_definition->name.token.lexeme);

    const Ast_Statements* statements = &function_definition->statements;
    for (Index statement_index = 0;
         statement_index < statements->statements_count;
         ++statement_index)
    {
        const Ast_Statement* statement = &statements->statements[statement_index];

        switch (statement->type)
        {
            case AST_STATEMENT_UNDEFINED:
            {
                UNREACHABLE();
            } break;

            case AST_STATEMENT_VARIABLE_DEFINITION:
            {
                FAIL("IR: variable definitions are not supported yet");
            } break;

            case AST_STATEMENT_ASSIGNMENT:
            {
                FAIL("IR: assignments are not supported yet");
            } break;

            case AST_STATEMENT_RETURN:
            {
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                SSA_Variable return_statement_expression = {0};
                if (!return_statement->is_empty)
                {
                    return_statement_expression = convert_expression_to_ir(ir,
                                                                           &return_statement->expression);
                }

                IR_Instruction* instruction = create_empty_instruction(ir);
                instruction->type = SSA_TYPE_RETURN;

                SSA_Return* return_instruction = &instruction->return_instruction;

                return_instruction->is_empty = return_statement->is_empty;
                if (!return_statement->is_empty)
                {
                    SSA_Operand* return_value = &return_instruction->return_value;
                    return_value->type = SSA_OPERAND_TYPE_VARIABLE;
                    return_value->variable = return_statement_expression;
                }
            } break;

            case AST_STATEMENT_WHILE:
            {
                FAIL("IR: while loops are not supported yet");
            } break;

            case AST_STATEMENT_IF:
            {
                FAIL("IR: if statements are not supported yet");
            } break;

            case AST_STATEMENT_CALL:
            {
                FAIL("IR: calls are not supported yet");
            } break;
        }
    }

    return true;
}

internal Bool
convert_ast_to_ir(const Ast* ast, IR* ir)
{
    for (Index function_index = 0;
         function_index < ast->function_definitions_count;
         ++function_index)
    {
        const Ast_Function_Definition* function = &ast->function_definitions[function_index];

        if (!convert_function_definition_to_ir(ir, function))
        {
            return false;
        }
    }

    return true;
}
