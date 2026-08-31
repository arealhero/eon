#include "converters.h"

#include <eon_compilation_context.h>
#include <eon_lexical_scopes.h>
#include <eon_cfg.h>

// FIXME(vlad): Use 'tac_function->first_tac_variable_index' instead.
struct Conversion_Context
{
    Index temporary_variables_offset;
};
typedef struct Conversion_Context Conversion_Context;

internal void
convert_tac_operand_to_string(Compilation_Context* context,
                              String_Builder* builder,
                              const Tac_Operand* operand,
                              Conversion_Context* conversion_context)
{
    Tac* tac = &context->tac;

    switch (operand->kind)
    {
        case TAC_OPERAND_NONE:
        {
        } break;

        case TAC_OPERAND_FUNCTION_LABEL:
        {
            const Tac_Function_Label_Id function_label_id = operand->function_label_id;
            const Tac_Function* function = get_tac_function_by_label(tac, function_label_id);

            append_string(builder, " ");
            append_string(builder, function->ast_function_definition->name.token.lexeme);
        } break;

        case TAC_OPERAND_VARIABLE:
        {
            const Tac_Variable_Id variable_id = operand->variable_id;
            ASSERT(variable_id.index != INVALID_TAC_INDEX);
            ASSERT(variable_id.ssa_version != SSA_VERSION_UNDEFINED);
            ASSERT(variable_id.ssa_version != SSA_VERSION_UNSET);

            const Tac_Variable* variable = &tac->variables[variable_id.index];

            String_View name = {0};

            if (variable->is_temporary)
            {
                if (conversion_context->temporary_variables_offset == 0)
                {
                    conversion_context->temporary_variables_offset = variable_id.index;
                }

                const Index temporary_variable_index = variable_id.index - conversion_context->temporary_variables_offset + 1;
                name = string_view(format_string(context->scratch_arena, "<temp_{}>", temporary_variable_index));
            }
            else
            {
                const Symbol* symbol = get_symbol_by_id(context, variable->symbol_id);
                name = symbol->name;
            }

            append_string(builder, format_string(context->scratch_arena, " VARIABLE {}@{}", name, variable_id.ssa_version));
        } break;

        case TAC_OPERAND_LABEL:
        {
            ASSERT(operand->label_id.index != INVALID_TAC_INDEX);
            append_string(builder, format_string(context->scratch_arena, " LABEL_{}", operand->label_id.index));
        } break;

        case TAC_OPERAND_CONSTANT:
        {
            const Tac_Constant_Id constant_id = operand->constant_id;
            ASSERT(constant_id.index != INVALID_TAC_INDEX);

            const Tac_Constant* constant = &tac->constants[constant_id.index];

            append_string(builder, " CONSTANT ");

            String_View constant_string = {0};

            switch (constant->kind)
            {
                case TAC_CONSTANT_UNDEFINED:
                {
                    UNREACHABLE();
                } break;

                case TAC_CONSTANT_BOOLEAN:
                {
                    if (constant->boolean_value)
                    {
                        constant_string = string_view("bool TRUE");
                    }
                    else
                    {
                        constant_string = string_view("bool FALSE");
                    }
                } break;

#define DECLARE_INTEGER_CASE(kind, Type)                                \
                case kind:                                              \
                {                                                       \
                    constant_string = string_view(format_string(context->scratch_arena, #Type " {}", constant->integer_value)); \
                } break

                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT8,  s8);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT16, s16);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT32, s32);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT64, s64);

                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT8,  u8);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT16, u16);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT32, u32);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT64, u64);

                case TAC_CONSTANT_FLOAT32:
                {
                    constant_string = string_view(format_string(context->scratch_arena, "f32 {}", constant->float32_value));
                } break;

                case TAC_CONSTANT_FLOAT64:
                {
                    constant_string = string_view(format_string(context->scratch_arena, "f64 {}", constant->float64_value));
                } break;
            }

            ASSERT(!strings_are_equal(constant_string, string_view("")));

            append_string(builder, constant_string);
        } break;

        case TAC_OPERAND_PARAMETER_INDEX:
        {
            append_string(builder, format_string(context->scratch_arena, " ARGUMENT {}", operand->parameter_index.index));
        } break;

        case TAC_OPERAND_NUMBER_OF_ARGUMENTS:
        {
            append_string(builder, format_string(context->scratch_arena, " ARGUMENTS COUNT {}", operand->number_of_arguments));
        } break;
    }
}

internal String_View
convert_ssa_to_string(Arena* arena, Compilation_Context* context)
{
    const Tac* tac = &context->tac;

    String_Builder builder = {0};
    create_string_builder(&builder, arena);

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        const Tac_Function* tac_function = &tac->functions[function_index];
        const Ast_Function_Definition* ast_definition = tac_function->ast_function_definition;

        append_string(&builder, ast_definition->name.token.lexeme);
        append_string(&builder, ":\n");

        Index current_instruction_index = 1;

        Conversion_Context conversion_context = {0};

        for (Index block_index = 0;
             block_index < tac_function->cfg_blocks_count;
             ++block_index)
        {
            const Cfg_Block* block = &tac_function->cfg_blocks[block_index];

            if (block->phi_nodes_count > 0)
            {
                append_string(&builder, "       |\n");
            }

            for (Index phi_node_index = 0;
                 phi_node_index < block->phi_nodes_count;
                 ++phi_node_index)
            {
                append_string(&builder, "       | ");
                append_string(&builder, "          PHI             ");
                const Phi_Node* phi_node = &block->phi_nodes[phi_node_index];

                ASSERT(phi_node->previous_variables_count == block->predecessors_count);

                Tac_Operand destination = {0};
                destination.kind = TAC_OPERAND_VARIABLE;
                destination.variable_id = phi_node->destination;

                convert_tac_operand_to_string(context, &builder, &destination, &conversion_context);

                for (Index argument_index = 0;
                     argument_index < phi_node->previous_variables_count;
                     ++argument_index)
                {
                    Tac_Operand previous_variable = {0};
                    previous_variable.kind = TAC_OPERAND_VARIABLE;
                    previous_variable.variable_id = phi_node->previous_variables[argument_index];

                    if (previous_variable.variable_id.ssa_version != SSA_VERSION_UNSET)
                    {
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &previous_variable, &conversion_context);
                    }
                }

                append_string(&builder, "\n");
            }

            for (const Tac_Instruction* instruction = block->first_tac_instruction;
                 instruction != NULL;
                 instruction = instruction->next_instruction)
            {
                if (instruction->operation == TAC_NOP)
                {
                    // NOTE(vlad): Sanity check.
                    ASSERT(instruction->destination.kind == TAC_OPERAND_NONE);
                    ASSERT(instruction->first_argument.kind == TAC_OPERAND_NONE);
                    ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                    continue;
                }

                append_string(&builder, format_string(context->scratch_arena,
                                                      "{left-pad-count: 6} | ",
                                                      current_instruction_index));
                current_instruction_index += 1;

                switch (instruction->operation)
                {
                    case TAC_NOP:
                    {
                        UNREACHABLE();
                    } break;

                    case TAC_ASSIGN:
                    {
                        append_string(&builder, "          ASSIGN          ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_GET_ADDRESS:
                    {
                        append_string(&builder, "          GET_ADDRESS     ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_LOAD_BY_ADDRESS:
                    {
                        append_string(&builder, "          LOAD_BY_ADDRESS ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_STORE_BY_ADDRESS:
                    {
                        append_string(&builder, "          STORE_BY_ADDRESS");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_ADD:
                    {
                        append_string(&builder, "          ADD             ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_SUBTRACT:
                    {
                        append_string(&builder, "          SUBTRACT        ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_MULTIPLY:
                    {
                        append_string(&builder, "          MULTIPLY        ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_DIVIDE:
                    {
                        append_string(&builder, "          DIVIDE          ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_EQUAL:
                    {
                        append_string(&builder, "          EQUAL           ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_NOT_EQUAL:
                    {
                        append_string(&builder, "          NOT_EQUAL       ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_LESS:
                    {
                        append_string(&builder, "          LESS            ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_LESS_OR_EQUAL:
                    {
                        append_string(&builder, "          LESS_OR_EQUAL   ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_GREATER:
                    {
                        append_string(&builder, "          GREATER         ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_GREATER_OR_EQUAL:
                    {
                        append_string(&builder, "          GREATER_OR_EQUAL");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_LABEL:
                    {
                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        ASSERT(instruction->destination.label_id.index != INVALID_TAC_INDEX);
                        append_string(&builder, format_string(context->scratch_arena,
                                                              "LABEL_{}:",
                                                              instruction->destination.label_id.index));
                    } break;

                    case TAC_JUMP:
                    {
                        append_string(&builder, "          JUMP            ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                    } break;

                    case TAC_JUMP_IF_TRUE:
                    {
                        append_string(&builder, "          JUMP_IF_TRUE    ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_JUMP_IF_FALSE:
                    {
                        append_string(&builder, "          JUMP_IF_FALSE   ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_SET_PARAMETER:
                    {
                        append_string(&builder, "          SET_PARAMETER   ");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_GET_PARAMETER:
                    {
                        append_string(&builder, "          GET_PARAMETER   ");

                        ASSERT(instruction->destination.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->first_argument.kind == TAC_OPERAND_PARAMETER_INDEX);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_CALL:
                    {
                        append_string(&builder, "          CALL            ");
                        ASSERT(instruction->first_argument.kind == TAC_OPERAND_FUNCTION_LABEL);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NUMBER_OF_ARGUMENTS);

                        if (instruction->destination.kind != TAC_OPERAND_NONE)
                        {
                            convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                            append_string(&builder, ",");
                        }

                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, ",");
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_RETURN:
                    {
                        append_string(&builder, "          RETURN");

                        ASSERT(instruction->destination.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        if (instruction->first_argument.kind != TAC_OPERAND_NONE)
                        {
                            append_string(&builder, "          ");
                            convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        }
                    } break;
                }

                append_string(&builder, "\n");
            }
        }

        if (function_index != tac->functions_count - 1)
        {
            append_string(&builder, "\n");
        }
    }

    return string_builder_to_string(&builder);
}

internal void
convert_mir_operand_to_string(Compilation_Context* context,
                              String_Builder* builder,
                              const MIR_Operand* operand)
{
    switch (operand->kind)
    {
        case MIR_OPERAND_NONE:
        {
        } break;

        case MIR_OPERAND_VIRTUAL_REGISTER:
        {
            // TODO(vlad): Print register kind here.
            const Virtual_Register* virtual_register = operand->virtual_register;
            ASSERT(virtual_register->sequence_number != 0);
            append_string(builder, format_string(context->scratch_arena,
                                                 " VREG {}",
                                                 virtual_register->sequence_number));
        } break;

        case MIR_OPERAND_IMMEDIATE_VALUE:
        {
            const u64 value = operand->immediate_value;
            append_string(builder, format_string(context->scratch_arena,
                                                 " IMM {}",
                                                 value));
        } break;

        case MIR_OPERAND_BLOCK:
        {
            const MIR_Block* block = operand->block;
            append_string(builder, format_string(context->scratch_arena,
                                                 "LABEL_{}",
                                                 block->sequence_number));
        } break;
    }
}

internal String_View
convert_mir_to_string(Arena* arena, Compilation_Context* context)
{
    const MIR* mir = &context->mir;

    String_Builder builder = {0};
    create_string_builder(&builder, arena);

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        const MIR_Function* mir_function = &mir->functions[function_index];
        const Ast_Function_Definition* ast_definition = mir_function->ast_function_definition;

        append_string(&builder, ast_definition->name.token.lexeme);
        append_string(&builder, ":\n");

        Index current_instruction_index = 1;

        local_stack(MIR_Block*, blocks_to_visit);
        local_array(const MIR_Block*, visited_blocks); // TODO(vlad): Speed this up.

        stack_push(context->scratch_arena, blocks_to_visit, MIR_Block*, mir_function->entry_block);

        while (blocks_to_visit_count > 0)
        {
            const MIR_Block* block = *stack_top(blocks_to_visit);
            stack_pop(blocks_to_visit);

            append_array(context->scratch_arena, visited_blocks, const MIR_Block*, block);

            append_string(&builder, format_string(context->scratch_arena,
                                                  "       |\n"
                                                  "       | LABEL_{}:\n",
                                                  block->sequence_number));

            for (MIR_Instruction* instruction = block->first_instruction;
                 instruction != NULL;
                 instruction = instruction->next_instruction)
            {
                if (instruction->opcode == MIR_NOP)
                {
                    // NOTE(vlad): Sanity checks.
                    ASSERT(instruction->definitions_count == 0);
                    ASSERT(instruction->uses_count == 0);

                    continue;
                }

                append_string(&builder, format_string(context->scratch_arena,
                                                      "{left-pad-count: 6} | ",
                                                      current_instruction_index));
                current_instruction_index += 1;

                switch (instruction->opcode)
                {
                    case MIR_NOP:
                    {
                        UNREACHABLE();
                    } break;

                    case MIR_ADD:
                    {
                        append_string(&builder, "          ADD                ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_SUBTRACT:
                    {
                        append_string(&builder, "          SUBTRACT           ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_MULTIPLY:
                    {
                        append_string(&builder, "          MULTIPLY           ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_DIVIDE:
                    {
                        append_string(&builder, "          DIVIDE             ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_EQUAL:
                    {
                        append_string(&builder, "          EQUAL              ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_NOT_EQUAL:
                    {
                        append_string(&builder, "          NOT_EQUAL          ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_LESS:
                    {
                        append_string(&builder, "          LESS               ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_LESS_OR_EQUAL:
                    {
                        append_string(&builder, "          LESS_OR_EQUAL      ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_GREATER:
                    {
                        append_string(&builder, "          GREATER            ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_GREATER_OR_EQUAL:
                    {
                        append_string(&builder, "          GREATER_OR_EQUAL   ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 2);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[1]);
                    } break;

                    case MIR_LOAD32:
                    {
                        FAIL("[MIR] LOAD32 is not supported yet.");
                    } break;

                    case MIR_STORE32:
                    {
                        FAIL("[MIR] STORE32 is not supported yet.");
                    } break;

                    case MIR_LOAD64:
                    {
                        FAIL("[MIR] LOAD64 is not supported yet.");
                    } break;

                    case MIR_STORE64:
                    {
                        FAIL("[MIR] STORE64 is not supported yet.");
                    } break;

                    case MIR_GET_ADDRESS:
                    {
                        FAIL("[MIR] GET_ADDRESS is not supported yet.");
                    } break;

                    case MIR_MOVE:
                    {
                        append_string(&builder, "          MOVE               ");

                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 1);

                        convert_mir_operand_to_string(context, &builder, &instruction->definitions[0]);
                        append_string(&builder, ",");
                        convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                    } break;

                    case MIR_JUMP:
                    {
                        append_string(&builder, "          JUMP               ");

                        ASSERT(instruction->definitions_count == 0);
                        ASSERT(instruction->uses_count == 1);

                        const MIR_Operand* destination = &instruction->uses[0];
                        ASSERT(destination->kind == MIR_OPERAND_BLOCK);
                        append_string(&builder, " ");
                        convert_mir_operand_to_string(context, &builder, destination);
                    } break;

                    case MIR_JUMP_IF_TRUE:
                    {
                        append_string(&builder, "          JUMP_IF_TRUE       ");

                        ASSERT(instruction->definitions_count == 0);
                        ASSERT(instruction->uses_count == 2);

                        const MIR_Operand* condition = &instruction->uses[0];
                        convert_mir_operand_to_string(context, &builder, condition);

                        append_string(&builder, ", ");

                        const MIR_Operand* destination = &instruction->uses[1];
                        ASSERT(destination->kind == MIR_OPERAND_BLOCK);
                        convert_mir_operand_to_string(context, &builder, destination);
                    } break;

                    case MIR_JUMP_IF_FALSE:
                    {
                        append_string(&builder, "          JUMP_IF_FALSE      ");

                        ASSERT(instruction->definitions_count == 0);
                        ASSERT(instruction->uses_count == 2);

                        const MIR_Operand* condition = &instruction->uses[0];
                        convert_mir_operand_to_string(context, &builder, condition);

                        append_string(&builder, ", ");

                        const MIR_Operand* destination = &instruction->uses[1];
                        ASSERT(destination->kind == MIR_OPERAND_BLOCK);
                        convert_mir_operand_to_string(context, &builder, destination);
                    } break;

                    case MIR_PHI:
                    {
                        ASSERT(instruction->phi_arguments_count != 0);

                        append_string(&builder, "          PHI                ");

                        const MIR_Operand* definition = &instruction->phi_definition;
                        ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);
                        convert_mir_operand_to_string(context, &builder, definition);

                        for (Index argument_index = 0;
                             argument_index < instruction->phi_arguments_count;
                             ++argument_index)
                        {
                            append_string(&builder, ",");
                            MIR_PHI_Argument* argument = &instruction->phi_arguments[argument_index];

                            MIR_Operand argument_operand = {0};
                            argument_operand.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                            argument_operand.virtual_register = argument->virtual_register;

                            convert_mir_operand_to_string(context, &builder, &argument_operand);
                        }
                    } break;

                    case MIR_CALL:
                    {
                        append_string(&builder, "          CALL               ");

                        if (instruction->has_return_value)
                        {
                            convert_mir_operand_to_string(context, &builder, &instruction->return_operand);
                            append_string(&builder, ", ");
                        }
                        else
                        {
                            append_string(&builder, " ");
                        }

                        const Tac_Function* called_function = get_tac_function_by_label(&context->tac,
                                                                                        instruction->function_label_id);
                        append_string(&builder, called_function->ast_function_definition->name.token.lexeme);

                        for (Index argument_index = 0;
                             argument_index < instruction->function_arguments_count;
                             ++argument_index)
                        {
                            MIR_Operand* argument = &instruction->function_arguments[argument_index];
                            append_string(&builder, ",");
                            convert_mir_operand_to_string(context, &builder, argument);
                        }
                    } break;

                    case MIR_RETURN:
                    {
                        append_string(&builder, "          RETURN             ");

                        ASSERT(instruction->definitions_count == 0);

                        if (instruction->uses_count == 1)
                        {
                            convert_mir_operand_to_string(context, &builder, &instruction->uses[0]);
                        }
                        else
                        {
                            ASSERT(instruction->uses_count == 0);
                        }
                    } break;
                }

                append_string(&builder, "\n");
            }

            for (Index successor_index = 0;
                 successor_index < block->successors_count;
                 ++successor_index)
            {
                MIR_Block* successor = block->successors[successor_index];

                Bool successor_was_visited = false;
                for (Index visited_block_index = 0;
                     visited_block_index < visited_blocks_count;
                     ++visited_block_index)
                {
                    const MIR_Block* visited_block = visited_blocks[visited_block_index];
                    if (visited_block == successor)
                    {
                        successor_was_visited = true;
                        break;
                    }
                }

                if (!successor_was_visited)
                {
                    stack_push(context->scratch_arena, blocks_to_visit, MIR_Block*, successor);
                }
            }
        }

        if (function_index != mir->functions_count - 1)
        {
            append_string(&builder, "\n");
        }
    }

    return string_builder_to_string(&builder);
}
