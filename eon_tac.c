#include "eon_tac.h"

#include "eon_compilation_context.h"
#include "eon_lexical_scopes.h"
#include "eon_types.h"

enum
{
    GLOBAL_TAC_FUNCTION_LABEL_INDEX = 1,
};

internal inline Tac_Instruction*
add_new_instruction_to_tac_function(Compilation_Context* context,
                                    Tac_Function* function,
                                    Ast_Statement* statement,
                                    Ast_Expression* expression)
{
    Tac_Instruction* instruction = allocate(context->tac_instructions_arena, Tac_Instruction);

    if (statement)
    {
        instruction->ast_statement = statement;
    }

    if (expression)
    {
        ASSERT(instruction->ast_statement != NULL);
        instruction->ast_expression = expression;
    }

    instruction->was_automatically_inserted = (instruction->ast_statement == NULL
                                               && instruction->ast_expression == NULL);

    if (function->first_instruction == NULL)
    {
        function->first_instruction = instruction;
        function->last_instruction = instruction;
    }
    else
    {
        ASSERT(function->last_instruction != NULL);

        instruction->previous_instruction = function->last_instruction;
        function->last_instruction->next_instruction = instruction;
        function->last_instruction = instruction;
    }

    return instruction;
}

internal Tac_Function_Label_Id
create_tac_function_label(Compilation_Context* context)
{
    Tac* tac = &context->tac;
    append_array(context->tac_function_labels_arena, tac->function_labels, Tac_Function_Label, (Tac_Function_Label){0});

    Tac_Function_Label_Id id = {0};
    id.index = tac->function_labels_count - 1;
    return id;
}

internal Tac_Variable_Id
create_tac_variable(Compilation_Context* context)
{
    Tac* tac = &context->tac;
    append_array(context->tac_variables_arena, tac->variables, Tac_Variable, (Tac_Variable){0});

    Tac_Variable_Id id = {0};
    id.index = tac->variables_count - 1;
    return id;
}

internal Tac_Constant_Id
create_tac_constant(Compilation_Context* context)
{
    Tac* tac = &context->tac;
    append_array(context->tac_constants_arena, tac->constants, Tac_Constant, (Tac_Constant){0});

    Tac_Constant_Id id = {0};
    id.index = tac->constants_count - 1;
    return id;
}

internal Tac_Label_Id
create_tac_label(Compilation_Context* context)
{
    Tac* tac = &context->tac;
    append_array(context->tac_labels_arena, tac->labels, Tac_Label, (Tac_Label){0});

    Tac_Label_Id id = {0};
    id.index = tac->labels_count - 1;
    return id;
}

internal inline Tac_Function*
get_tac_function_by_label(Tac* tac, const Tac_Function_Label_Id label_id)
{
    ASSERT(label_id.index != INVALID_TAC_INDEX);

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* function = &tac->functions[function_index];
        if (function->label_id.index == label_id.index)
        {
            return function;
        }
    }

    UNREACHABLE();
}

internal inline Tac_Function_Label*
get_tac_function_label_by_id(Tac* tac, const Tac_Function_Label_Id id)
{
    ASSERT(0 <= id.index && id.index < tac->function_labels_count);
    ASSERT(id.index != INVALID_TAC_INDEX);
    return &tac->function_labels[id.index];
}

internal inline Tac_Variable*
get_tac_variable_by_id(Tac* tac, const Tac_Variable_Id id)
{
    ASSERT(0 <= id.index && id.index < tac->variables_count);
    ASSERT(id.index != INVALID_TAC_INDEX);
    return &tac->variables[id.index];
}

internal inline Tac_Constant*
get_tac_constant_by_id(Tac* tac, const Tac_Constant_Id id)
{
    ASSERT(0 <= id.index && id.index < tac->constants_count);
    ASSERT(id.index != INVALID_TAC_INDEX);
    return &tac->constants[id.index];
}

internal inline Tac_Label*
get_tac_label_by_id(Tac* tac, const Tac_Label_Id id)
{
    ASSERT(0 <= id.index && id.index < tac->labels_count);
    ASSERT(id.index != INVALID_TAC_INDEX);
    return &tac->labels[id.index];
}

internal Tac_Operand
create_tac_function_label_for_function(Compilation_Context* context,
                                       const Ast_Function_Definition* definition)
{
    const Tac_Function_Label_Id id = create_tac_function_label(context);

    Tac_Function_Label* function_label = get_tac_function_label_by_id(&context->tac, id);
    function_label->symbol_id = definition->name.symbol_id;

    Tac_Operand operand = {0};
    operand.kind = TAC_OPERAND_FUNCTION_LABEL;
    operand.function_label_id = id;

    return operand;
}

internal Tac_Operand
create_tac_variable_for_symbol(Compilation_Context* context,
                               const Symbol_Id symbol_id)
{
    const Tac_Variable_Id id = create_tac_variable(context);

    Tac_Variable* variable = get_tac_variable_by_id(&context->tac, id);
    const Symbol* symbol = get_symbol_by_id(context, symbol_id);

    variable->type_id = symbol->type_id;
    variable->is_temporary = false;
    variable->symbol_id = symbol_id;

    Tac_Operand operand = {0};
    operand.kind = TAC_OPERAND_VARIABLE;
    operand.variable_id = id;

    return operand;
}

internal void
set_variable_definition_in_tac(Compilation_Context* context, Tac_Instruction* instruction)
{
    ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);

    Tac_Variable* variable = get_tac_variable_by_id(&context->tac, instruction->destination.variable_id);
    Symbol* symbol = get_symbol_by_id(context, variable->symbol_id);
    symbol->defined_at_tac_instruction = instruction;
}

internal Tac_Operand
create_tac_temporary_variable(Compilation_Context* context,
                              const Type_Id type_id)
{
    const Tac_Variable_Id id = create_tac_variable(context);

    Tac_Variable* variable = get_tac_variable_by_id(&context->tac, id);

    variable->type_id = type_id;
    variable->is_temporary = true;

    Tac_Operand operand = {0};
    operand.kind = TAC_OPERAND_VARIABLE;
    operand.variable_id = id;

    return operand;
}

// TODO(vlad): Remove code duplication here and in type-to-string conversion function.
internal Tac_Constant_Kind
get_constant_kind_by_type_id(Compilation_Context* context,
                             const Type_Id type_id)
{
    const Type* type = get_type_by_id(context, type_id);

    switch (type->kind)
    {
        case TYPE_NUMBER_VARIABLE:
        {
            const Number_Constraints* constraints = &type->number_constraints;

            if (constraints->must_be_a_floating_point_number)
            {
                switch (constraints->min_bit_width)
                {
                    case 0:
                    case 32:
                    {
                        return TAC_CONSTANT_FLOAT32;
                    } break;

                    case 64:
                    {
                        return TAC_CONSTANT_FLOAT64;
                    } break;

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }
            }
            else
            {
                if (constraints->sign == SIGN_UNSIGNED)
                {
                    switch (constraints->min_bit_width)
                    {
                        case 8:
                        {
                            return TAC_CONSTANT_UINT8;
                        } break;

                        case 16:
                        {
                            return TAC_CONSTANT_UINT16;
                        } break;

                        case 0:
                        case 32:
                        {
                            return TAC_CONSTANT_UINT32;
                        } break;

                        case 64:
                        {
                            return TAC_CONSTANT_UINT64;
                        } break;

                        default:
                        {
                            UNREACHABLE();
                        } break;
                    }
                }
                else
                {
                    switch (constraints->min_bit_width)
                    {
                        case 8:
                        {
                            return TAC_CONSTANT_INT8;
                        } break;

                        case 16:
                        {
                            return TAC_CONSTANT_INT16;
                        } break;

                        case 0:
                        case 32:
                        {
                            return TAC_CONSTANT_INT32;
                        } break;

                        case 64:
                        {
                            return TAC_CONSTANT_INT64;
                        } break;

                        default:
                        {
                            UNREACHABLE();
                        } break;
                    }
                }
            }
        } break;

        case TYPE_INTEGER:
        {
            const Integer_Type_Info* integer_info = &type->integer_info;

            if (integer_info->is_signed)
            {
                switch (integer_info->width_in_bits)
                {
                    case 8:
                    {
                        return TAC_CONSTANT_INT8;
                    } break;

                    case 16:
                    {
                        return TAC_CONSTANT_INT16;
                    } break;

                    case 0:
                    case 32:
                    {
                        return TAC_CONSTANT_INT32;
                    } break;

                    case 64:
                    {
                        return TAC_CONSTANT_INT64;
                    } break;

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }
            }
            else
            {
                switch (integer_info->width_in_bits)
                {
                    case 8:
                    {
                        return TAC_CONSTANT_UINT8;
                    } break;

                    case 16:
                    {
                        return TAC_CONSTANT_UINT16;
                    } break;

                    case 0:
                    case 32:
                    {
                        return TAC_CONSTANT_UINT32;
                    } break;

                    case 64:
                    {
                        return TAC_CONSTANT_UINT64;
                    } break;

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }
            }
        } break;

        case TYPE_FLOAT:
        {
            const Float_Type_Info* float_info = &type->float_info;

            switch (float_info->width_in_bits)
            {
                case 0:
                case 32:
                {
                    return TAC_CONSTANT_FLOAT32;
                } break;

                case 64:
                {
                    return TAC_CONSTANT_FLOAT64;
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }
        } break;

        case TYPE_BOOLEAN:
        {
            return TAC_CONSTANT_BOOLEAN;
        } break;

        case TYPE_UNDEFINED:
        case TYPE_INVALID:
        case TYPE_VOID:
        case TYPE_VARIABLE:
        case TYPE_POINTER:
        case TYPE_FUNCTION:
        {
            UNREACHABLE();
        } break;
    }

    UNREACHABLE();
}

internal Tac_Operand
create_tac_constant_for_number(Compilation_Context* context,
                               const Ast_Expression* expression)
{
    ASSERT(expression->kind == AST_EXPRESSION_NUMBER);

    const Tac_Constant_Id id = create_tac_constant(context);

    Tac_Constant* constant = get_tac_constant_by_id(&context->tac, id);
    constant->kind = get_constant_kind_by_type_id(context, expression->type_id);

    const Ast_Number* number = &expression->number;

    switch (constant->kind)
    {
        case TAC_CONSTANT_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case TAC_CONSTANT_BOOLEAN:
        {
            FAIL("[TAC] Booleans are not supported yet");
        } break;

        case TAC_CONSTANT_INT8:
        case TAC_CONSTANT_INT16:
        case TAC_CONSTANT_INT32:
        case TAC_CONSTANT_INT64:
        case TAC_CONSTANT_UINT8:
        case TAC_CONSTANT_UINT16:
        case TAC_CONSTANT_UINT32:
        case TAC_CONSTANT_UINT64:
        {
            ASSERT(parse_integer(number->token.lexeme, &constant->integer_value));
        } break;

        case TAC_CONSTANT_FLOAT32:
        {
            ASSERT(parse_float(number->token.lexeme, &constant->float32_value));
        } break;

        case TAC_CONSTANT_FLOAT64:
        {
            ASSERT(parse_float(number->token.lexeme, &constant->float64_value));
        } break;
    }

    Tac_Operand operand = {0};
    operand.kind = TAC_OPERAND_CONSTANT;
    operand.constant_id = id;

    return operand;
}

internal Tac_Operand
lower_expression_to_tac(Compilation_Context* context,
                        Tac_Function* tac_function,
                        Ast_Statement* parent_statement,
                        Ast_Expression* expression)
{
    Tac_Operand result = {0};

    switch (expression->kind)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        {
            result = create_tac_constant_for_number(context, expression);
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("[TAC] String literals are not supported yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            const Ast_Identifier* identifier = &expression->identifier;
            const Symbol* identifier_symbol = get_symbol_by_id(context, identifier->symbol_id);

            if (identifier_symbol->is_builtin)
            {
                result.kind = TAC_OPERAND_VARIABLE;

                const Tac_Constant_Id constant_id = create_tac_constant(context);

                Tac_Constant* constant = get_tac_constant_by_id(&context->tac, constant_id);
                constant->kind = TAC_CONSTANT_BOOLEAN;

                if (strings_are_equal(identifier_symbol->name, string_view("true")))
                {
                    constant->boolean_value = true;
                }
                else if (strings_are_equal(identifier_symbol->name, string_view("false")))
                {
                    constant->boolean_value = false;
                }
                else
                {
                    FAIL("[TAC] Unknown builtin symbol encountered");
                }

                result.kind = TAC_OPERAND_CONSTANT;
                result.constant_id = constant_id;
                break;
            }

            if (identifier_symbol->is_a_global_function)
            {
                result.kind = TAC_OPERAND_FUNCTION_LABEL;
                result.function_label_id = identifier_symbol->tac_function_label_id;
                break;
            }

            // TODO(vlad): Support global variables.
            const Tac_Instruction* instruction = identifier_symbol->defined_at_tac_instruction;
            ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);

            // XXX(vlad): Make this assertion optional?
            {
                const Tac_Variable* variable = get_tac_variable_by_id(&context->tac, instruction->destination.variable_id);
                ASSERT(variable->is_temporary == false);
            }

            result.kind = TAC_OPERAND_VARIABLE;
            result.variable_id = instruction->destination.variable_id;
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
            const Tac_Operand lhs = lower_expression_to_tac(context, tac_function, parent_statement, binary_expression->lhs);
            const Tac_Operand rhs = lower_expression_to_tac(context, tac_function, parent_statement, binary_expression->rhs);

            Tac_Instruction* instruction = add_new_instruction_to_tac_function(context,
                                                                               tac_function,
                                                                               parent_statement,
                                                                               expression);

            switch (expression->kind)
            {
                case AST_EXPRESSION_ADD:
                {
                    instruction->operation = TAC_ADD;
                } break;

                case AST_EXPRESSION_SUBTRACT:
                {
                    instruction->operation = TAC_SUBTRACT;
                } break;

                case AST_EXPRESSION_MULTIPLY:
                {
                    instruction->operation = TAC_MULTIPLY;
                } break;

                case AST_EXPRESSION_DIVIDE:
                {
                    instruction->operation = TAC_DIVIDE;
                } break;

                case AST_EXPRESSION_EQUAL:
                {
                    instruction->operation = TAC_EQUAL;
                } break;

                case AST_EXPRESSION_NOT_EQUAL:
                {
                    instruction->operation = TAC_NOT_EQUAL;
                } break;

                case AST_EXPRESSION_LESS:
                {
                    instruction->operation = TAC_LESS;
                } break;

                case AST_EXPRESSION_LESS_OR_EQUAL:
                {
                    instruction->operation = TAC_LESS_OR_EQUAL;
                } break;

                case AST_EXPRESSION_GREATER:
                {
                    instruction->operation = TAC_GREATER;
                } break;

                case AST_EXPRESSION_GREATER_OR_EQUAL:
                {
                    instruction->operation = TAC_GREATER_OR_EQUAL;
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            instruction->destination = create_tac_temporary_variable(context, expression->type_id);
            instruction->first_argument = lhs;
            instruction->second_argument = rhs;

            result = instruction->destination;
        } break;

        case AST_EXPRESSION_NEGATE:
        {
            FAIL("[TAC] Negations are not supported yet");
        } break;

        case AST_EXPRESSION_DEREFERENCE:
        {
            const Ast_Unary_Expression* address_of_expression = &expression->unary_expression;

            const Tac_Operand operand = lower_expression_to_tac(context,
                                                                tac_function,
                                                                parent_statement,
                                                                address_of_expression->operand);
            ASSERT(operand.kind == TAC_OPERAND_VARIABLE);

            Tac_Instruction* instruction = add_new_instruction_to_tac_function(context,
                                                                               tac_function,
                                                                               parent_statement,
                                                                               expression);
            instruction->operation = TAC_LOAD_BY_ADDRESS;
            instruction->destination = create_tac_temporary_variable(context, expression->type_id);
            instruction->first_argument = operand;

            result = instruction->destination;
        } break;

        case AST_EXPRESSION_ADDRESS_OF:
        {
            const Ast_Unary_Expression* address_of_expression = &expression->unary_expression;

            const Tac_Operand operand = lower_expression_to_tac(context,
                                                                tac_function,
                                                                parent_statement,
                                                                address_of_expression->operand);
            ASSERT(operand.kind == TAC_OPERAND_VARIABLE);

            Tac_Instruction* instruction = add_new_instruction_to_tac_function(context,
                                                                               tac_function,
                                                                               parent_statement,
                                                                               expression);
            instruction->operation = TAC_GET_ADDRESS;
            instruction->destination = create_tac_temporary_variable(context, expression->type_id);
            instruction->first_argument = operand;

            result = instruction->destination;
        } break;

        case AST_EXPRESSION_CALL:
        {
            const Ast_Call* call = &expression->call;

            const Tac_Operand called_expression_operand = lower_expression_to_tac(context,
                                                                                  tac_function,
                                                                                  parent_statement,
                                                                                  call->called_expression);

            for (Index argument_index = call->arguments_count - 1;
                 argument_index >= 0;
                 --argument_index)
            {
                Ast_Expression* argument = call->arguments[argument_index];

                const Tac_Operand argument_operand = lower_expression_to_tac(context,
                                                                             tac_function,
                                                                             parent_statement,
                                                                             argument);

                Tac_Instruction* instruction = add_new_instruction_to_tac_function(context,
                                                                                   tac_function,
                                                                                   parent_statement,
                                                                                   expression);
                instruction->operation = TAC_SET_PARAMETER;
                instruction->first_argument = argument_operand;
            }

            Tac_Instruction* call_instruction = add_new_instruction_to_tac_function(context,
                                                                                    tac_function,
                                                                                    parent_statement,
                                                                                    expression);
            call_instruction->operation = TAC_CALL;

            {
                const Type* called_expression_type = get_type_by_id(context, call->called_expression->type_id);
                ASSERT(called_expression_type->kind == TYPE_FUNCTION);

                const Type_Id return_type_id = called_expression_type->function_info.return_type_id;
                const Type_Id void_type_id = get_void_type_id(context);

                if (!type_ids_are_equal(context, return_type_id, void_type_id))
                {
                    call_instruction->destination = create_tac_temporary_variable(context, expression->type_id);
                }
            }

            call_instruction->first_argument = called_expression_operand;

            {
                Tac_Operand number_of_arguments_operand = {0};
                number_of_arguments_operand.kind = TAC_OPERAND_NUMBER_OF_ARGUMENTS;
                number_of_arguments_operand.number_of_arguments = call->arguments_count;

                call_instruction->second_argument = number_of_arguments_operand;
            }

            result = call_instruction->destination;
        } break;
    }

    return result;
}

internal void
lower_statement_to_tac(Compilation_Context* context,
                       Tac_Function* tac_function,
                       Ast_Statement* statement)
{
    switch (statement->kind)
    {
        case AST_STATEMENT_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_STATEMENT_VARIABLE_DEFINITION:
        {
            Ast_Variable_Definition* definition = &statement->variable_definition;

            Tac_Operand initial_value = {0};
            if (definition->has_initial_value)
            {
                initial_value = lower_expression_to_tac(context,
                                                        tac_function,
                                                        statement,
                                                        &definition->initial_value);
            }

            Tac_Instruction* instruction = add_new_instruction_to_tac_function(context, tac_function, statement, NULL);
            instruction->operation = TAC_ASSIGN;
            instruction->destination = create_tac_variable_for_symbol(context, definition->name.symbol_id);
            instruction->first_argument = initial_value;

            set_variable_definition_in_tac(context, instruction);
        } break;

        case AST_STATEMENT_ASSIGNMENT:
        {
            Ast_Assignment* assignment = &statement->assignment;

            const Tac_Operand rhs = lower_expression_to_tac(context, tac_function, statement, &assignment->rhs);

            if (assignment->lhs.kind == AST_EXPRESSION_DEREFERENCE)
            {
                const Ast_Unary_Expression* lhs_dereference = &assignment->lhs.unary_expression;
                const Tac_Operand lhs_dereference_operand = lower_expression_to_tac(context,
                                                                                    tac_function,
                                                                                    statement,
                                                                                    lhs_dereference->operand);

                Tac_Instruction* instruction = add_new_instruction_to_tac_function(context,
                                                                                   tac_function,
                                                                                   statement,
                                                                                   NULL);
                instruction->operation = TAC_STORE_BY_ADDRESS;
                instruction->destination = lhs_dereference_operand;
                instruction->first_argument = rhs;
            }
            else
            {
                const Tac_Operand lhs = lower_expression_to_tac(context, tac_function, statement, &assignment->lhs);

                Tac_Instruction* instruction = add_new_instruction_to_tac_function(context, tac_function, statement, NULL);
                instruction->operation = TAC_ASSIGN;
                instruction->destination = lhs;
                instruction->first_argument = rhs;
            }
        } break;

        case AST_STATEMENT_RETURN:
        {
            Ast_Return_Statement* return_statement = &statement->return_statement;

            Tac_Operand return_value = {0};
            if (!return_statement->is_empty)
            {
                return_value = lower_expression_to_tac(context,
                                                       tac_function,
                                                       statement,
                                                       &return_statement->expression);
            }

            Tac_Instruction* instruction = add_new_instruction_to_tac_function(context, tac_function, statement, NULL);
            instruction->operation = TAC_RETURN;
            instruction->first_argument = return_value;
        } break;

        case AST_STATEMENT_WHILE:
        {
            Ast_While_Statement* while_statement = &statement->while_statement;

            const Tac_Label_Id start_label_id = create_tac_label(context);
            const Tac_Label_Id end_label_id = create_tac_label(context);

            while_statement->start_label_id = start_label_id;
            while_statement->end_label_id = end_label_id;

            {
                Tac_Instruction* start_label_instruction = add_new_instruction_to_tac_function(context,
                                                                                               tac_function,
                                                                                               statement,
                                                                                               NULL);
                start_label_instruction->operation = TAC_LABEL;
                start_label_instruction->destination.kind = TAC_OPERAND_LABEL;
                start_label_instruction->destination.label_id = start_label_id;

                Tac_Label* start_label = get_tac_label_by_id(&context->tac, start_label_id);
                start_label->points_to = start_label_instruction;
            }

            {
                const Tac_Operand condition_operand = lower_expression_to_tac(context,
                                                                              tac_function,
                                                                              statement,
                                                                              &while_statement->condition);

                Tac_Instruction* condition_instruction = add_new_instruction_to_tac_function(context,
                                                                                             tac_function,
                                                                                             statement,
                                                                                             NULL);
                condition_instruction->operation = TAC_JUMP_IF_FALSE;
                condition_instruction->destination.kind = TAC_OPERAND_LABEL;
                condition_instruction->destination.label_id = end_label_id;
                condition_instruction->first_argument = condition_operand;
            }

            {
                const Ast_Code_Block* body = &while_statement->body;
                for (Index statement_index = 0;
                     statement_index < body->statements_count;
                     ++statement_index)
                {
                    Ast_Statement* body_statement = &body->statements[statement_index];
                    lower_statement_to_tac(context, tac_function, body_statement);
                }
            }

            {
                Tac_Instruction* loop_instruction = add_new_instruction_to_tac_function(context,
                                                                                        tac_function,
                                                                                        statement,
                                                                                        NULL);
                loop_instruction->operation = TAC_JUMP;
                loop_instruction->destination.kind = TAC_OPERAND_LABEL;
                loop_instruction->destination.label_id = start_label_id;
                loop_instruction->was_automatically_inserted = true;
            }

            {
                Tac_Instruction* end_label_instruction = add_new_instruction_to_tac_function(context,
                                                                                             tac_function,
                                                                                             statement,
                                                                                             NULL);
                end_label_instruction->operation = TAC_LABEL;
                end_label_instruction->destination.kind = TAC_OPERAND_LABEL;
                end_label_instruction->destination.label_id = end_label_id;
                end_label_instruction->was_automatically_inserted = true;

                Tac_Label* end_label = get_tac_label_by_id(&context->tac, end_label_id);
                end_label->points_to = end_label_instruction;
            }
        } break;

        case AST_STATEMENT_IF:
        {
            // TODO(vlad): Optimize instructions in case of empty if/else block?
            //             Or maybe it is a good idea to optimize them out after the CFG construction.

            Ast_If_Statement* if_statement = &statement->if_statement;

            const Tac_Label_Id else_label_id = create_tac_label(context);
            const Tac_Label_Id end_label_id = create_tac_label(context);

            {
                const Tac_Operand condition_operand = lower_expression_to_tac(context,
                                                                              tac_function,
                                                                              statement,
                                                                              &if_statement->condition);

                Tac_Instruction* condition_instruction = add_new_instruction_to_tac_function(context,
                                                                                             tac_function,
                                                                                             statement,
                                                                                             NULL);
                condition_instruction->operation = TAC_JUMP_IF_FALSE;
                condition_instruction->destination.kind = TAC_OPERAND_LABEL;
                condition_instruction->destination.label_id = else_label_id;
                condition_instruction->first_argument = condition_operand;
            }

            {
                const Ast_Code_Block* then_code_block = &if_statement->if_statements;
                for (Index statement_index = 0;
                     statement_index < then_code_block->statements_count;
                     ++statement_index)
                {
                    Ast_Statement* then_statement = &then_code_block->statements[statement_index];
                    lower_statement_to_tac(context, tac_function, then_statement);
                }
            }

            {
                Tac_Instruction* jump_after_then_instruction = add_new_instruction_to_tac_function(context,
                                                                                                   tac_function,
                                                                                                   statement,
                                                                                                   NULL);
                jump_after_then_instruction->operation = TAC_JUMP;
                jump_after_then_instruction->destination.kind = TAC_OPERAND_LABEL;
                jump_after_then_instruction->destination.label_id = end_label_id;
                jump_after_then_instruction->was_automatically_inserted = true;
            }

            {
                Tac_Instruction* else_label_instruction = add_new_instruction_to_tac_function(context,
                                                                                              tac_function,
                                                                                              statement,
                                                                                              NULL);
                else_label_instruction->operation = TAC_LABEL;
                else_label_instruction->destination.kind = TAC_OPERAND_LABEL;
                else_label_instruction->destination.label_id = else_label_id;

                Tac_Label* else_label = get_tac_label_by_id(&context->tac, else_label_id);
                else_label->points_to = else_label_instruction;
            }

            {
                const Ast_Code_Block* else_code_block = &if_statement->else_statements;
                for (Index statement_index = 0;
                     statement_index < else_code_block->statements_count;
                     ++statement_index)
                {
                    Ast_Statement* else_statement = &else_code_block->statements[0];
                    lower_statement_to_tac(context, tac_function, else_statement);
                }
            }

            {
                Tac_Instruction* end_label_instruction = add_new_instruction_to_tac_function(context,
                                                                                             tac_function,
                                                                                             statement,
                                                                                             NULL);
                end_label_instruction->operation = TAC_LABEL;
                end_label_instruction->destination.kind = TAC_OPERAND_LABEL;
                end_label_instruction->destination.label_id = end_label_id;

                Tac_Label* end_label = get_tac_label_by_id(&context->tac, end_label_id);
                end_label->points_to = end_label_instruction;
            }
        } break;

        case AST_STATEMENT_CALL:
        {
            Ast_Call_Statement* call_statement = &statement->call_statement;
            ASSERT(call_statement->call_expression.kind == AST_EXPRESSION_CALL);
            lower_expression_to_tac(context, tac_function, statement, &call_statement->call_expression);
        } break;

        case AST_STATEMENT_BREAK:
        {
            Ast_Jump* jump = &statement->jump;

            ASSERT(jump->destination != NULL);

            Ast_Statement* loop = jump->destination;

            switch (loop->kind)
            {
                case AST_STATEMENT_WHILE:
                {
                    Ast_While_Statement* while_statement = &loop->while_statement;

                    Tac_Instruction* loop_instruction = add_new_instruction_to_tac_function(context,
                                                                                            tac_function,
                                                                                            statement,
                                                                                            NULL);
                    loop_instruction->operation = TAC_JUMP;
                    loop_instruction->destination.kind = TAC_OPERAND_LABEL;

                    ASSERT(while_statement->end_label_id.index != INVALID_TAC_INDEX);
                    loop_instruction->destination.label_id = while_statement->end_label_id;
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }
        } break;

        case AST_STATEMENT_CONTINUE:
        {
            Ast_Jump* jump = &statement->jump;

            ASSERT(jump->destination != NULL);

            Ast_Statement* loop = jump->destination;

            switch (loop->kind)
            {
                case AST_STATEMENT_WHILE:
                {
                    Ast_While_Statement* while_statement = &loop->while_statement;

                    Tac_Instruction* loop_instruction = add_new_instruction_to_tac_function(context,
                                                                                            tac_function,
                                                                                            statement,
                                                                                            NULL);
                    loop_instruction->operation = TAC_JUMP;
                    loop_instruction->destination.kind = TAC_OPERAND_LABEL;

                    ASSERT(while_statement->start_label_id.index != INVALID_TAC_INDEX);
                    loop_instruction->destination.label_id = while_statement->start_label_id;
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }
        } break;
    }
}

internal void
lower_ast_to_tac(Compilation_Context* context)
{
    const Ast* ast = &context->ast;
    Tac* tac = &context->tac;

    // NOTE(vlad): Creating sentinel objects for invalid ids.
    {
        const Tac_Function_Label_Id function_label_id = create_tac_function_label(context);
        ASSERT(function_label_id.index == INVALID_TAC_INDEX);

        const Tac_Variable_Id variable_id = create_tac_variable(context);
        ASSERT(variable_id.index == INVALID_TAC_INDEX);

        const Tac_Constant_Id constant_id = create_tac_constant(context);
        ASSERT(constant_id.index == INVALID_TAC_INDEX);

        const Tac_Label_Id label_id = create_tac_label(context);
        ASSERT(label_id.index == INVALID_TAC_INDEX);
    }

    // NOTE(vlad): Creating variable ids for global functions.
    {
        for (Index function_index = 0;
             function_index < ast->function_definitions_count;
             ++function_index)
        {
            const Ast_Function_Definition* ast_function = &ast->function_definitions[function_index];

            const Tac_Operand operand = create_tac_function_label_for_function(context, ast_function);
            ASSERT(operand.kind == TAC_OPERAND_FUNCTION_LABEL);
            ASSERT(operand.function_label_id.index != INVALID_TAC_INDEX);

            Symbol* function_symbol = get_symbol_by_id(context, ast_function->name.symbol_id);
            ASSERT(function_symbol->is_a_global_function);
            function_symbol->tac_function_label_id = operand.function_label_id;
        }
    }

    for (Index function_index = 0;
         function_index < ast->function_definitions_count;
         ++function_index)
    {
        const Ast_Function_Definition* ast_function = &ast->function_definitions[function_index];

        append_array(context->tac_functions_arena,
                     tac->functions,
                     Tac_Function,
                     (Tac_Function){0});

        Tac_Function* tac_function = &tac->functions[tac->functions_count - 1];
        tac_function->ast_function_definition = ast_function;

        {
            const Symbol* function_symbol = get_symbol_by_id(context, ast_function->name.symbol_id);
            ASSERT(function_symbol->is_a_global_function);
            ASSERT(function_symbol->tac_function_label_id.index != INVALID_TAC_INDEX);

            tac_function->label_id = function_symbol->tac_function_label_id;
        }

        tac_function->first_tac_variable_index = tac->variables_count;
        tac_function->first_tac_label_index = tac->labels_count;

        ASSERT(ast_function->type->kind == AST_TYPE_FUNCTION);
        const Ast_Function_Type* ast_function_type = &ast_function->type->function;

        for (Index parameter_index = 0;
             parameter_index < ast_function_type->parameters_count;
             ++parameter_index)
        {
            const Ast_Function_Parameter* parameter = &ast_function_type->parameters[parameter_index];

            const Tac_Operand parameter_operand = create_tac_variable_for_symbol(context, parameter->name.symbol_id);

            Tac_Operand parameter_index_operand = {0};
            parameter_index_operand.kind = TAC_OPERAND_PARAMETER_INDEX;
            parameter_index_operand.parameter_index.index = parameter_index;

            Tac_Instruction* instruction = add_new_instruction_to_tac_function(context, tac_function, NULL, NULL);
            instruction->operation = TAC_GET_PARAMETER;
            instruction->destination = parameter_operand;
            instruction->first_argument = parameter_index_operand;

            set_variable_definition_in_tac(context, instruction);
        }

        const Type* function_type = get_type_by_id(context, ast_function->type->type_id);
        ASSERT(function_type->kind == TYPE_FUNCTION);

        const Type_Id void_type_id = get_void_type_id(context);
        const Type_Id return_type_id = function_type->function_info.return_type_id;

        const Ast_Code_Block* function_body = &ast_function->body;
        for (Index statement_index = 0;
             statement_index < function_body->statements_count;
             ++statement_index)
        {
            Ast_Statement* statement = &function_body->statements[statement_index];
            lower_statement_to_tac(context, tac_function, statement);
        }

        const Bool should_emit_empty_return_statement = type_ids_are_equal(context, return_type_id, void_type_id);
        if (should_emit_empty_return_statement)
        {
            Tac_Instruction* instruction = add_new_instruction_to_tac_function(context, tac_function, NULL, NULL);
            instruction->operation = TAC_RETURN;
        }

        tac_function->last_tac_variable_index = tac->variables_count;
        tac_function->last_tac_label_index = tac->labels_count;
    }
}
