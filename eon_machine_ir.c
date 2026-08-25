#include "eon_machine_ir.h"

#include "eon_compilation_context.h"
#include "eon_cfg.h"

enum
{
    x64_RAX = 1,
    x64_RBX = 2,
    x64_RCX = 3,
    x64_RDX = 4,
    x64_RSP = 5,
    x64_RBP = 6,
    x64_RSI = 7,
    x64_RDI = 8,
    // TODO(vlad): Fill this in.
    // TODO(vlad): Add aarch64.
};

internal inline MIR_Block*
create_new_mir_block(Compilation_Context* context)
{
    return allocate(context->mir_blocks_arena, MIR_Block);
}

internal inline Virtual_Register*
get_virtual_register_for_ssa_variable(Compilation_Context* context, const Tac_Variable_Id id)
{
    Tac* tac = &context->tac;
    MIR* mir = &context->mir;

    ASSERT(INVALID_TAC_INDEX < id.index && id.index < mir->virtual_registers_count);

    Tac_Variable* ssa_variable = get_tac_variable_by_id(tac, id);

    ASSERT(id.ssa_version != SSA_VERSION_UNDEFINED);
    ASSERT(id.ssa_version != SSA_VERSION_UNSET);
    ASSERT(id.ssa_version <= ssa_variable->max_ssa_version);

    return &mir->virtual_registers[ssa_variable->mir_virtual_register_offset + id.ssa_version];
}

internal inline MIR_Operand*
add_new_use_operand(MIR_Instruction* instruction)
{
    ASSERT(0 <= instruction->uses_count && instruction->uses_count < MAX_USES_IN_MIR_INSTRUCTION);
    return &instruction->uses[instruction->uses_count++];
}

internal inline MIR_Operand*
add_new_def_operand(MIR_Instruction* instruction)
{
    ASSERT(0 <= instruction->definitions_count && instruction->definitions_count < MAX_DEFINITIONS_IN_MIR_INSTRUCTION);
    return &instruction->definitions[instruction->definitions_count++];
}

internal void
make_immediate_value_for_constant(Compilation_Context* context,
                                  MIR_Operand* operand,
                                  const Tac_Constant_Id constant_id)
{
    operand->kind = MIR_OPERAND_IMMEDIATE_VALUE;

    const Tac_Constant* constant = get_tac_constant_by_id(&context->tac, constant_id);

    switch (constant->kind)
    {
        case TAC_CONSTANT_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case TAC_CONSTANT_BOOLEAN:
        {
            operand->immediate_value = constant->boolean_value ? 1 : 0;
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
            operand->immediate_value = constant->integer_value;
        } break;

        case TAC_CONSTANT_FLOAT32:
        case TAC_CONSTANT_FLOAT64:
        {
            FAIL("[MIR] Floating-point immediate values are not supported yet.");
        } break;
    }
}

internal MIR_Instruction*
add_new_instruction_to_mir_block(Compilation_Context* context, MIR_Block* block)
{
    MIR_Instruction* instruction = allocate(context->mir_instructions_arena, MIR_Instruction);

    if (block->first_instruction == NULL)
    {
        block->first_instruction = instruction;
        block->last_instruction = instruction;
    }
    else
    {
        ASSERT(block->last_instruction != NULL);

        instruction->previous_instruction = block->last_instruction;
        block->last_instruction->next_instruction = instruction;
        block->last_instruction = instruction;
    }

    return instruction;
}

internal MIR_Block*
lower_ssa_block_to_mir(Compilation_Context* context,
                       Tac_Function* ssa_function,
                       const Cfg_Block_Id this_ssa_block_id,
                       MIR_Block** mir_blocks_map)
{
    if (mir_blocks_map[this_ssa_block_id.index])
    {
        return mir_blocks_map[this_ssa_block_id.index];
    }

    Cfg_Block* this_ssa_block = get_cfg_block_by_id(ssa_function, this_ssa_block_id);
    ASSERT(this_ssa_block != NULL);

    MIR_Block* this_block = create_new_mir_block(context);
    this_block->successors = allocate_array(context->mir_edges_arena, this_ssa_block->edges_count, MIR_Block*);
    this_block->successors_count = this_ssa_block->edges_count;

    this_block->predecessors = allocate_array(context->mir_edges_arena, this_ssa_block->predecessors_count, MIR_Block*);
    this_block->predecessors_count = this_ssa_block->predecessors_count;

    mir_blocks_map[this_ssa_block_id.index] = this_block;

    stack(MIR_Operand, parameters_stack);
    parameters_stack = NULL;
    parameters_stack_count = 0;
    parameters_stack_capacity = 0;

    for (Index phi_node_index = 0;
         phi_node_index < this_ssa_block->phi_nodes_count;
         ++phi_node_index)
    {
        const Phi_Node* phi_node = &this_ssa_block->phi_nodes[phi_node_index];

        MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);

        instruction->opcode = MIR_PHI;
        instruction->phi_arguments_count = phi_node->previous_variables_count;
        instruction->phi_arguments = allocate_array(context->mir_operands_arena,
                                                    instruction->phi_arguments_count,
                                                    MIR_PHI_Argument);

        for (Index phi_argument_index = 0;
             phi_argument_index < instruction->phi_arguments_count;
             ++phi_argument_index)
        {
            MIR_PHI_Argument* argument = &instruction->phi_arguments[phi_argument_index];
            argument->virtual_register = get_virtual_register_for_ssa_variable(context, phi_node->previous_variables[phi_argument_index]);

            const Cfg_Block_Id predecessor_block_id = this_ssa_block->predecessors[phi_argument_index];
            argument->source_block = lower_ssa_block_to_mir(context, ssa_function, predecessor_block_id, mir_blocks_map);
        }
    }

    for (Index instruction_index = this_ssa_block->instructions_range.start_instruction_index;
         instruction_index < this_ssa_block->instructions_range.end_instruction_index;
         ++instruction_index)
    {
        const Tac_Instruction* ssa_instruction = &ssa_function->instructions[instruction_index];

        const Tac_Operand* ssa_destination = &ssa_instruction->destination;
        const Tac_Operand* ssa_first_argument = &ssa_instruction->first_argument;
        const Tac_Operand* ssa_second_argument = &ssa_instruction->second_argument;

        switch (ssa_instruction->operation)
        {
            case TAC_NOP:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_NONE);
                ASSERT(ssa_first_argument->kind == TAC_OPERAND_NONE);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                // TODO(vlad): Should we emit MIR_NOP here?
            } break;

            case TAC_ASSIGN:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_VARIABLE);
                ASSERT(ssa_first_argument->kind != TAC_OPERAND_NONE);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_MOVE;

                MIR_Operand* def = add_new_def_operand(instruction);
                def->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                def->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_destination->variable_id);

                MIR_Operand* use = add_new_use_operand(instruction);

                switch (ssa_first_argument->kind)
                {
                    case TAC_OPERAND_VARIABLE:
                    {
                        use->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                        use->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_first_argument->variable_id);
                    } break;

                    case TAC_OPERAND_CONSTANT:
                    {
                        make_immediate_value_for_constant(context, use, ssa_first_argument->constant_id);
                    } break;

                    case TAC_OPERAND_NONE:
                    case TAC_OPERAND_FUNCTION_LABEL:
                    case TAC_OPERAND_LABEL:
                    case TAC_OPERAND_PARAMETER_INDEX:
                    case TAC_OPERAND_NUMBER_OF_ARGUMENTS:
                    {
                        FAIL("[MIR] Unexpected ASSIGN operand kind encountered.");
                    } break;
                }
            } break;

            case TAC_GET_ADDRESS:
            {
                FAIL("[MIR] GET_ADDRESS is not supported yet");
            } break;

            case TAC_LOAD_BY_ADDRESS:
            {
                FAIL("[MIR] LOAD_BY_ADDRESS is not supported yet");
            } break;

            case TAC_STORE_BY_ADDRESS:
            {
                FAIL("[MIR] STORE_BY_ADDRESS is not supported yet");
            } break;

            case TAC_ADD:
            case TAC_SUBTRACT:
            case TAC_MULTIPLY:
            case TAC_DIVIDE:
            case TAC_EQUAL:
            case TAC_NOT_EQUAL:
            case TAC_LESS:
            case TAC_LESS_OR_EQUAL:
            case TAC_GREATER:
            case TAC_GREATER_OR_EQUAL:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_VARIABLE);
                ASSERT(ssa_first_argument->kind != TAC_OPERAND_NONE);
                ASSERT(ssa_second_argument->kind != TAC_OPERAND_NONE);

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                switch (ssa_instruction->operation)
                {
                    case TAC_ADD:
                    {
                        instruction->opcode = MIR_ADD;
                    } break;

                    case TAC_SUBTRACT:
                    {
                        instruction->opcode = MIR_SUBTRACT;
                    } break;

                    case TAC_MULTIPLY:
                    {
                        instruction->opcode = MIR_MULTIPLY;
                    } break;

                    case TAC_DIVIDE:
                    {
                        instruction->opcode = MIR_DIVIDE;
                    } break;

                    case TAC_EQUAL:
                    {
                        instruction->opcode = MIR_EQUAL;
                    } break;

                    case TAC_NOT_EQUAL:
                    {
                        instruction->opcode = MIR_NOT_EQUAL;
                    } break;

                    case TAC_LESS:
                    {
                        instruction->opcode = MIR_LESS;
                    } break;

                    case TAC_LESS_OR_EQUAL:
                    {
                        instruction->opcode = MIR_LESS_OR_EQUAL;
                    } break;

                    case TAC_GREATER:
                    {
                        instruction->opcode = MIR_GREATER;
                    } break;

                    case TAC_GREATER_OR_EQUAL:
                    {
                        instruction->opcode = MIR_GREATER_OR_EQUAL;
                    } break;

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }

                MIR_Operand* def = add_new_def_operand(instruction);
                def->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                def->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_destination->variable_id);

                {
                    MIR_Operand* first_use = add_new_use_operand(instruction);
                    switch (ssa_first_argument->kind)
                    {
                        case TAC_OPERAND_VARIABLE:
                        {
                            first_use->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                            first_use->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_first_argument->variable_id);
                        } break;

                        case TAC_OPERAND_CONSTANT:
                        {
                            make_immediate_value_for_constant(context, first_use, ssa_first_argument->constant_id);
                        } break;

                        case TAC_OPERAND_NONE:
                        case TAC_OPERAND_FUNCTION_LABEL:
                        case TAC_OPERAND_LABEL:
                        case TAC_OPERAND_PARAMETER_INDEX:
                        case TAC_OPERAND_NUMBER_OF_ARGUMENTS:
                        {
                            FAIL("[MIR] Unexpected binary operand kind encountered.");
                        } break;
                    }
                }

                {
                    MIR_Operand* second_use = add_new_use_operand(instruction);
                    switch (ssa_second_argument->kind)
                    {
                        case TAC_OPERAND_VARIABLE:
                        {
                            second_use->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                            second_use->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_second_argument->variable_id);
                        } break;

                        case TAC_OPERAND_CONSTANT:
                        {
                            make_immediate_value_for_constant(context, second_use, ssa_second_argument->constant_id);
                        } break;

                        case TAC_OPERAND_NONE:
                        case TAC_OPERAND_FUNCTION_LABEL:
                        case TAC_OPERAND_LABEL:
                        case TAC_OPERAND_PARAMETER_INDEX:
                        case TAC_OPERAND_NUMBER_OF_ARGUMENTS:
                        {
                            FAIL("[MIR] Unexpected binary operand kind encountered.");
                        } break;
                    }
                }
            } break;

            case TAC_LABEL:
            {
                // NOTE(vlad): We already map labels to Cfg_Blocks, so we don't need to lower labels to MIR.
            } break;

            case TAC_JUMP:
            {
                FAIL("[MIR] JUMP is not supported yet");
            } break;

            case TAC_JUMP_IF_TRUE:
            {
                FAIL("[MIR] JUMP_IF_TRUE is not supported yet");
            } break;

            case TAC_JUMP_IF_FALSE:
            {
                FAIL("[MIR] JUMP_IF_FALSE is not supported yet");
            } break;

            case TAC_SET_PARAMETER:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_NONE);
                ASSERT(ssa_first_argument->kind != TAC_OPERAND_NONE);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                MIR_Operand parameter = {0};

                switch (ssa_first_argument->kind)
                {
                    case TAC_OPERAND_VARIABLE:
                    {
                        parameter.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                        parameter.virtual_register = get_virtual_register_for_ssa_variable(context, ssa_first_argument->variable_id);
                    } break;

                    case TAC_OPERAND_CONSTANT:
                    {
                        make_immediate_value_for_constant(context, &parameter, ssa_first_argument->constant_id);
                    } break;

                    case TAC_OPERAND_NONE:
                    case TAC_OPERAND_FUNCTION_LABEL:
                    case TAC_OPERAND_LABEL:
                    case TAC_OPERAND_PARAMETER_INDEX:
                    case TAC_OPERAND_NUMBER_OF_ARGUMENTS:
                    {
                        FAIL("[MIR] Unexpected ASSIGN operand kind encountered.");
                    } break;
                }

                stack_push(context->scratch_arena, parameters_stack, MIR_Operand, parameter);
            } break;

            case TAC_GET_PARAMETER:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_VARIABLE);
                ASSERT(ssa_first_argument->kind == TAC_OPERAND_PARAMETER_INDEX);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_MOVE;

                MIR_Operand* def = add_new_def_operand(instruction);
                def->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                def->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_destination->variable_id);

                ASSERT(ssa_first_argument->parameter_index.index >= 0);

                MIR_Operand* use = add_new_use_operand(instruction);
                use->kind = MIR_OPERAND_IMMEDIATE_VALUE;
                use->immediate_value = (u64)(ssa_first_argument->parameter_index.index);
            } break;

            case TAC_CALL:
            {
                ASSERT(ssa_first_argument->kind == TAC_OPERAND_FUNCTION_LABEL);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NUMBER_OF_ARGUMENTS);

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_CALL;

                if (ssa_destination->kind != TAC_OPERAND_NONE)
                {
                    ASSERT(ssa_destination->kind == TAC_OPERAND_VARIABLE);

                    MIR_Operand* def = add_new_def_operand(instruction);
                    def->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                    def->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_destination->variable_id);
                }

                {
                    MIR_Operand* function_use = add_new_use_operand(instruction);
                    function_use->kind = MIR_OPERAND_FUNCTION;
                    function_use->function_label_id = ssa_first_argument->function_label_id;
                }

                const Size number_of_arguments = ssa_second_argument->number_of_arguments;
                if (number_of_arguments > parameters_stack_count)
                {
                    FAIL("[MIR] Cannot lower CALL instruction: insufficient number of arguments provided");
                }

                instruction->function_arguments_count = number_of_arguments;
                instruction->function_arguments = allocate_array(context->mir_operands_arena,
                                                                 number_of_arguments,
                                                                 MIR_Operand);
                for (Index argument_index = 0;
                     argument_index < number_of_arguments;
                     ++argument_index)
                {
                    const MIR_Operand argument = *stack_top(parameters_stack);
                    stack_pop(parameters_stack);

                    instruction->function_arguments[argument_index] = argument;
                }
            } break;

            case TAC_RETURN:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_NONE);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_RETURN;

                switch (ssa_first_argument->kind)
                {
                    case TAC_OPERAND_NONE:
                    {
                        // NOTE(vlad): OK, no return value here.
                    } break;

                    case TAC_OPERAND_VARIABLE:
                    {
                        MIR_Operand* use = add_new_use_operand(instruction);
                        use->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                        use->virtual_register = get_virtual_register_for_ssa_variable(context, ssa_first_argument->variable_id);
                    } break;

                    case TAC_OPERAND_CONSTANT:
                    {
                        MIR_Operand* use = add_new_use_operand(instruction);
                        make_immediate_value_for_constant(context, use, ssa_first_argument->constant_id);
                    } break;

                    case TAC_OPERAND_FUNCTION_LABEL:
                    case TAC_OPERAND_LABEL:
                    case TAC_OPERAND_PARAMETER_INDEX:
                    case TAC_OPERAND_NUMBER_OF_ARGUMENTS:
                    {
                        FAIL("[MIR] Unexpected ASSIGN operand kind encountered.");
                    } break;
                }
            } break;
        }
    }

    for (Index successor_index = 0;
         successor_index < this_block->successors_count;
         ++successor_index)
    {
        const Cfg_Block_Id successor_id = this_ssa_block->edges[successor_index];
        const Cfg_Block* ssa_successor = get_cfg_block_by_id(ssa_function, successor_id);

        MIR_Block* successor = lower_ssa_block_to_mir(context, ssa_function, successor_id, mir_blocks_map);
        this_block->successors[successor_index] = successor;

        Bool predecessor_found = false;
        for (Index predecessor_index = 0;
             predecessor_index < ssa_successor->predecessors_count;
             ++predecessor_index)
        {
            const Cfg_Block_Id predecessor_id = ssa_successor->predecessors[predecessor_index];

            if (predecessor_id.index == this_ssa_block_id.index)
            {
                successor->predecessors[predecessor_index] = this_block;

                predecessor_found = true;
                break;
            }
        }

        ASSERT(predecessor_found);
    }

    return this_block;
}

internal void
lower_ssa_to_mir(Compilation_Context* context)
{
    MIR* mir = &context->mir;
    const Tac* tac = &context->tac;

    mir->functions = allocate_array(context->mir_functions_arena, tac->functions_count, MIR_Function);
    mir->functions_count = tac->functions_count;

    // TODO(vlad): Reserve space for virtual registers.

    for (Index variable_index = INVALID_TAC_INDEX + 1;
         variable_index < tac->variables_count;
         ++variable_index)
    {
        Tac_Variable* ssa_variable = &tac->variables[variable_index];
        ssa_variable->mir_virtual_register_offset = mir->virtual_registers_count;

        ASSERT(ssa_variable->max_ssa_version != 0);

        for (Index version = 0;
             version < ssa_variable->max_ssa_version;
             ++version)
        {
            append_array(context->mir_virtual_registers_arena, mir->virtual_registers, Virtual_Register, (Virtual_Register){0});
        }
    }

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];
        MIR_Function* mir_function = &mir->functions[function_index];

        const Cfg_Block_Id entry_block_id = {0};
        MIR_Block** mir_blocks_map = allocate_array(context->scratch_arena, tac_function->cfg_blocks_count, MIR_Block*);
        mir_function->first_block = lower_ssa_block_to_mir(context, tac_function, entry_block_id, mir_blocks_map);
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}
