#include "eon_tac.h"

#include "eon_compilation_context.h"
#include "eon_lexical_scopes.h"

enum
{
    INVALID_TAC_ID = 0,
};

internal inline void
emit_tac_instruction(Tac_Function* function,
                     const Tac_Instruction instruction)
{
    append_array(function->instructions_arena, function->instructions, Tac_Instruction, instruction);
}

// FIXME(vlad): Return 'Tac_Operand'.
internal inline Tac_Variable
create_tac_variable_for_symbol(Compilation_Context* context,
                               const Symbol_Id symbol_id)
{
    Tac* tac = &context->tac;

    const Symbol* symbol = get_symbol_by_id(context, symbol_id);

    Tac_Variable variable = {0};
    variable.id = ++tac->next_variable_id;
    variable.type_id = symbol->type_id;
    variable.is_temporary = false;
    variable.symbol_id = symbol_id;

    return variable;
}

// FIXME(vlad): Return 'Tac_Operand'.
internal inline Tac_Variable
create_tac_variable_for_expression(Compilation_Context* context,
                                   const Type_Id expression_type_id)
{
    Tac* tac = &context->tac;

    Tac_Variable variable = {0};
    variable.id = ++tac->next_variable_id;
    variable.type_id = expression_type_id;
    variable.is_temporary = true;

    return variable;
}

internal inline Tac_Constant
create_tac_constant_for_number(Compilation_Context* context,
                               const Ast_Expression* expression)
{
    Tac* tac = &context->tac;

    Tac_Constant constant = {0};
    constant.id = ++tac->next_constant_id;
    constant.type_id = expression->type_id;

    ASSERT(expression->kind == AST_EXPRESSION_NUMBER);
    constant.ast_number = &expression->number;

    return constant;
}

internal Tac_Operand
lower_expression_to_tac(Compilation_Context* context,
                        Tac_Function* tac_function,
                        const Ast_Expression* expression)
{
    switch (expression->kind)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        {
            Tac_Operand result = {0};
            result.kind = TAC_OPERAND_CONSTANT;
            result.constant = create_tac_constant_for_number(context, expression);
            return result;
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("[TAC] String literals are not support yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            FAIL("[TAC] Identifiers are not support yet");
        } break;

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        {
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;
            const Tac_Operand lhs = lower_expression_to_tac(context, tac_function, binary_expression->lhs);
            const Tac_Operand rhs = lower_expression_to_tac(context, tac_function, binary_expression->rhs);

            Tac_Instruction instruction = {0};

            switch (expression->kind)
            {
                case AST_EXPRESSION_ADD:
                {
                    instruction.operation = TAC_ADD;
                } break;

                case AST_EXPRESSION_SUBTRACT:
                {
                    instruction.operation = TAC_SUBTRACT;
                } break;

                case AST_EXPRESSION_MULTIPLY:
                {
                    instruction.operation = TAC_MULTIPLY;
                } break;

                case AST_EXPRESSION_DIVIDE:
                {
                    instruction.operation = TAC_DIVIDE;
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            instruction.destination.kind = TAC_OPERAND_VARIABLE;
            instruction.destination.variable = create_tac_variable_for_expression(context, expression->type_id);

            instruction.first_argument = lhs;
            instruction.second_argument = rhs;

            emit_tac_instruction(tac_function, instruction);
            return instruction.destination;
        } break;

        case AST_EXPRESSION_EQUAL:
        case AST_EXPRESSION_NOT_EQUAL:
        case AST_EXPRESSION_LESS:
        case AST_EXPRESSION_LESS_OR_EQUAL:
        case AST_EXPRESSION_GREATER:
        case AST_EXPRESSION_GREATER_OR_EQUAL:
        {
            FAIL("[TAC] Comparisons are not support yet");
        } break;

        case AST_EXPRESSION_NEGATE:
        {
            FAIL("[TAC] Negations are not support yet");
        } break;

        case AST_EXPRESSION_DEREFERENCE:
        {
            FAIL("[TAC] Dereferences are not support yet");
        } break;

        case AST_EXPRESSION_ADDRESS_OF:
        {
            FAIL("[TAC] Address-of expressions are not support yet");
        } break;

        case AST_EXPRESSION_CALL:
        {
            FAIL("[TAC] Calls are not support yet");
        } break;
    }
}

internal void
lower_statement_to_tac(Compilation_Context* context,
                       Tac_Function* tac_function,
                       const Ast_Statement* statement)
{
    switch (statement->type)
    {
        case AST_STATEMENT_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_STATEMENT_VARIABLE_DEFINITION:
        {
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            Tac_Instruction instruction = {0};
            instruction.operation = TAC_ASSIGN;
            instruction.destination.kind = TAC_OPERAND_VARIABLE;

            if (definition->has_initial_value)
            {
                instruction.first_argument = lower_expression_to_tac(context,
                                                                     tac_function,
                                                                     &definition->initial_value);
            }

            instruction.destination.variable = create_tac_variable_for_symbol(context, definition->name.symbol_id);

            emit_tac_instruction(tac_function, instruction);
        } break;

        case AST_STATEMENT_ASSIGNMENT:
        {
            FAIL("[TAC] Assignments are not support yet");
        } break;

        case AST_STATEMENT_RETURN:
        {
            FAIL("[TAC] Return statements are not support yet");
        } break;

        case AST_STATEMENT_WHILE:
        {
            FAIL("[TAC] While loops are not support yet");
        } break;

        case AST_STATEMENT_IF:
        {
            FAIL("[TAC] If statements are not support yet");
        } break;

        case AST_STATEMENT_CALL:
        {
            FAIL("[TAC] Call statements are not support yet");
        } break;
    }
}

internal void
lower_ast_to_tac(Compilation_Context* context)
{
    const Ast* ast = &context->ast;
    Tac* tac = &context->tac;

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

        tac_function->label.id = ++tac->next_function_label_id;
        tac_function->label.symbol_id = ast_function->name.symbol_id;

        ASSERT(tac_function->label.id != INVALID_TAC_ID);

        tac_function->instructions_arena = acquire_arena_from_provider(context->arena_provider,
                                                                       string_view("tac-function-instructions"),
                                                                       GiB(1),
                                                                       MiB(1));

        ASSERT(ast_function->type->kind == AST_TYPE_FUNCTION);
        const Ast_Function_Type* ast_function_type = &ast_function->type->function;

        for (Index parameter_index = 0;
             parameter_index < ast_function_type->parameters_count;
             ++parameter_index)
        {
            const Ast_Function_Parameter* parameter = &ast_function_type->parameters[parameter_index];

            Tac_Operand parameter_operand = {0};
            parameter_operand.kind = TAC_OPERAND_VARIABLE;
            parameter_operand.variable = create_tac_variable_for_symbol(context, parameter->name.symbol_id);

            Tac_Operand parameter_index_operand = {0};
            parameter_index_operand.kind = TAC_OPERAND_PARAMETER_INDEX;
            parameter_index_operand.parameter_index.index = parameter_index;

            Tac_Instruction instruction = {0};
            instruction.operation = TAC_GET_PARAMETER;
            instruction.destination = parameter_operand;
            instruction.first_argument = parameter_index_operand;

            emit_tac_instruction(tac_function, instruction);
        }

        const Ast_Code_Block* function_body = &ast_function->body;
        for (Index statement_index = 0;
             statement_index < function_body->statements_count;
             ++statement_index)
        {
            const Ast_Statement* statement = &function_body->statements[statement_index];
            lower_statement_to_tac(context, tac_function, statement);
        }

        // FIXME(vlad): Emit 'TAC_RETURN' here e.g. for void functions.
    }
}
