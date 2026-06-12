#include "eon_tac.h"

#include "eon_compilation_context.h"
#include "eon_lexical_scopes.h"
#include "eon_types.h"

enum
{
    INVALID_TAC_INDEX = 0,
};

internal inline Index
emit_tac_instruction(Tac_Function* function, const Tac_Instruction instruction)
{
    append_array(function->instructions_arena, function->instructions, Tac_Instruction, instruction);
    return function->instructions_count - 1;
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
set_tac_instruction_id_by_variable_id(Compilation_Context* context,
                                      const Tac_Variable_Id variable_id,
                                      const Tac_Function* tac_function,
                                      const Index instruction_index)
{
    Tac_Variable* variable = get_tac_variable_by_id(&context->tac, variable_id);
    Symbol* symbol = get_symbol_by_id(context, variable->symbol_id);
    symbol->tac_instruction_id.function_label_id = tac_function->label_id;
    symbol->tac_instruction_id.instruction_index = instruction_index;
}

internal Tac_Operand
create_tac_variable_for_expression(Compilation_Context* context,
                                   const Ast_Expression* expression)
{
    const Tac_Variable_Id id = create_tac_variable(context);

    Tac_Variable* variable = get_tac_variable_by_id(&context->tac, id);

    variable->type_id = expression->type_id;
    variable->is_temporary = true;

    Tac_Operand operand = {0};
    operand.kind = TAC_OPERAND_VARIABLE;
    operand.variable_id = id;

    return operand;
}

internal Tac_Operand
create_tac_constant_for_number(Compilation_Context* context,
                               const Ast_Expression* expression)
{
    const Tac_Constant_Id id = create_tac_constant(context);

    Tac_Constant* constant = get_tac_constant_by_id(&context->tac, id);
    constant->type_id = expression->type_id;

    ASSERT(expression->kind == AST_EXPRESSION_NUMBER);
    constant->ast_number = &expression->number;

    Tac_Operand operand = {0};
    operand.kind = TAC_OPERAND_CONSTANT;
    operand.constant_id = id;

    return operand;
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
            return create_tac_constant_for_number(context, expression);
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("[TAC] String literals are not support yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            const Ast_Identifier* identifier = &expression->identifier;
            const Symbol* identifier_symbol = get_symbol_by_id(context, identifier->symbol_id);
            const Tac_Instruction_Id* instruction_id = &identifier_symbol->tac_instruction_id;

            // TODO(vlad): Support global variables.
            ASSERT(instruction_id->function_label_id.index == tac_function->label_id.index);
            const Tac_Instruction* instruction = &tac_function->instructions[instruction_id->instruction_index];
            ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);

            // XXX(vlad): Make this assertion optional?
            {
                const Tac_Variable* variable = get_tac_variable_by_id(&context->tac, instruction->destination.variable_id);
                ASSERT(variable->is_temporary == false);
            }

            Tac_Operand operand = {0};
            operand.kind = TAC_OPERAND_VARIABLE;
            operand.variable_id = instruction->destination.variable_id;
            return operand;
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

            instruction.destination = create_tac_variable_for_expression(context, expression);
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

            if (definition->has_initial_value)
            {
                instruction.first_argument = lower_expression_to_tac(context,
                                                                     tac_function,
                                                                     &definition->initial_value);
            }

            instruction.destination = create_tac_variable_for_symbol(context, definition->name.symbol_id);

            const Index instruction_index = emit_tac_instruction(tac_function, instruction);
            set_tac_instruction_id_by_variable_id(context,
                                                  instruction.destination.variable_id,
                                                  tac_function,
                                                  instruction_index);
        } break;

        case AST_STATEMENT_ASSIGNMENT:
        {
            const Ast_Assignment* assignment = &statement->assignment;
            const Tac_Operand lhs = lower_expression_to_tac(context, tac_function, &assignment->lhs);
            const Tac_Operand rhs = lower_expression_to_tac(context, tac_function, &assignment->rhs);

            Tac_Instruction instruction = {0};
            instruction.operation = TAC_ASSIGN;
            instruction.destination = lhs;
            instruction.first_argument = rhs;

            emit_tac_instruction(tac_function, instruction);
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

    // NOTE(vlad): Creating sentinel objects for invalid ids.
    {
        const Tac_Function_Label_Id function_label_id = create_tac_function_label(context);
        ASSERT(function_label_id.index == INVALID_TAC_INDEX);

        const Tac_Variable_Id variable_id = create_tac_variable(context);
        ASSERT(variable_id.index == INVALID_TAC_INDEX);

        const Tac_Constant_Id constant_id = create_tac_constant(context);
        ASSERT(constant_id.index == INVALID_TAC_INDEX);
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
        {
            const Tac_Operand operand = create_tac_function_label_for_function(context, ast_function);
            ASSERT(operand.kind == TAC_OPERAND_FUNCTION_LABEL);
            ASSERT(operand.function_label_id.index != INVALID_TAC_INDEX);
            tac_function->label_id = operand.function_label_id;
        }

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

            const Tac_Operand parameter_operand = create_tac_variable_for_symbol(context, parameter->name.symbol_id);

            Tac_Operand parameter_index_operand = {0};
            parameter_index_operand.kind = TAC_OPERAND_PARAMETER_INDEX;
            parameter_index_operand.parameter_index.index = parameter_index;

            Tac_Instruction instruction = {0};
            instruction.operation = TAC_GET_PARAMETER;
            instruction.destination = parameter_operand;
            instruction.first_argument = parameter_index_operand;

            const Index instruction_index = emit_tac_instruction(tac_function, instruction);
            set_tac_instruction_id_by_variable_id(context,
                                                  instruction.destination.variable_id,
                                                  tac_function,
                                                  instruction_index);
        }

        const Type* function_type = get_type_by_id(context, ast_function->type->type_id);
        ASSERT(function_type->kind == TYPE_FUNCTION);

        const Type_Id void_type_id = get_void_type_id(context);
        const Type_Id return_type_id = function_type->function_info.return_type_id;
        Bool should_emit_empty_return_statement = type_ids_are_equal(context, return_type_id, void_type_id);

        const Ast_Code_Block* function_body = &ast_function->body;
        for (Index statement_index = 0;
             statement_index < function_body->statements_count;
             ++statement_index)
        {
            const Ast_Statement* statement = &function_body->statements[statement_index];
            lower_statement_to_tac(context, tac_function, statement);

            // NOTE(vlad): Basic dead code elimination.
            if (statement->type == AST_STATEMENT_RETURN)
            {
                should_emit_empty_return_statement = false;
                break;
            }

            // TODO(vlad): Emit a warning about unreachable code.
            //
            //             Frankly I don't know where should we do that. Maybe it is ok to emit everything (including
            //             implicit TAC_RETURN instruction for void functions) and eliminate dead code somewhere else
            //             (e.g. in SSA passes).
            //
            //             Also if we do that here, maybe we can do that before that, e.g. as a part of the type system
            //             pass. That said, it's hard to properly access this question and come up with a good solution
            //             right now.
            //
            //             (FYI I am writing this comment before the TAC lowering is completed, let alone TAC & SSA
            //             passes).
        }

        if (should_emit_empty_return_statement)
        {
            Tac_Instruction instruction = {0};
            instruction.operation = TAC_RETURN;
            emit_tac_instruction(tac_function, instruction);
        }
    }
}
