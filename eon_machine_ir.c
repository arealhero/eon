#include "eon_machine_ir.h"

#include "eon_compilation_context.h"
#include "eon_cfg.h"

enum
{
    // NOTE(vlad): We treat rsp and rbp as an implicit hardware state so the SSA form is not violated.
    X86_64_RAX = 1,
    X86_64_RBX = 2,
    X86_64_RCX = 3,
    X86_64_RDX = 4,
    X86_64_RSI = 5,
    X86_64_RDI = 6,
    X86_64_R8  = 7,
    X86_64_R9  = 8,
    X86_64_R10 = 9,
    X86_64_R11 = 10,
    X86_64_R12 = 11,
    X86_64_R13 = 12,
    X86_64_R14 = 13,
    X86_64_R15 = 14,

    // TODO(vlad): Add X86_64_RBP for optimized builds because we can omit frame pointers in that case and use RBP as a
    //             15th GPR. We should also consider supporting the '-fno-omit-frame-pointer' compilation option.

    // TODO(vlad): Add 32, 16, and 8-bit registers.
    // TODO(vlad): Add aarch64.
};

internal inline MIR_Block*
create_new_mir_block(Compilation_Context* context)
{
    MIR_Block* block = allocate(context->mir_blocks_arena, MIR_Block);
    block->sequence_number = ++context->next_mir_block_sequence_number;
    return block;
}

internal inline Index
create_virtual_register(Compilation_Context* context)
{
    MIR* mir = &context->mir;
    append_array(context->mir_virtual_registers_arena,
                 mir->virtual_registers,
                 Virtual_Register,
                 (Virtual_Register){0});
    return mir->virtual_registers_count - 1;
}

internal inline Index
get_virtual_register_index_for_ssa_variable(Compilation_Context* context, const Tac_Variable_Id id)
{
    Tac* tac = &context->tac;
    MIR* mir = &context->mir;

    ASSERT(INVALID_TAC_INDEX < id.index && id.index < mir->virtual_registers_count);

    Tac_Variable* ssa_variable = get_tac_variable_by_id(tac, id);

    ASSERT(id.ssa_version != SSA_VERSION_UNDEFINED);
    ASSERT(id.ssa_version != SSA_VERSION_UNSET);
    ASSERT(id.ssa_version <= ssa_variable->max_ssa_version);

    return ssa_variable->mir_virtual_register_offset + id.ssa_version;
}

// FIXME(vlad): Accept MIR_Operand* instead of returning one and rename this function.
internal inline MIR_Operand*
add_new_def_operand(MIR_Instruction* instruction)
{
    ASSERT(0 <= instruction->definitions_count
           && instruction->definitions_count < NUMBER_OF_STATIC_ARRAY_ELEMENTS(instruction->definitions));
    return &instruction->definitions[instruction->definitions_count++];
}

internal inline MIR_Operand*
add_new_use_operand(MIR_Instruction* instruction)
{
    ASSERT(0 <= instruction->uses_count
           && instruction->uses_count < NUMBER_OF_STATIC_ARRAY_ELEMENTS(instruction->uses));
    return &instruction->uses[instruction->uses_count++];
}

internal inline MIR_Operand*
add_new_implicit_def_operand(MIR_Instruction* instruction)
{
    ASSERT(0 <= instruction->implicit_definitions_count
           && instruction->implicit_definitions_count < NUMBER_OF_STATIC_ARRAY_ELEMENTS(instruction->implicit_definitions));
    return &instruction->implicit_definitions[instruction->implicit_definitions_count++];
}

internal inline MIR_Operand*
add_new_implicit_use_operand(MIR_Instruction* instruction)
{
    ASSERT(0 <= instruction->implicit_uses_count
           && instruction->implicit_uses_count < NUMBER_OF_STATIC_ARRAY_ELEMENTS(instruction->implicit_uses));
    return &instruction->implicit_uses[instruction->implicit_uses_count++];
}

internal inline void
add_coalescing_hint(Compilation_Context* context,
                    MIR_Instruction* instruction,
                    const Index first_virtual_register_index,
                    const Index second_virtual_register_index)
{
    Coalescing_Hint hint = {0};
    hint.first_virtual_register_index = first_virtual_register_index;
    hint.second_virtual_register_index = second_virtual_register_index;

    append_array(context->mir_coalescing_hints_arena, instruction->coalescing_hints, Coalescing_Hint, hint);
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

internal MIR_Instruction*
prepend_instruction(Compilation_Context* context,
                    MIR_Block* block,
                    MIR_Instruction* current_instruction)
{
    MIR_Instruction* instruction = allocate(context->mir_instructions_arena, MIR_Instruction);

    MIR_Instruction* previous_instruction = current_instruction->previous_instruction;

    instruction->previous_instruction = previous_instruction;
    instruction->next_instruction = current_instruction;
    current_instruction->previous_instruction = instruction;

    if (previous_instruction == NULL)
    {
        ASSERT(block->first_instruction == current_instruction);
        block->first_instruction = instruction;
    }
    else
    {
        previous_instruction->next_instruction = instruction;
    }

    return instruction;
}

internal MIR_Instruction*
append_instruction(Compilation_Context* context,
                   MIR_Block* block,
                   MIR_Instruction* current_instruction)
{
    MIR_Instruction* instruction = allocate(context->mir_instructions_arena, MIR_Instruction);

    MIR_Instruction* next_instruction = current_instruction->next_instruction;

    instruction->previous_instruction = current_instruction;
    instruction->next_instruction = next_instruction;
    current_instruction->next_instruction = instruction;

    if (next_instruction == NULL)
    {
        ASSERT(block->last_instruction == current_instruction);
        block->last_instruction = instruction;
    }
    else
    {
        next_instruction->previous_instruction = instruction;
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
        instruction->phi_definition.kind = MIR_OPERAND_VIRTUAL_REGISTER;
        instruction->phi_definition.virtual_register_index
            = get_virtual_register_index_for_ssa_variable(context, phi_node->destination);
        instruction->phi_arguments_count = phi_node->previous_variables_count;
        instruction->phi_arguments = allocate_array(context->mir_operands_arena,
                                                    instruction->phi_arguments_count,
                                                    MIR_PHI_Argument);

        for (Index phi_argument_index = 0;
             phi_argument_index < instruction->phi_arguments_count;
             ++phi_argument_index)
        {
            MIR_PHI_Argument* argument = &instruction->phi_arguments[phi_argument_index];
            argument->virtual_register_index
                = get_virtual_register_index_for_ssa_variable(context, phi_node->previous_variables[phi_argument_index]);

            const Cfg_Block_Id predecessor_block_id = this_ssa_block->predecessors[phi_argument_index];
            argument->source_block = lower_ssa_block_to_mir(context, ssa_function, predecessor_block_id, mir_blocks_map);
        }
    }

    for (const Tac_Instruction* ssa_instruction = this_ssa_block->first_tac_instruction;
         ssa_instruction != NULL;
         ssa_instruction = ssa_instruction->next_instruction)
    {
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
                def->virtual_register_index = get_virtual_register_index_for_ssa_variable(context,
                                                                                          ssa_destination->variable_id);

                MIR_Operand* use = add_new_use_operand(instruction);

                switch (ssa_first_argument->kind)
                {
                    case TAC_OPERAND_VARIABLE:
                    {
                        use->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                        use->virtual_register_index
                            = get_virtual_register_index_for_ssa_variable(context, ssa_first_argument->variable_id);
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
                def->virtual_register_index = get_virtual_register_index_for_ssa_variable(context,
                                                                                          ssa_destination->variable_id);

                {
                    MIR_Operand* first_use = add_new_use_operand(instruction);
                    switch (ssa_first_argument->kind)
                    {
                        case TAC_OPERAND_VARIABLE:
                        {
                            first_use->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                            first_use->virtual_register_index
                                = get_virtual_register_index_for_ssa_variable(context, ssa_first_argument->variable_id);
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
                            second_use->virtual_register_index
                                = get_virtual_register_index_for_ssa_variable(context, ssa_second_argument->variable_id);
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
                ASSERT(ssa_destination->kind == TAC_OPERAND_LABEL);
                ASSERT(ssa_first_argument->kind == TAC_OPERAND_NONE);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                const Tac_Label_Id label_id = ssa_destination->label_id;
                const Cfg_Block_Id destination_block_id = context->tac.label_index_to_cfg_block_id_map[label_id.index];

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_JUMP;

                MIR_Operand* use = add_new_use_operand(instruction);
                use->kind = MIR_OPERAND_BLOCK;
                use->block = lower_ssa_block_to_mir(context, ssa_function, destination_block_id, mir_blocks_map);
            } break;

            case TAC_JUMP_IF_TRUE:
            case TAC_JUMP_IF_FALSE:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_LABEL);
                ASSERT(ssa_first_argument->kind == TAC_OPERAND_VARIABLE);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                const Tac_Label_Id label_id = ssa_destination->label_id;
                const Cfg_Block_Id destination_block_id = context->tac.label_index_to_cfg_block_id_map[label_id.index];

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                if (ssa_instruction->operation == TAC_JUMP_IF_TRUE)
                {
                    instruction->opcode = MIR_JUMP_IF_TRUE;
                }
                else if (ssa_instruction->operation == TAC_JUMP_IF_FALSE)
                {
                    instruction->opcode = MIR_JUMP_IF_FALSE;
                }
                else
                {
                    UNREACHABLE();
                }

                MIR_Operand* condition_operand = add_new_use_operand(instruction);
                condition_operand->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                condition_operand->virtual_register_index
                    = get_virtual_register_index_for_ssa_variable(context, ssa_first_argument->variable_id);

                MIR_Operand* destination_operand = add_new_use_operand(instruction);
                destination_operand->kind = MIR_OPERAND_BLOCK;
                destination_operand->block = lower_ssa_block_to_mir(context, ssa_function, destination_block_id, mir_blocks_map);
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
                        parameter.virtual_register_index
                            = get_virtual_register_index_for_ssa_variable(context, ssa_first_argument->variable_id);
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
                instruction->opcode = MIR_GET_PARAMETER;

                MIR_Operand* def = add_new_def_operand(instruction);
                def->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                def->virtual_register_index
                    = get_virtual_register_index_for_ssa_variable(context, ssa_destination->variable_id);

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

                    instruction->has_return_value = true;
                    instruction->return_operand.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                    instruction->return_operand.virtual_register_index
                        = get_virtual_register_index_for_ssa_variable(context, ssa_destination->variable_id);
                }

                instruction->function_label_id = ssa_first_argument->function_label_id;

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
                        use->virtual_register_index
                            = get_virtual_register_index_for_ssa_variable(context, ssa_first_argument->variable_id);
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
             version <= ssa_variable->max_ssa_version;
             ++version)
        {
            create_virtual_register(context);
        }
    }

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];
        MIR_Function* mir_function = &mir->functions[function_index];

        mir_function->ast_function_definition = tac_function->ast_function_definition;

        const Cfg_Block_Id entry_block_id = {0};
        MIR_Block** mir_blocks_map = allocate_array(context->scratch_arena, tac_function->cfg_blocks_count, MIR_Block*);
        mir_function->entry_block = lower_ssa_block_to_mir(context, tac_function, entry_block_id, mir_blocks_map);
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}

internal inline Calling_Convention
get_calling_convention_for_the_function(Compilation_Context* context, MIR_Function* function)
{
    // TODO(vlad): Support calling convention attribute for functions (especially for foreign ones).
    UNUSED(function);
    return context->calling_convention;
}

internal void
add_isa_constraints_to_mir_instruction(Compilation_Context* context,
                                       MIR_Function* function,
                                       MIR_Block* block,
                                       MIR_Instruction* instruction)
{
    switch (context->target_architecture)
    {
        case TARGET_ARCH_X86_64:
        {
            switch (instruction->opcode)
            {
                case MIR_UNDEFINED:
                {
                    UNREACHABLE();
                } break;

                case MIR_NOP:
                {
                    // NOTE(vlad): 'nop' instruction has no constraints.
                } break;

                case MIR_ADD:
                case MIR_SHRINK_STACK:
                case MIR_SUBTRACT:
                case MIR_GROW_STACK:
                {
                    // NOTE(vlad): These instructions clobber EFLAGS, but since we rematerialize them before
                    //             conditional jumps we don't actually need to track them.
                    //
                    //             But note that this will not be true if we would implement the peephole optimization
                    //             pass, so be careful.
                } break;

                case MIR_MULTIPLY:
                {
                    ASSERT(instruction->definitions_count == 1);
                    ASSERT(instruction->uses_count == 2);

                    {
                        MIR_Operand* second_argument = &instruction->uses[1];
                        if (second_argument->kind == MIR_OPERAND_IMMEDIATE_VALUE)
                        {
                            // NOTE(vlad): 'imul' operation does not support immediate values so we need to add explicit
                            //             MOVE instruction here.

                            MIR_Instruction* second_move_instruction = prepend_instruction(context, block, instruction);
                            second_move_instruction->opcode = MIR_MOVE;

                            const Index temp_register_index = create_virtual_register(context);

                            MIR_Operand* temp_operand = add_new_def_operand(second_move_instruction);
                            temp_operand->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                            temp_operand->virtual_register_index = temp_register_index;

                            MIR_Operand* immediate_value_operand = add_new_use_operand(second_move_instruction);
                            *immediate_value_operand = *second_argument;

                            add_isa_constraints_to_mir_instruction(context, function, block, second_move_instruction);

                            *second_argument = *temp_operand;
                        }
                    }

                    // NOTE(vlad): Note that 'imul' operation is semantically equivalent to '*=' operator in C
                    //             so we need to add a tie constraint. Register allocator will use this information
                    //             to color destination and first_argument to the same color (coalesce them).
                    //             If it can't, it will emit a MOVE.
                    {
                        MIR_Operand* destination = &instruction->definitions[0];
                        MIR_Operand* first_argument = &instruction->uses[0];

                        ASSERT(destination->kind == MIR_OPERAND_VIRTUAL_REGISTER);
                        ASSERT(first_argument->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                        add_coalescing_hint(context,
                                            instruction,
                                            destination->virtual_register_index,
                                            first_argument->virtual_register_index);
                    }
                } break;

                case MIR_DIVIDE:
                {
                    // NOTE(vlad): 'imul' instruction expects the dividend, quotient and remainder to be
                    //             in specific registers, so we create temporary virtual registers and force them
                    //             to be in these specific registers.

                    // TODO(vlad): Support numbers with bit widths other than 64.

                    ASSERT(instruction->definitions_count == 1);
                    ASSERT(instruction->uses_count == 2);

                    MIR_Operand quotient = instruction->definitions[0];
                    MIR_Operand dividend = instruction->uses[0];
                    MIR_Operand divisor = instruction->uses[1];

                    MIR_Operand rax_operand = {0};
                    MIR_Operand rdx_operand = {0};

                    {
                        const Index rax_register_index = create_virtual_register(context);

                        Virtual_Register* rax_register = &context->mir.virtual_registers[rax_register_index];
                        rax_register->kind = REGISTER_GPR64;
                        rax_register->fixed_physical_register = X86_64_RAX;

                        rax_operand.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                        rax_operand.virtual_register_index = rax_register_index;
                    }

                    {
                        const Index rdx_register_index = create_virtual_register(context);

                        Virtual_Register* rdx_register = &context->mir.virtual_registers[rdx_register_index];
                        rdx_register->kind = REGISTER_GPR64;
                        rdx_register->fixed_physical_register = X86_64_RDX;

                        rdx_operand.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                        rdx_operand.virtual_register_index = rdx_register_index;
                    }

                    {
                        MIR_Instruction* move_instruction = prepend_instruction(context, block, instruction);
                        move_instruction->opcode = MIR_MOVE;

                        MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                        *move_destination = rax_operand;

                        MIR_Operand* move_argument = add_new_use_operand(move_instruction);
                        *move_argument = dividend;

                        add_isa_constraints_to_mir_instruction(context, function, block, move_instruction);
                    }

                    // NOTE(vlad): We will need to emit 'cqo' instruction before 'imul' to extend the sign of RAX to RDX.

                    instruction->definitions_count = 0;
                    instruction->uses_count = 0;

                    {
                        MIR_Operand* new_divisor = add_new_use_operand(instruction);
                        *new_divisor = divisor;
                    }

                    {
                        MIR_Operand* rax_use = add_new_implicit_use_operand(instruction);
                        *rax_use = rax_operand;

                        MIR_Operand* rdx_use = add_new_implicit_use_operand(instruction);
                        *rdx_use = rdx_operand;
                    }

                    {
                        MIR_Operand* rax_def = add_new_implicit_def_operand(instruction);
                        *rax_def = rax_operand;

                        MIR_Operand* rdx_def = add_new_implicit_def_operand(instruction);
                        *rdx_def = rdx_operand;
                    }

                    // NOTE(vlad): Moving quotient to the destination.
                    {
                        MIR_Instruction* move_instruction = append_instruction(context, block, instruction);
                        move_instruction->opcode = MIR_MOVE;

                        MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                        *move_destination = quotient;

                        MIR_Operand* move_argument = add_new_use_operand(move_instruction);
                        *move_argument = rax_operand;

                        add_isa_constraints_to_mir_instruction(context, function, block, move_instruction);
                    }
                } break;

                case MIR_EQUAL:
                case MIR_NOT_EQUAL:
                case MIR_LESS:
                case MIR_LESS_OR_EQUAL:
                case MIR_GREATER:
                case MIR_GREATER_OR_EQUAL:
                {
                    // NOTE(vlad): These instructions clobber EFLAGS, but since we rematerialize them we don't
                    //             need to track them.
                } break;

                case MIR_LOAD32:
                {
                    FAIL("[MIR] LOAD32 instruction is not supported yet.");
                } break;

                case MIR_STORE32:
                {
                    FAIL("[MIR] STORE32 instruction is not supported yet.");
                } break;

                case MIR_LOAD64:
                {
                    FAIL("[MIR] LOAD64 instruction is not supported yet.");
                } break;

                case MIR_STORE64:
                {
                    FAIL("[MIR] STORE64 instruction is not supported yet.");
                } break;

                case MIR_GET_ADDRESS:
                {
                    FAIL("[MIR] GET_ADDRESS instruction is not supported yet.");
                } break;

                case MIR_GET_PARAMETER:
                {
                    const Calling_Convention calling_convention = get_calling_convention_for_the_function(context,
                                                                                                          function);

                    switch (calling_convention)
                    {
                        case CALLING_CONVENTION_SYSTEM_V:
                        {
                            FAIL("[MIR] This calling convention is not supported yet.");
                        } break;

                        case CALLING_CONVENTION_MICROSOFT_X64:
                        {
                            // TODO(vlad): Remove code duplication (see MIR_CALL).
                            local_persist Index argument_registers[] = {
                                X86_64_RCX,
                                X86_64_RDX,
                                X86_64_R8,
                                X86_64_R9,
                            };

                            ASSERT(instruction->uses_count == 1);

                            MIR_Operand* parameter_index_operand = &instruction->uses[0];
                            ASSERT(parameter_index_operand->kind == MIR_OPERAND_IMMEDIATE_VALUE);

                            const u64 parameter_index = parameter_index_operand->immediate_value;
                            if (parameter_index > NUMBER_OF_STATIC_ARRAY_ELEMENTS(argument_registers))
                            {
                                FAIL("[MIR] Stack arguments are not supported yet");
                            }

                            MIR_Operand abi_parameter = {0};
                            {
                                const Index parameter_register_index = create_virtual_register(context);

                                Virtual_Register* parameter_register = &context->mir.virtual_registers[parameter_register_index];
                                parameter_register->kind = REGISTER_GPR64;
                                parameter_register->fixed_physical_register = argument_registers[parameter_index];

                                abi_parameter.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                abi_parameter.virtual_register_index = parameter_register_index;
                            }

                            instruction->opcode = MIR_MOVE;
                            instruction->uses[0] = abi_parameter;

                            add_isa_constraints_to_mir_instruction(context, function, block, instruction);
                        } break;
                    }
                } break;

                case MIR_MOVE:
                {
                    // NOTE(vlad): 'mov' instruction has no constraints.
                } break;

                case MIR_JUMP:
                {
                    // NOTE(vlad): 'jmp' instruction has no constraints.
                } break;

                case MIR_JUMP_IF_TRUE:
                case MIR_JUMP_IF_FALSE:
                {
                    // NOTE(vlad): We don't keep track of the EFLAGS, thus conditional jumps have no ISA-specific
                    //             constraints here. The condition will be rematerialized during the binary emission.
                } break;

                case MIR_PHI:
                {
                    // NOTE(vlad): PHI instruction is not implemented in CPUs, thus it has no ISA-specific constraints.
                } break;

                case MIR_CALL:
                {
                    const Calling_Convention calling_convention = get_calling_convention_for_the_function(context,
                                                                                                          function);

                    switch (calling_convention)
                    {
                        case CALLING_CONVENTION_SYSTEM_V:
                        {
                            FAIL("[MIR] This calling convention is not supported yet.");
                        } break;

                        case CALLING_CONVENTION_MICROSOFT_X64:
                        {
                            local_persist Index argument_registers[] = {
                                X86_64_RCX,
                                X86_64_RDX,
                                X86_64_R8,
                                X86_64_R9,
                            };

                            local_persist Index caller_saved_registers[] = {
                                X86_64_RAX,
                                X86_64_RCX,
                                X86_64_RDX,
                                X86_64_R8,
                                X86_64_R9,
                                X86_64_R10,
                                X86_64_R11,
                            };

                            if (instruction->function_arguments_count > NUMBER_OF_STATIC_ARRAY_ELEMENTS(argument_registers))
                            {
                                FAIL("[MIR] Stack arguments are not supported yet");
                            }

                            for (Index argument_index = 0;
                                 argument_index < instruction->function_arguments_count;
                                 ++argument_index)
                            {
                                const MIR_Operand* argument = &instruction->function_arguments[argument_index];

                                MIR_Operand abi_argument = {0};
                                {
                                    const Index argument_register_index = create_virtual_register(context);

                                    Virtual_Register* argument_register = &context->mir.virtual_registers[argument_register_index];
                                    argument_register->kind = REGISTER_GPR64;
                                    argument_register->fixed_physical_register = argument_registers[argument_index];

                                    abi_argument.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                    abi_argument.virtual_register_index = argument_register_index;
                                }

                                MIR_Instruction* move_instruction = prepend_instruction(context, block, instruction);
                                move_instruction->opcode = MIR_MOVE;

                                MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                                *move_destination = abi_argument;

                                MIR_Operand* move_argument = add_new_use_operand(move_instruction);
                                *move_argument = *argument;

                                {
                                    MIR_Operand* argument_use = add_new_implicit_use_operand(instruction);
                                    *argument_use = abi_argument;
                                }

                                add_isa_constraints_to_mir_instruction(context, function, block, move_instruction);
                            }

                            instruction->function_arguments_count = 0;

                            // TODO(vlad): Handle stack arguments. We need to emit 'STORE SLOT, operand' here.
                            //             We will need a new MIR_OPERAND_STACK_SLOT kind of operand here.

                            // TODO(vlad): Align stack to 16 bytes.

                            // NOTE(vlad): Allocating shadow space.
                            {
                                MIR_Instruction* grow_instruction = prepend_instruction(context, block, instruction);
                                grow_instruction->opcode = MIR_GROW_STACK;

                                MIR_Operand* amount_operand = add_new_use_operand(grow_instruction);
                                amount_operand->kind = MIR_OPERAND_IMMEDIATE_VALUE;
                                amount_operand->immediate_value = 32;

                                add_isa_constraints_to_mir_instruction(context, function, block, grow_instruction);
                            }

                            // NOTE(vlad): Treat caller-saved registers as clobbered.
                            for (Index register_index = 0;
                                 register_index < NUMBER_OF_STATIC_ARRAY_ELEMENTS(caller_saved_registers);
                                 ++register_index)
                            {
                                MIR_Operand* operand = add_new_implicit_def_operand(instruction);

                                const Index virtual_register_index = create_virtual_register(context);

                                Virtual_Register* virtual_register = &context->mir.virtual_registers[virtual_register_index];
                                virtual_register->kind = REGISTER_GPR64;
                                virtual_register->fixed_physical_register = caller_saved_registers[register_index];

                                operand->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                operand->virtual_register_index = virtual_register_index;
                            }

                            MIR_Instruction* last_instruction = instruction;

                            if (instruction->has_return_value)
                            {
                                // NOTE(vlad): Retrieving returned value.
                                MIR_Instruction* move_instruction = append_instruction(context, block, instruction);

                                const MIR_Operand* return_operand = &instruction->return_operand;

                                MIR_Operand rax_operand = {0};
                                {
                                    const Index rax_register_index = create_virtual_register(context);

                                    Virtual_Register* rax_register = &context->mir.virtual_registers[rax_register_index];
                                    rax_register->kind = REGISTER_GPR64;
                                    rax_register->fixed_physical_register = X86_64_RAX;

                                    rax_operand.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                    rax_operand.virtual_register_index = rax_register_index;
                                }

                                move_instruction->opcode = MIR_MOVE;

                                MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                                *move_destination = *return_operand;

                                MIR_Operand* move_argument = add_new_use_operand(move_instruction);
                                *move_argument = rax_operand;

                                add_isa_constraints_to_mir_instruction(context, function, block, move_instruction);

                                last_instruction = move_instruction;

                                // NOTE(vlad): We already added an implicit def for RAX as a caller-saved register.
                                instruction->has_return_value = false;
                            }

                            // NOTE(vlad): Deallocating shadow space.
                            {
                                MIR_Instruction* shrink_instruction = append_instruction(context, block, last_instruction);
                                shrink_instruction->opcode = MIR_SHRINK_STACK;

                                MIR_Operand* amount_operand = add_new_use_operand(shrink_instruction);
                                amount_operand->kind = MIR_OPERAND_IMMEDIATE_VALUE;
                                amount_operand->immediate_value = 32;

                                add_isa_constraints_to_mir_instruction(context, function, block, shrink_instruction);
                            }
                        } break;
                    }
                } break;

                case MIR_RETURN:
                {
                    ASSERT(instruction->definitions_count == 0);

                    // TODO(vlad): Restore callee-saved registers here. Although we need to do that after register
                    //             allocation because we need to know what registers to save/restore in this function.

                    // STUDY(vlad): Does that mean that we need to prioritize caller-saved registers during allocation?
                    //              I mean, they were already pushed/popped so there is no need to push/pop some
                    //              other registers, right?

                    if (instruction->uses_count != 0)
                    {
                        ASSERT(instruction->uses_count == 1);

                        const Calling_Convention calling_convention = get_calling_convention_for_the_function(context,
                                                                                                              function);

                        switch (calling_convention)
                        {
                            case CALLING_CONVENTION_SYSTEM_V:
                            {
                                FAIL("[MIR] This calling convention is not supported yet.");
                            } break;

                            case CALLING_CONVENTION_MICROSOFT_X64:
                            {
                                MIR_Operand* return_value = &instruction->uses[0];

                                MIR_Operand rax_operand = {0};
                                {
                                    const Index rax_register_index = create_virtual_register(context);

                                    Virtual_Register* rax_register = &context->mir.virtual_registers[rax_register_index];
                                    rax_register->kind = REGISTER_GPR64;
                                    rax_register->fixed_physical_register = X86_64_RAX;

                                    rax_operand.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                    rax_operand.virtual_register_index = rax_register_index;
                                }

                                {
                                    MIR_Instruction* move_instruction = prepend_instruction(context, block, instruction);
                                    move_instruction->opcode = MIR_MOVE;

                                    MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                                    *move_destination = rax_operand;

                                    MIR_Operand* move_argument = add_new_use_operand(move_instruction);
                                    *move_argument = *return_value;

                                    add_isa_constraints_to_mir_instruction(context, function, block, move_instruction);
                                }

                                instruction->uses_count = 0;

                                {
                                    MIR_Operand* rax_use = add_new_implicit_use_operand(instruction);
                                    *rax_use = rax_operand;
                                }
                            } break;
                        }
                    }
                } break;
            }
        } break;

        case TARGET_ARCH_AARCH64:
        {
            FAIL("[MIR] This architecture is not supported yet.");
        } break;
    }
}

internal void
add_isa_constraints_to_mir(Compilation_Context* context)
{
    MIR* mir = &context->mir;

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        MIR_Function* function = &mir->functions[function_index];

        // XXX(vlad): Maybe we can store a boolean flag in each MIR_Block? Something like 'was_visited'.
        //            The downside of this approach is that we would need to traverse the blocks twice instead of once,
        //            so it is probably ok to allocate 'visited_blocks' alongside the stack.
        local_stack(MIR_Block*, blocks_to_visit);
        local_array(const MIR_Block*, visited_blocks); // TODO(vlad): Speed this up.

        stack_push(context->scratch_arena, blocks_to_visit, MIR_Block*, function->entry_block);

        while (blocks_to_visit_count > 0)
        {
            MIR_Block* block = *stack_top(blocks_to_visit);
            stack_pop(blocks_to_visit);

            append_array(context->scratch_arena, visited_blocks, const MIR_Block*, block);

            for (MIR_Instruction* instruction = block->first_instruction;
                 instruction != NULL;
                 instruction = instruction->next_instruction)
            {
                add_isa_constraints_to_mir_instruction(context, function, block, instruction);
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
    }
}

internal String_View physical_register_to_string(Compilation_Context* context,
                                                 const Index physical_register)
{
    switch (context->target_architecture)
    {
        case TARGET_ARCH_X86_64:
        {
            switch (physical_register)
            {
                case NO_REGISTER:
                {
                    FAIL("[MIR] Physical register was not defined");
                } break;

                case X86_64_RAX:
                {
                    return string_view("RAX");
                } break;

                case X86_64_RBX:
                {
                    return string_view("RBX");
                } break;

                case X86_64_RCX:
                {
                    return string_view("RCX");
                } break;

                case X86_64_RDX:
                {
                    return string_view("RDX");
                } break;

                case X86_64_RSI:
                {
                    return string_view("RSI");
                } break;

                case X86_64_RDI:
                {
                    return string_view("RDI");
                } break;

                case X86_64_R8:
                {
                    return string_view("R8");
                } break;

                case X86_64_R9:
                {
                    return string_view("R9");
                } break;

                case X86_64_R10:
                {
                    return string_view("R10");
                } break;

                case X86_64_R11:
                {
                    return string_view("R11");
                } break;

                case X86_64_R12:
                {
                    return string_view("R12");
                } break;

                case X86_64_R13:
                {
                    return string_view("R13");
                } break;

                case X86_64_R14:
                {
                    return string_view("R14");
                } break;

                case X86_64_R15:
                {
                    return string_view("R15");
                } break;

                default:
                {
                    FAIL("[MIR] Unknown physical register provided.");
                } break;
            }
        } break;

        case TARGET_ARCH_AARCH64:
        {
            FAIL("[MIR] This target architecture is not supported yet.");
        } break;
    }
}
