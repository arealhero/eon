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

    X86_64_GPR_COUNT,

    // TODO(vlad): Add X86_64_RBP for optimized builds because we can omit frame pointers in that case and use RBP as a
    //             15th GPR. We should also consider supporting the '-fno-omit-frame-pointer' compilation option.

    // TODO(vlad): Add 32, 16, and 8-bit registers.
    // TODO(vlad): Add aarch64.
};

internal Bool
instruction_is_a_block_terminator(const MIR_Instruction* instruction)
{
    switch (instruction->opcode)
    {
        case MIR_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case MIR_JUMP:
        case MIR_RETURN:
        {
            return true;
        } break;

        case MIR_NOP:
        case MIR_ADD:
        case MIR_SUBTRACT:
        case MIR_MULTIPLY:
        case MIR_DIVIDE:
        case MIR_EQUAL:
        case MIR_NOT_EQUAL:
        case MIR_LESS:
        case MIR_LESS_OR_EQUAL:
        case MIR_GREATER:
        case MIR_GREATER_OR_EQUAL:
        case MIR_LOAD32:
        case MIR_STORE32:
        case MIR_LOAD64:
        case MIR_STORE64:
        case MIR_GET_ADDRESS:
        case MIR_GROW_STACK:
        case MIR_SHRINK_STACK:
        case MIR_GET_PARAMETER:
        case MIR_MOVE:
        case MIR_PHI:
        case MIR_CALL:
        case MIR_JUMP_IF_FALSE:
        {
            return false;
        } break;
    }

    UNREACHABLE();
}

internal String_View
physical_register_to_string(Compilation_Context* context,
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

    UNREACHABLE();
}
internal inline MIR_Block*
create_new_mir_block(Compilation_Context* context)
{
    MIR_Block* block = allocate(context->mir_blocks_arena, MIR_Block);
    block->sequence_number = ++context->next_mir_block_sequence_number;
    return block;
}

internal inline Index
create_virtual_register(MIR_Function* function)
{
    append_array(function->virtual_registers_arena,
                 function->virtual_registers,
                 Virtual_Register,
                 (Virtual_Register){0});
    return function->virtual_registers_count - 1;
}

internal inline Index
create_stack_slot(MIR_Function* function, const Size size_in_bytes)
{
    MIR_Stack_Slot stack_slot = {0};
    stack_slot.offset_in_bytes = function->current_stack_offset_in_bytes;
    stack_slot.size_in_bytes = size_in_bytes;

    function->current_stack_offset_in_bytes += size_in_bytes;

    append_array(function->stack_slots_arena,
                 function->stack_slots,
                 MIR_Stack_Slot,
                 stack_slot);
    return function->stack_slots_count - 1;
}

internal inline Index
get_virtual_register_index_for_ssa_variable(Compilation_Context* context, const Tac_Variable_Id id)
{
    Tac* tac = &context->tac;

    ASSERT(INVALID_TAC_INDEX < id.index && id.index < tac->variables_count);

    Tac_Variable* ssa_variable = get_tac_variable_by_id(tac, id);

    ASSERT(id.ssa_version != SSA_VERSION_UNDEFINED);
    ASSERT(id.ssa_version != SSA_VERSION_UNSET);
    ASSERT(id.ssa_version <= ssa_variable->max_ssa_version);

    return ssa_variable->mir_virtual_register_offset + id.ssa_version - 1;
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
add_coalescing_hint_to_instruction(Compilation_Context* context,
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

    local_stack(MIR_Operand, parameters_stack);

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

        for (Index phi_argument_index = 0;
             phi_argument_index < phi_node->previous_variables_count;
             ++phi_argument_index)
        {
            const Tac_Variable_Id* previous_variable = &phi_node->previous_variables[phi_argument_index];

            if (previous_variable->ssa_version == SSA_VERSION_UNSET)
            {
                continue;
            }

            MIR_PHI_Argument argument = {0};
            argument.virtual_register_index = get_virtual_register_index_for_ssa_variable(context,
                                                                                          phi_node->previous_variables[phi_argument_index]);

            const Cfg_Block_Id predecessor_block_id = this_ssa_block->predecessors[phi_argument_index];
            argument.source_block = lower_ssa_block_to_mir(context, ssa_function, predecessor_block_id, mir_blocks_map);

            append_array(context->mir_operands_arena, instruction->phi_arguments, MIR_PHI_Argument, argument);
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
                const Cfg_Block_Id destination_block_id = ssa_function->label_index_to_cfg_block_id_map[label_id.index];

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_JUMP;

                MIR_Operand* use = add_new_use_operand(instruction);
                use->kind = MIR_OPERAND_BLOCK;
                use->block = lower_ssa_block_to_mir(context, ssa_function, destination_block_id, mir_blocks_map);
            } break;

            case TAC_JUMP_IF_FALSE:
            {
                ASSERT(ssa_destination->kind == TAC_OPERAND_LABEL);
                ASSERT(ssa_first_argument->kind == TAC_OPERAND_VARIABLE);
                ASSERT(ssa_second_argument->kind == TAC_OPERAND_NONE);

                const Tac_Label_Id label_id = ssa_destination->label_id;
                const Cfg_Block_Id destination_block_id = ssa_function->label_index_to_cfg_block_id_map[label_id.index];

                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_JUMP_IF_FALSE;

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

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];
        MIR_Function* mir_function = &mir->functions[function_index];

        mir_function->ast_function_definition = tac_function->ast_function_definition;
        mir_function->tac_function = tac_function;

        mir_function->virtual_registers_arena = acquire_arena_from_provider(context->arena_provider,
                                                                            string_view("mir-function-virtual-registers"),
                                                                            GiB(1),
                                                                            MiB(1));
        mir_function->stack_slots_arena = acquire_arena_from_provider(context->arena_provider,
                                                                      string_view("mir-function-stack-slots"),
                                                                      GiB(1),
                                                                      MiB(1));

        // TODO(vlad): Reserve space for virtual registers.

        for (Index variable_index = tac_function->first_tac_variable_index;
             variable_index < tac_function->last_tac_variable_index;
             ++variable_index)
        {
            Tac_Variable* ssa_variable = &tac->variables[variable_index];
            ssa_variable->mir_virtual_register_offset = mir_function->virtual_registers_count;

            ASSERT(ssa_variable->max_ssa_version != 0);

            for (Index version = 1;
                 version <= ssa_variable->max_ssa_version;
                 ++version)
            {
                create_virtual_register(mir_function);
            }
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

                            const Index temp_register_index = create_virtual_register(function);

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

                        add_coalescing_hint_to_instruction(context,
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
                        const Index rax_register_index = create_virtual_register(function);

                        Virtual_Register* rax_register = &function->virtual_registers[rax_register_index];
                        rax_register->kind = REGISTER_GPR64;
                        rax_register->fixed_physical_register = X86_64_RAX;

                        rax_operand.kind = MIR_OPERAND_VIRTUAL_REGISTER;
                        rax_operand.virtual_register_index = rax_register_index;
                    }

                    {
                        const Index rdx_register_index = create_virtual_register(function);

                        Virtual_Register* rdx_register = &function->virtual_registers[rdx_register_index];
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
                                const Index parameter_register_index = create_virtual_register(function);

                                Virtual_Register* parameter_register = &function->virtual_registers[parameter_register_index];
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

                    // NOTE(vlad): Adding coalescing hint so we could eliminate this move if possible.
                    {
                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 1);

                        // NOTE(vlad): Sanity check.
                        ASSERT(instruction->implicit_definitions_count == 0);
                        ASSERT(instruction->implicit_uses_count == 0);

                        MIR_Operand* destination = &instruction->definitions[0];
                        MIR_Operand* first_argument = &instruction->uses[0];

                        ASSERT(destination->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                        if (first_argument->kind == MIR_OPERAND_VIRTUAL_REGISTER)
                        {
                            add_coalescing_hint_to_instruction(context,
                                                               instruction,
                                                               destination->virtual_register_index,
                                                               first_argument->virtual_register_index);
                        }
                    }
                } break;

                case MIR_JUMP:
                {
                    // NOTE(vlad): 'jmp' instruction has no constraints.
                } break;

                case MIR_JUMP_IF_FALSE:
                {
                    // NOTE(vlad): We don't keep track of the EFLAGS, thus conditional jumps have no ISA-specific
                    //             constraints here. The condition will be rematerialized during the binary emission.
                } break;

                case MIR_PHI:
                {
                    // NOTE(vlad): PHI instruction is not implemented in CPUs, thus it has no ISA-specific constraints.

                    // NOTE(vlad): Adding coalescing hints so we can later remove redundant moves.
                    {
                        const MIR_Operand* definition = &instruction->phi_definition;
                        ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                        // NOTE(vlad): Sanity check.
                        ASSERT(instruction->definitions_count == 0);
                        ASSERT(instruction->uses_count == 0);

                        ASSERT(instruction->implicit_definitions_count == 0);
                        ASSERT(instruction->implicit_uses_count == 0);

                        for (Index phi_argument_index = 0;
                             phi_argument_index < instruction->phi_arguments_count;
                             ++phi_argument_index)
                        {
                            const MIR_PHI_Argument* argument = &instruction->phi_arguments[phi_argument_index];
                            add_coalescing_hint_to_instruction(context,
                                                               instruction,
                                                               definition->virtual_register_index,
                                                               argument->virtual_register_index);
                        }
                    }
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
                            local_persist const Index argument_registers[] = {
                                X86_64_RCX,
                                X86_64_RDX,
                                X86_64_R8,
                                X86_64_R9,
                            };

                            local_persist const Index caller_saved_registers[] = {
                                X86_64_RAX,
                                X86_64_RCX,
                                X86_64_RDX,
                                X86_64_R8,
                                X86_64_R9,
                                X86_64_R10,
                                X86_64_R11,
                            };

                            local_persist const Index return_value_register = X86_64_RAX;

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
                                    const Index argument_register_index = create_virtual_register(function);

                                    Virtual_Register* argument_register = &function->virtual_registers[argument_register_index];
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
                            // TODO(vlad): We would want to use function->current_stack_offset for that but it is not
                            //             possible until the registers are allocated because we have no idea when and
                            //             what will be spilled on the stack. That said, we need to emit a GROW_STACK
                            //             instruction here and patch it after register allocation is complete.

                            // NOTE(vlad): Allocating shadow space.
                            {
                                MIR_Instruction* grow_instruction = prepend_instruction(context, block, instruction);
                                grow_instruction->opcode = MIR_GROW_STACK;

                                MIR_Operand* amount_operand = add_new_use_operand(grow_instruction);
                                amount_operand->kind = MIR_OPERAND_IMMEDIATE_VALUE;
                                amount_operand->immediate_value = 32;

                                add_isa_constraints_to_mir_instruction(context, function, block, grow_instruction);
                            }

                            MIR_Operand* return_value_operand = NULL;

                            // NOTE(vlad): Treat caller-saved registers as clobbered.
                            for (Index register_index = 0;
                                 register_index < NUMBER_OF_STATIC_ARRAY_ELEMENTS(caller_saved_registers);
                                 ++register_index)
                            {
                                MIR_Operand* operand = add_new_implicit_def_operand(instruction);

                                const Index virtual_register_index = create_virtual_register(function);
                                const Index physical_register = caller_saved_registers[register_index];

                                Virtual_Register* virtual_register = &function->virtual_registers[virtual_register_index];
                                virtual_register->kind = REGISTER_GPR64;
                                virtual_register->fixed_physical_register = physical_register;

                                operand->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                operand->virtual_register_index = virtual_register_index;

                                if (physical_register == return_value_register)
                                {
                                    return_value_operand = operand;
                                }
                            }

                            MIR_Instruction* last_instruction = instruction;

                            if (instruction->has_return_value)
                            {
                                // NOTE(vlad): Retrieving returned value.
                                MIR_Instruction* move_instruction = append_instruction(context, block, instruction);

                                const MIR_Operand* return_operand = &instruction->return_operand;

                                move_instruction->opcode = MIR_MOVE;

                                MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                                *move_destination = *return_operand;

                                MIR_Operand* move_argument = add_new_use_operand(move_instruction);
                                *move_argument = *return_value_operand;

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
                                    const Index rax_register_index = create_virtual_register(function);

                                    Virtual_Register* rax_register = &function->virtual_registers[rax_register_index];
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

struct MIR_Reverse_Post_Order
{
    array(MIR_Block*, blocks);
    array(MIR_Block*, visited_blocks); // TODO(vlad): Move this out.
};
typedef struct MIR_Reverse_Post_Order MIR_Reverse_Post_Order;

internal void
visit_mir_blocks_in_post_order(Arena* arena,
                               MIR_Block* block,
                               MIR_Reverse_Post_Order* rpo)
{
    for (Index block_index = 0;
         block_index < rpo->visited_blocks_count;
         ++block_index)
    {
        const MIR_Block* visited_block = rpo->visited_blocks[block_index];
        if (visited_block == block)
        {
            return;
        }
    }

    append_array(arena, rpo->visited_blocks, MIR_Block*, block);

    for (Index successor_index = 0;
         successor_index < block->successors_count;
         ++successor_index)
    {
        MIR_Block* successor = block->successors[successor_index];
        visit_mir_blocks_in_post_order(arena, successor, rpo);
    }

    append_array(arena, rpo->blocks, MIR_Block*, block);
}

internal MIR_Reverse_Post_Order
calculate_reverse_post_order_of_mir_blocks(Arena* arena,
                                           MIR_Function* function)
{
    MIR_Reverse_Post_Order reverse_post_order = {0};
    visit_mir_blocks_in_post_order(arena, function->entry_block, &reverse_post_order);
    reverse_array(reverse_post_order.blocks, MIR_Block*);
    return reverse_post_order;
}

internal MIR_Register_Bitset
create_register_bitset(Compilation_Context* context,
                       const Size number_of_virtual_registers)
{
    MIR_Register_Bitset bitset = {0};
    const Size word_size_in_bits = size_of(bitset.words[0]) * 8;
    const Size number_of_words_to_allocate = (number_of_virtual_registers + word_size_in_bits - 1) / word_size_in_bits;
    bitset.words = allocate_array(context->scratch_arena, number_of_words_to_allocate, u64);
    bitset.words_count = number_of_words_to_allocate;
    return bitset;
}

internal inline void
clear_register_bitset(MIR_Register_Bitset* bitset)
{
    fill_with_zeros(bitset->words, bitset->words_count, u64);
}

internal inline void
set_register_bit(MIR_Register_Bitset* bitset, const Index virtual_register_index)
{
    const Size word_size_in_bits = size_of(bitset->words[0]) * 8;

    const Index word_index = virtual_register_index / word_size_in_bits;
    const Index bit_index = virtual_register_index % word_size_in_bits;
    const u64 word_mask = (1ull << bit_index);

    ASSERT(word_index < bitset->words_count);

    bitset->words[word_index] |= word_mask;
}

internal inline void
clear_register_bit(MIR_Register_Bitset* bitset, const Index virtual_register_index)
{
    const Size word_size_in_bits = size_of(bitset->words[0]) * 8;

    const Index word_index = virtual_register_index / word_size_in_bits;
    const Index bit_index = virtual_register_index % word_size_in_bits;
    const u64 word_mask = (1ull << bit_index);

    ASSERT(word_index < bitset->words_count);

    bitset->words[word_index] &= ~word_mask;
}

internal inline Bool
register_bit_is_set(MIR_Register_Bitset* bitset, const Index virtual_register_index)
{
    const Size word_size_in_bits = size_of(bitset->words[0]) * 8;

    const Index word_index = virtual_register_index / word_size_in_bits;
    const Index bit_index = virtual_register_index % word_size_in_bits;
    const u64 word_mask = (1ull << bit_index);

    ASSERT(word_index < bitset->words_count);

    return (bitset->words[word_index] & word_mask) != 0;
}

internal inline void
add_register_bitset(MIR_Register_Bitset* destination, const MIR_Register_Bitset* source)
{
    ASSERT(destination->words_count == source->words_count);

    for (Index word_index = 0;
         word_index < destination->words_count;
         ++word_index)
    {
        destination->words[word_index] |= source->words[word_index];
    }
}

internal inline Bool
register_bitsets_are_equal(const MIR_Register_Bitset* lhs, const MIR_Register_Bitset* rhs)
{
    ASSERT(lhs->words_count == rhs->words_count);
    return memory_chunks_are_equal(as_bytes(lhs->words), as_bytes(rhs->words), lhs->words_count * size_of(lhs->words[0]));
}

internal inline MIR_Register_Bitset
copy_register_bitset(Compilation_Context* context,
                     const MIR_Register_Bitset* source)
{
    MIR_Register_Bitset copy = {0};
    copy.words = allocate_array(context->scratch_arena, source->words_count, u64);
    copy.words_count = source->words_count;
    copy_memory(as_bytes(copy.words), as_bytes(source->words), source->words_count * size_of(copy.words[0]));
    return copy;
}

#define FOR_EACH_REGISTER_IN_SET(bitset, register_index)                \
    for (Index word_index = 0;                                          \
         word_index < (bitset)->words_count;                            \
         ++word_index)                                                  \
        for (u64 word = (bitset)->words[word_index];                    \
             word != 0;                                                 \
             word &= word - 1)                                          \
            if (((register_index) = (word_index * (size_of((bitset)->words[0]) * 8) \
                                      + count_trailing_zero_bits(word))), \
                true)

struct Interference_Graph
{
    MIR_Register_Bitset* adjacent_registers_set;
    Size number_of_virtual_registers;
};
typedef struct Interference_Graph Interference_Graph;

internal void
add_interference(Interference_Graph* graph,
                 const Index lhs_virtual_register_index,
                 const Index rhs_virtual_register_index)
{
    if (lhs_virtual_register_index == rhs_virtual_register_index)
    {
        // NOTE(vlad): Virtual register does not interfere with itself.
        return;
    }

    set_register_bit(&graph->adjacent_registers_set[lhs_virtual_register_index], rhs_virtual_register_index);
    set_register_bit(&graph->adjacent_registers_set[rhs_virtual_register_index], lhs_virtual_register_index);
}

internal void
add_interference_with_live_registers(Interference_Graph* graph,
                                     const Index virtual_register_index,
                                     MIR_Register_Bitset* live_registers)
{
    Index other_live_register_index = 0;
    FOR_EACH_REGISTER_IN_SET(live_registers, other_live_register_index)
    {
        add_interference(graph, virtual_register_index, other_live_register_index);
    }
}

struct MIR_Perfect_Elimination_Order
{
    Index* virtual_register_indices;
    Size virtual_register_indices_count;
};
typedef struct MIR_Perfect_Elimination_Order MIR_Perfect_Elimination_Order;

internal void
allocate_registers(Compilation_Context* context)
{
    MIR* mir = &context->mir;

    // TODO(vlad): Should we accept this as an argument?
    Size available_registers_count = 0;
    switch (context->target_architecture)
    {
        case TARGET_ARCH_X86_64:
        {
            available_registers_count = X86_64_GPR_COUNT;
        } break;

        case TARGET_ARCH_AARCH64:
        {
            FAIL("[MIR] This architecture is not supported yet.");
        } break;
    }

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        MIR_Function* function = &mir->functions[function_index];

        MIR_Reverse_Post_Order reverse_post_order = calculate_reverse_post_order_of_mir_blocks(context->scratch_arena,
                                                                                               function);

        // NOTE(vlad): Performing liveness analysis.
        {
            for (Index block_index = 0;
                 block_index < reverse_post_order.blocks_count;
                 ++block_index)
            {
                MIR_Block* block = reverse_post_order.blocks[block_index];

                block->live_in_registers = create_register_bitset(context, function->virtual_registers_count);
                block->live_out_registers = create_register_bitset(context, function->virtual_registers_count);
            }

            Bool some_set_was_changed = true;
            while (some_set_was_changed)
            {
                some_set_was_changed = false;

                for (Index block_index = reverse_post_order.blocks_count - 1;
                     block_index >= 0;
                     --block_index)
                {
                    MIR_Block* this_block = reverse_post_order.blocks[block_index];

                    MIR_Register_Bitset current_live_out_registers = create_register_bitset(context,
                                                                                            function->virtual_registers_count);

                    // NOTE(vlad): Calculating live out registers.
                    {
                        for (Index successor_index = 0;
                             successor_index < this_block->successors_count;
                             ++successor_index)
                        {
                            MIR_Block* successor = this_block->successors[successor_index];
                            add_register_bitset(&current_live_out_registers, &successor->live_in_registers);

                            // NOTE(vlad): If the successor has PHI nodes corresponding to this block
                            //             must be alive at the end of this block.
                            for (MIR_Instruction* instruction = successor->first_instruction;
                                 instruction != NULL && instruction->opcode == MIR_PHI;
                                 instruction = instruction->next_instruction)
                            {
                                for (Index phi_argument_index = 0;
                                     phi_argument_index < instruction->phi_arguments_count;
                                     ++phi_argument_index)
                                {
                                    const MIR_PHI_Argument* argument = &instruction->phi_arguments[phi_argument_index];
                                    if (argument->source_block == this_block)
                                    {
                                        set_register_bit(&current_live_out_registers, argument->virtual_register_index);
                                    }
                                }
                            }
                        }
                    }

                    MIR_Register_Bitset current_live_in_registers = copy_register_bitset(context,
                                                                                         &current_live_out_registers);

                    for (MIR_Instruction* instruction = this_block->last_instruction;
                         instruction != NULL;
                         instruction = instruction->previous_instruction)
                    {
                        // FIXME(vlad): Refactor this code.

                        Bool should_process_implicit_definitions = true;
                        Bool should_process_implicit_uses = true;

                        switch (instruction->opcode)
                        {
                            case MIR_UNDEFINED:
                            {
                                UNREACHABLE();
                            } break;

                            case MIR_CALL:
                            {
                                // NOTE(vlad): Calls do not have explicit use/def lists.
                            } break;

                            case MIR_PHI:
                            {
                                should_process_implicit_uses = false;

                                // TODO(vlad): We probably do not need should_process_implicit_uses/defs.
                                ASSERT(instruction->implicit_uses_count == 0);
                                ASSERT(instruction->implicit_definitions_count == 0);

                                MIR_Operand* definition = &instruction->phi_definition;
                                ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                                clear_register_bit(&current_live_in_registers, definition->virtual_register_index);
                            } break;

                            default:
                            {
                                // NOTE(vlad): If virtual register was defined in the instruction then it was born here
                                //             thus was not alive at the start of the current block.
                                for (Index definition_index = 0;
                                     definition_index < instruction->definitions_count;
                                     ++definition_index)
                                {
                                    MIR_Operand* definition = &instruction->definitions[definition_index];
                                    ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                                    clear_register_bit(&current_live_in_registers, definition->virtual_register_index);
                                }

                                // NOTE(vlad): If virtual register was used here then it must be born before this instruction.
                                for (Index use_index = 0;
                                     use_index < instruction->uses_count;
                                     ++use_index)
                                {
                                    MIR_Operand* use = &instruction->uses[use_index];
                                    if (use->kind == MIR_OPERAND_VIRTUAL_REGISTER)
                                    {
                                        set_register_bit(&current_live_in_registers, use->virtual_register_index);
                                    }
                                }
                            } break;
                        }

                        if (should_process_implicit_definitions)
                        {
                            for (Index definition_index = 0;
                                 definition_index < instruction->implicit_definitions_count;
                                 ++definition_index)
                            {
                                MIR_Operand* definition = &instruction->implicit_definitions[definition_index];
                                ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                                clear_register_bit(&current_live_in_registers, definition->virtual_register_index);
                            }
                        }

                        if (should_process_implicit_uses)
                        {
                            for (Index use_index = 0;
                                 use_index < instruction->implicit_uses_count;
                                 ++use_index)
                            {
                                MIR_Operand* use = &instruction->implicit_uses[use_index];
                                if (use->kind == MIR_OPERAND_VIRTUAL_REGISTER)
                                {
                                    set_register_bit(&current_live_in_registers, use->virtual_register_index);
                                }
                            }
                        }
                    }

                    const Bool live_in_changed = !register_bitsets_are_equal(&current_live_in_registers,
                                                                             &this_block->live_in_registers);
                    const Bool live_out_changed = !register_bitsets_are_equal(&current_live_out_registers,
                                                                             &this_block->live_out_registers);

                    if (live_in_changed || live_out_changed)
                    {
                        some_set_was_changed = true;
                        this_block->live_in_registers = current_live_in_registers;
                        this_block->live_out_registers = current_live_out_registers;
                    }
                }
            }
        }

        Interference_Graph graph = {0};

        // NOTE(vlad): Building the interference graph.
        {
            graph.number_of_virtual_registers = function->virtual_registers_count;
            graph.adjacent_registers_set = allocate_array(context->scratch_arena,
                                                          graph.number_of_virtual_registers,
                                                          MIR_Register_Bitset);

            for (Index set_index = 0;
                 set_index < graph.number_of_virtual_registers;
                 ++set_index)
            {
                graph.adjacent_registers_set[set_index] = create_register_bitset(context,
                                                                                 graph.number_of_virtual_registers);
            }

            for (Index block_index = 0;
                 block_index < reverse_post_order.blocks_count;
                 ++block_index)
            {
                MIR_Block* block = reverse_post_order.blocks[block_index];

                MIR_Register_Bitset live_registers = copy_register_bitset(context, &block->live_out_registers);

                for (MIR_Instruction* instruction = block->last_instruction;
                     instruction != NULL;
                     instruction = instruction->previous_instruction)
                {
                    // FIXME(vlad): Refactor this code.

                    ASSERT(instruction->opcode != MIR_UNDEFINED);

                    if (instruction->opcode == MIR_PHI)
                    {
                        // TODO(vlad): Merge definitions and phi_definition?
                        ASSERT(instruction->definitions_count == 0);
                        ASSERT(instruction->uses_count == 0);

                        ASSERT(instruction->implicit_definitions_count == 0);
                        ASSERT(instruction->implicit_uses_count == 0);

                        const MIR_Operand* definition = &instruction->phi_definition;
                        ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                        const Index virtual_register_index = definition->virtual_register_index;
                        add_interference_with_live_registers(&graph, virtual_register_index, &live_registers);
                        clear_register_bit(&live_registers, virtual_register_index);
                    }

                    // NOTE(vlad): Adding these definitions to the interference graph.
                    {
                        for (Index definition_index = 0;
                             definition_index < instruction->definitions_count;
                             ++definition_index)
                        {
                            const MIR_Operand* definition = &instruction->definitions[definition_index];
                            ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);
                            add_interference_with_live_registers(&graph,
                                                                 definition->virtual_register_index,
                                                                 &live_registers);
                        }

                        for (Index definition_index = 0;
                             definition_index < instruction->implicit_definitions_count;
                             ++definition_index)
                        {
                            const MIR_Operand* definition = &instruction->implicit_definitions[definition_index];
                            ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);
                            add_interference_with_live_registers(&graph,
                                                                 definition->virtual_register_index,
                                                                 &live_registers);
                        }
                    }

                    // NOTE(vlad): Removing definitions from live registers set.
                    {
                        for (Index definition_index = 0;
                             definition_index < instruction->definitions_count;
                             ++definition_index)
                        {
                            const MIR_Operand* definition = &instruction->definitions[definition_index];
                            ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);
                            clear_register_bit(&live_registers, definition->virtual_register_index);
                        }

                        for (Index definition_index = 0;
                             definition_index < instruction->implicit_definitions_count;
                             ++definition_index)
                        {
                            const MIR_Operand* definition = &instruction->implicit_definitions[definition_index];
                            ASSERT(definition->kind == MIR_OPERAND_VIRTUAL_REGISTER);
                            clear_register_bit(&live_registers, definition->virtual_register_index);
                        }
                    }

                    // NOTE(vlad): Adding uses to live registers set.
                    {
                        for (Index use_index = 0;
                             use_index < instruction->uses_count;
                             ++use_index)
                        {
                            const MIR_Operand* use = &instruction->uses[use_index];
                            if (use->kind == MIR_OPERAND_VIRTUAL_REGISTER)
                            {
                                set_register_bit(&live_registers, use->virtual_register_index);
                            }
                        }

                        for (Index use_index = 0;
                             use_index < instruction->implicit_uses_count;
                             ++use_index)
                        {
                            const MIR_Operand* use = &instruction->implicit_uses[use_index];
                            if (use->kind == MIR_OPERAND_VIRTUAL_REGISTER)
                            {
                                set_register_bit(&live_registers, use->virtual_register_index);
                            }
                        }
                    }
                }

                // TODO(vlad): Add a sanity check that live_registers set exactly equals to the live_in_registers of
                //             this block.

                // TODO(vlad): Can we move this out of the for-loop?
                if (block == function->entry_block)
                {
                    // NOTE(vlad): Function arguments have no definitions, but must interfere with each other.
                    Index argument_virtual_register_index = 0;
                    FOR_EACH_REGISTER_IN_SET(&live_registers, argument_virtual_register_index)
                    {
                        add_interference_with_live_registers(&graph, argument_virtual_register_index, &live_registers);
                    }
                }
            }
        }

        MIR_Perfect_Elimination_Order perfect_elimination_order = {0};

        // NOTE(vlad): Computing PEO.
        {
            perfect_elimination_order.virtual_register_indices = allocate_array(context->scratch_arena,
                                                                                function->virtual_registers_count,
                                                                                Index);
            perfect_elimination_order.virtual_register_indices_count = function->virtual_registers_count;

            // NOTE(vlad): We are using Maximum Cardinality Search (MCS) to calculate the PEO.

            Index* weights = allocate_array(context->scratch_arena, function->virtual_registers_count, Index);
            Bool* register_was_selected = allocate_array(context->scratch_arena, function->virtual_registers_count, Bool);

            for (Index order_index = 0;
                 order_index < function->virtual_registers_count;
                 ++order_index)
            {
                Index best_weight = -1;
                Index best_register_index = -1;

                for (Index register_index = 0;
                     register_index < function->virtual_registers_count;
                     ++register_index)
                {
                    if (!register_was_selected[register_index]
                        && (weights[register_index] > best_weight))
                    {
                        best_weight = weights[register_index];
                        best_register_index = register_index;
                    }
                }

                ASSERT(best_weight != -1);
                ASSERT(best_register_index != -1);

                register_was_selected[best_register_index] = true;
                perfect_elimination_order.virtual_register_indices[order_index] = best_register_index;

                // NOTE(vlad): Incrementing weights of the best register's unselected neighbours.
                {
                    Index neighbour_index = 0;
                    FOR_EACH_REGISTER_IN_SET(&graph.adjacent_registers_set[best_register_index], neighbour_index)
                    {
                        if (!register_was_selected[neighbour_index])
                        {
                            weights[neighbour_index] += 1;
                        }
                    }
                }
            }
        }

        Bool registers_were_spilled = false;

        // NOTE(vlad): Coloring the interference graph.
        {
            local_array(Coalescing_Hint, coalescing_hints);
            {
                // NOTE(vlad): The order in which we traverse function blocks does not matter
                //             so we can use the RPO since we've already built it.
                for (Index block_index = 0;
                     block_index < reverse_post_order.blocks_count;
                     ++block_index)
                {
                    const MIR_Block* block = reverse_post_order.blocks[block_index];

                    for (MIR_Instruction* instruction = block->first_instruction;
                         instruction != NULL;
                         instruction = instruction->next_instruction)
                    {
                        for (Index hint_index = 0;
                             hint_index < instruction->coalescing_hints_count;
                             ++hint_index)
                        {
                            append_array(context->scratch_arena,
                                         coalescing_hints,
                                         Coalescing_Hint,
                                         instruction->coalescing_hints[hint_index]);
                        }
                    }
                }
            }

            Bool* color_was_used = allocate_array(context->scratch_arena, available_registers_count, Bool);

            for (Index order_index = perfect_elimination_order.virtual_register_indices_count - 1;
                 order_index >= 0;
                 --order_index)
            {
                const Index this_register_index = perfect_elimination_order.virtual_register_indices[order_index];

                // NOTE(vlad): Computing what colors are used by interfering neighbours.
                {
                    // NOTE(vlad): Physical register indices start from 1.
                    color_was_used[NO_REGISTER] = true;

                    Index neighbour_index = 0;
                    FOR_EACH_REGISTER_IN_SET(&graph.adjacent_registers_set[this_register_index], neighbour_index)
                    {
                        Virtual_Register* neighbour = &function->virtual_registers[neighbour_index];
                        if (neighbour->assigned_physical_register != NO_REGISTER)
                        {
                            color_was_used[neighbour->assigned_physical_register] = true;
                        }
                    }
                }

                Virtual_Register* this_register = &function->virtual_registers[this_register_index];
                if (this_register->fixed_physical_register != NO_REGISTER)
                {
                    // NOTE(vlad): ISA forces us to use the specified register here.

                    const Index forced_color = this_register->fixed_physical_register;

                    if (!color_was_used[forced_color])
                    {
                        this_register->assigned_physical_register = forced_color;
                        goto color_next_virtual_register;
                    }

                    // NOTE(vlad): The specified register is not available so we need to find the
                    //             neighbour that aquired it and try to recolor him.

                    Index neighbour_index = 0;
                    FOR_EACH_REGISTER_IN_SET(&graph.adjacent_registers_set[this_register_index], neighbour_index)
                    {
                        Virtual_Register* neighbour = &function->virtual_registers[neighbour_index];
                        if (neighbour->assigned_physical_register == forced_color)
                        {
                            // NOTE(vlad): We have a conflict here, trying to recolor the neighbour.

                            if (neighbour->assigned_physical_register == neighbour->fixed_physical_register)
                            {
                                // NOTE(vlad): Uh-oh, we cannot recolor the neighbour because the ISA forced us
                                //             to pick this color. That means that the spilling is unavoidable, so
                                //             we leave the neighbour alone.
                                this_register->fixed_physical_register = SPILLED_TO_STACK;
                                registers_were_spilled = true;
                                goto color_next_virtual_register;
                            }

                            // NOTE(vlad): Sanity check: if the ISA forces us to choose the physical register
                            //             then we MUST choose it.
                            ASSERT(neighbour->fixed_physical_register == NO_REGISTER);

                            Index new_color_of_neighbour = NO_REGISTER;
                            for (Index color_index = 0;
                                 color_index < available_registers_count;
                                 ++color_index)
                            {
                                if (!color_was_used[color_index])
                                {
                                    // TODO(vlad): Check that this color can be chosen: for example, we cannot just use the GPR64 if we MUST use FP64.

                                    new_color_of_neighbour = color_index;
                                    break;
                                }
                            }

                            if (new_color_of_neighbour == NO_REGISTER)
                            {
                                // NOTE(vlad): We have to spill here because there are no available physical registers
                                //             left for the neighbour.
                                neighbour->assigned_physical_register = SPILLED_TO_STACK;
                                registers_were_spilled = true;
                            }
                            else
                            {
                                neighbour->assigned_physical_register = new_color_of_neighbour;
                            }
                        }
                    }

                    // NOTE(vlad): We can safely assign the forced color because all conflicting neighbours were
                    //             recolored.
                    this_register->assigned_physical_register = forced_color;
                    goto color_next_virtual_register;
                }

                Index preferred_color = NO_REGISTER;
                // NOTE(vlad): Looking at coalescing hints first so we could reuse the physical register.
                {
                    for (Index hint_index = 0;
                         hint_index < coalescing_hints_count;
                         ++hint_index)
                    {
                        const Coalescing_Hint* hint = &coalescing_hints[hint_index];

                        Index other_register_index = -1;
                        if (hint->first_virtual_register_index == this_register_index)
                        {
                            other_register_index = hint->second_virtual_register_index;
                        }
                        else if (hint->second_virtual_register_index == this_register_index)
                        {
                            other_register_index = hint->first_virtual_register_index;
                        }

                        if (other_register_index == -1)
                        {
                            continue;
                        }

                        Virtual_Register* other_register = &function->virtual_registers[other_register_index];
                        const Index other_register_color = other_register->assigned_physical_register;

                        if (other_register_color != NO_REGISTER && !color_was_used[other_register_color])
                        {
                            preferred_color = other_register_color;
                            break;
                        }
                    }
                }

                Index chosen_color = preferred_color;
                if (chosen_color == NO_REGISTER)
                {
                    for (Index color_index = 0;
                         color_index < available_registers_count;
                         ++color_index)
                    {
                        if (!color_was_used[color_index])
                        {
                            // TODO(vlad): Check that this color can be chosen: for example, we cannot just use the GPR64 if we MUST use FP64.

                            chosen_color = color_index;
                            break;
                        }
                    }
                }

                if (chosen_color != NO_REGISTER)
                {
                    this_register->assigned_physical_register = chosen_color;
                }
                else
                {
                    this_register->assigned_physical_register = SPILLED_TO_STACK;
                    registers_were_spilled = true;
                }

        color_next_virtual_register:
                fill_memory_with_zeros(as_bytes(color_was_used), size_of(color_was_used[0]) * available_registers_count);
            }
        }

        if (registers_were_spilled)
        {
            FAIL("[MIR] Spilled virtual registers are not supported yet.");
#if 0
            // NOTE(vlad): We need to allocate stack slots for these registers.

            // TODO(vlad): Experiment with multiple iterations:
            //             1. Do the whole allocation pipeline
            //             2. Try to color the interference graph
            //             3. If some registers were spilled, insert STORE/LOAD instructions creating
            //                a brand new virtual register after LOAD.
            //             4. Loop until the number of spills reaches 0.
            //
            //             In our current implementation we just spill the virtual register onto stack for its entire
            //             lifetime. The resulting code is slower than it can be but the register allocation
            //             algorithm is simpler.

            // NOTE(vlad): The order in which we iterate over registers does not matter
            //             so we just reuse the PEO.
            for (Index order_index = 0;
                 order_index < perfect_elimination_order.virtual_register_indices_count;
                 ++order_index)
            {
                const Index this_register_index = perfect_elimination_order.virtual_register_indices[order_index];

                Virtual_Register* this_register = &function->virtual_registers[this_register_index];

                if (this_register->fixed_physical_register == SPILLED_TO_STACK)
                {
                    const Index stack_slot = create_stack_slot(function, /* size_in_bytes */ 8);
                    this_register->stack_slot_index = stack_slot;
                }
            }
#endif
        }

        // NOTE(vlad): Eliminating PHI nodes.
        {
            Bool some_edge_was_splitted = false;

            // NOTE(vlad): Splitting critical edges.
            {
                for (Index block_index = 0;
                     block_index < reverse_post_order.blocks_count;
                     ++block_index)
                {
                    MIR_Block* block = reverse_post_order.blocks[block_index];

                    if (block->successors_count < 2)
                    {
                        continue;
                    }

                    for (Index successor_index = 0;
                         successor_index < block->successors_count;
                         ++successor_index)
                    {
                        MIR_Block* successor = block->successors[successor_index];

                        if (successor->predecessors_count < 2)
                        {
                            continue;
                        }

                        // NOTE(vlad): 'block -> successor' edge is critical.

                        some_edge_was_splitted = true;

                        MIR_Block* split_block = create_new_mir_block(context);
                        MIR_Instruction* jump_instruction = add_new_instruction_to_mir_block(context, split_block);
                        {
                            MIR_Operand* destination = add_new_use_operand(jump_instruction);
                            destination->kind = MIR_OPERAND_BLOCK;
                            destination->block = successor;
                        }
                        // TODO(vlad): Should we add ISA constraints here?

                        split_block->successors = allocate_array(context->mir_edges_arena, 1, MIR_Block*);
                        split_block->successors_count = 1;
                        split_block->successors[0] = successor;

                        split_block->predecessors = allocate_array(context->mir_edges_arena, 1, MIR_Block*);
                        split_block->predecessors_count = 1;
                        split_block->predecessors[0] = block;

                        block->successors[successor_index] = split_block;

                        for (Index predecessor_index = 0;
                             predecessor_index < successor->predecessors_count;
                             ++predecessor_index)
                        {
                            MIR_Block** predecessor = &successor->predecessors[predecessor_index];

                            if (*predecessor == block)
                            {
                                *predecessor = split_block;
                                break;
                            }
                        }

                        for (MIR_Instruction* instruction = successor->first_instruction;
                             instruction != NULL && instruction->opcode == MIR_PHI;
                             instruction = instruction->next_instruction)
                        {
                            for (Index phi_argument_index = 0;
                                 phi_argument_index < instruction->phi_arguments_count;
                                 ++phi_argument_index)
                            {
                                MIR_PHI_Argument* argument = &instruction->phi_arguments[phi_argument_index];

                                if (argument->source_block == block)
                                {
                                    argument->source_block = split_block;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if (some_edge_was_splitted)
            {
                // NOTE(vlad): For simplicity's sake we recompute the RPO so we can easily traverse function blocks.
                reverse_post_order = calculate_reverse_post_order_of_mir_blocks(context->scratch_arena, function);
            }

            // NOTE(vlad): Converting PHI nodes to parallel copies.
            {
                for (Index block_index = 0;
                     block_index < reverse_post_order.blocks_count;
                     ++block_index)
                {
                    MIR_Block* block = reverse_post_order.blocks[block_index];

                    MIR_Instruction* instruction = block->first_instruction;
                    while (instruction != NULL && instruction->opcode == MIR_PHI)
                    {
                        MIR_Instruction* next_instruction = instruction->next_instruction;

                        MIR_Operand* destination_operand = &instruction->phi_definition;
                        ASSERT(destination_operand->kind == MIR_OPERAND_VIRTUAL_REGISTER);

                        for (Index phi_argument_index = 0;
                             phi_argument_index < instruction->phi_arguments_count;
                             ++phi_argument_index)
                        {
                            MIR_PHI_Argument* argument = &instruction->phi_arguments[phi_argument_index];

                            const Index source_register_index = argument->virtual_register_index;
                            MIR_Block* source_block = argument->source_block;

                            const Virtual_Register* source_register = &function->virtual_registers[source_register_index];
                            const Virtual_Register* destination_register
                                = &function->virtual_registers[destination_operand->virtual_register_index];

                            ASSERT(source_register->assigned_physical_register != NO_REGISTER);
                            ASSERT(source_register->assigned_physical_register != SPILLED_TO_STACK);

                            ASSERT(destination_register->assigned_physical_register != NO_REGISTER);
                            ASSERT(destination_register->assigned_physical_register != SPILLED_TO_STACK);

                            if (source_register->assigned_physical_register == destination_register->assigned_physical_register)
                            {
                                // NOTE(vlad): This copy will be removed as redundant anyway.
                                continue;
                            }

                            MIR_Parallel_Copy copy = {0};
                            copy.destination_virtual_register_index = destination_operand->virtual_register_index;
                            copy.destination_physical_register = destination_register->assigned_physical_register;
                            copy.source_virtual_register_index = source_register_index;
                            copy.source_physical_register = source_register->assigned_physical_register;

                            append_array(context->scratch_arena, source_block->parallel_copies, MIR_Parallel_Copy, copy);
                        }

                        // NOTE(vlad): Removing PHI node.
                        {
                            MIR_Instruction* previous_instruction = instruction->previous_instruction;

                            // NOTE(vlad): MIR_PHI cannot be the last instruction of the block.
                            ASSERT(next_instruction != NULL);
                            ASSERT(previous_instruction == NULL);

                            next_instruction->previous_instruction = NULL;
                            block->first_instruction = next_instruction;
                        }

                        instruction = next_instruction;
                    }
                }
            }

            // NOTE(vlad): Resolving parallel copies.
            {
                for (Index block_index = 0;
                     block_index < reverse_post_order.blocks_count;
                     ++block_index)
                {
                    MIR_Block* block = reverse_post_order.blocks[block_index];

                    MIR_Instruction* last_instruction = block->last_instruction;
                    ASSERT(last_instruction != NULL);
                    ASSERT(instruction_is_a_block_terminator(last_instruction));

                    Size emitted_moves_count = 0;
                    while (emitted_moves_count < block->parallel_copies_count)
                    {
                        Bool move_was_emitted = false;

                        // NOTE(vlad): Finding safe copy to emit.
                        for (Index copy_index = 0;
                             copy_index < block->parallel_copies_count;
                             ++copy_index)
                        {
                            MIR_Parallel_Copy* copy = &block->parallel_copies[copy_index];

                            if (copy->move_instruction_was_emitted)
                            {
                                continue;
                            }

                            Bool is_safe_to_emit = true;
                            for (Index other_copy_index = 0;
                                 other_copy_index < block->parallel_copies_count;
                                 ++other_copy_index)
                            {
                                MIR_Parallel_Copy* other_copy = &block->parallel_copies[other_copy_index];

                                if (other_copy->move_instruction_was_emitted)
                                {
                                    continue;
                                }

                                if (other_copy->source_physical_register == copy->destination_physical_register)
                                {
                                    is_safe_to_emit = false;
                                    break;
                                }
                            }

                            if (is_safe_to_emit)
                            {
                                MIR_Instruction* move_instruction = prepend_instruction(context,
                                                                                        block,
                                                                                        last_instruction);
                                move_instruction->opcode = MIR_MOVE;

                                MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                                move_destination->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                move_destination->virtual_register_index = copy->destination_virtual_register_index;

                                MIR_Operand* move_source = add_new_use_operand(move_instruction);
                                move_source->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                move_source->virtual_register_index = copy->source_virtual_register_index;

                                // TODO(vlad): Should we add ISA constraints here?

                                copy->move_instruction_was_emitted = true;
                                emitted_moves_count += 1;
                                move_was_emitted = true;
                            }
                        }

                        if (!move_was_emitted)
                        {
#if EON_DEBUG_BUILD
                            println("[MIR] Found cycle during parallel copies resolution!");
#endif
                            // NOTE(vlad): We stuck in a cycle, breaking it.

                            Index next_pending_copy_index = -1;
                            for (Index copy_index = 0;
                                 copy_index < block->parallel_copies_count;
                                 ++copy_index)
                            {
                                MIR_Parallel_Copy* copy = &block->parallel_copies[copy_index];

                                if (!copy->move_instruction_was_emitted)
                                {
                                    next_pending_copy_index = copy_index;
                                    break;
                                }
                            }

                            ASSERT(next_pending_copy_index != -1);
                            MIR_Parallel_Copy* copy = &block->parallel_copies[next_pending_copy_index];

                            if (copy->move_instruction_was_emitted)
                            {
                                continue;
                            }

                            const Index destination_virtual_register = copy->destination_virtual_register_index;
                            const Index destination_physical_register = copy->destination_physical_register;

                            // NOTE(vlad): Finding a safe scratch register.
                            Bool* register_was_used = allocate_array(context->scratch_arena,
                                                                     available_registers_count,
                                                                     Bool);
                            {
                                register_was_used[NO_REGISTER] = true;

                                Index alive_virtual_register_index = 0;
                                FOR_EACH_REGISTER_IN_SET(&block->live_out_registers, alive_virtual_register_index)
                                {
                                    const Virtual_Register* virtual_register
                                        = &function->virtual_registers[alive_virtual_register_index];

                                    ASSERT(virtual_register->assigned_physical_register != NO_REGISTER);
                                    ASSERT(virtual_register->assigned_physical_register != SPILLED_TO_STACK);

                                    register_was_used[virtual_register->assigned_physical_register] = true;
                                }
                            }

                            Index scratch_physical_register = NO_REGISTER;
                            for (Index candidate_register = 0;
                                 candidate_register < available_registers_count;
                                 ++candidate_register)
                            {
                                if (!register_was_used[candidate_register])
                                {
                                    scratch_physical_register = candidate_register;
                                    break;
                                }
                            }

                            ASSERT(scratch_physical_register != NO_REGISTER);

                            const Index scratch_virtual_register = create_virtual_register(function);
                            {
                                Virtual_Register* virtual_register
                                    = &function->virtual_registers[scratch_virtual_register];
                                virtual_register->fixed_physical_register = scratch_physical_register;
                                virtual_register->assigned_physical_register = scratch_physical_register;
                            }

                            {
                                MIR_Instruction* move_instruction = prepend_instruction(context,
                                                                                        block,
                                                                                        last_instruction);
                                move_instruction->opcode = MIR_MOVE;

                                MIR_Operand* move_destination = add_new_def_operand(move_instruction);
                                move_destination->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                move_destination->virtual_register_index = scratch_virtual_register;

                                MIR_Operand* move_source = add_new_use_operand(move_instruction);
                                move_source->kind = MIR_OPERAND_VIRTUAL_REGISTER;
                                move_source->virtual_register_index = destination_virtual_register;

                                // TODO(vlad): Should we add ISA constraints here?
                            }

                            // NOTE(vlad): Finding the other parallel copy that uses the destination virtual
                            //             register and patch it to use the scratch register instead, thus breaking
                            //             the cycle.
                            Bool cycle_was_broken = false;
                            for (Index other_copy_index = 0;
                                 other_copy_index < block->parallel_copies_count;
                                 ++other_copy_index)
                            {
                                MIR_Parallel_Copy* other_copy = &block->parallel_copies[other_copy_index];

                                if (other_copy->move_instruction_was_emitted)
                                {
                                    continue;
                                }

                                if (other_copy->source_physical_register == destination_physical_register)
                                {
                                    if (cycle_was_broken)
                                    {
                                        FAIL("[MIR] The register was used in more than one parallel copy -- is it possible?");
                                    }

                                    other_copy->source_virtual_register_index = scratch_virtual_register;
                                    other_copy->source_physical_register = scratch_physical_register;

                                    cycle_was_broken = true;
                                }
                            }
                        }
                    }

                    block->parallel_copies_count = 0;
                }
            }
        }

        // NOTE(vlad): Redundant move elimination pass.
        {
            // NOTE(vlad): The order in which we traverse function blocks does not matter
            //             so we can use the RPO since we've already built it.
            for (Index block_index = 0;
                 block_index < reverse_post_order.blocks_count;
                 ++block_index)
            {
                MIR_Block* block = reverse_post_order.blocks[block_index];

                MIR_Instruction* instruction = block->first_instruction;
                while (instruction != NULL)
                {
                    MIR_Instruction* next_instruction = instruction->next_instruction;

                    // NOTE(vlad): Sanity check: there should be no PHI nodes left.
                    ASSERT(instruction->opcode != MIR_PHI);

                    if (instruction->opcode == MIR_MOVE)
                    {
                        ASSERT(instruction->definitions_count == 1);
                        ASSERT(instruction->uses_count == 1);

                        ASSERT(instruction->implicit_definitions_count == 0);
                        ASSERT(instruction->implicit_uses_count == 0);

                        const MIR_Operand* destination = &instruction->definitions[0];
                        const MIR_Operand* source = &instruction->uses[0];

                        ASSERT(destination->kind == MIR_OPERAND_VIRTUAL_REGISTER);
                        if (source->kind == MIR_OPERAND_VIRTUAL_REGISTER)
                        {
                            const Virtual_Register* destination_virtual_register
                                = &function->virtual_registers[destination->virtual_register_index];
                            const Virtual_Register* source_virtual_register
                                = &function->virtual_registers[source->virtual_register_index];

                            ASSERT(destination_virtual_register->assigned_physical_register != NO_REGISTER);
                            ASSERT(source_virtual_register->assigned_physical_register != NO_REGISTER);

                            // TODO(vlad): Remove these after stack slots support is added.
                            ASSERT(destination_virtual_register->assigned_physical_register != SPILLED_TO_STACK);
                            ASSERT(source_virtual_register->assigned_physical_register != SPILLED_TO_STACK);

                            if (destination_virtual_register->assigned_physical_register == source_virtual_register->assigned_physical_register)
                            {
                                MIR_Instruction* previous_instruction = instruction->previous_instruction;

                                // NOTE(vlad): MIR_MOVE cannot be the last instruction of the block.
                                ASSERT(next_instruction != NULL);

                                next_instruction->previous_instruction = previous_instruction;

                                if (previous_instruction == NULL)
                                {
                                    block->first_instruction = next_instruction;
                                }
                                else
                                {
                                    previous_instruction->next_instruction = next_instruction;
                                }
                            }
                        }
                    }

                    instruction = next_instruction;
                }
            }
        }

        request_arena_reset(context->arena_provider, context->scratch_arena);
    }
}

internal void
compute_layout_of_mir_blocks(Compilation_Context* context)
{
    MIR* mir = &context->mir;

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        MIR_Function* function = &mir->functions[function_index];

        // NOTE(vlad): Computing simple RPO as a linear layout of MIR blocks.
        {
            MIR_Reverse_Post_Order reverse_post_order
                = calculate_reverse_post_order_of_mir_blocks(context->mir_block_layouts_arena, function);

            function->blocks_layout_computed = true;
            function->blocks_layout.blocks = reverse_post_order.blocks;
            function->blocks_layout.blocks_count = reverse_post_order.blocks_count;
        }

        // NOTE(vlad): Marking sequential jumps as fall-through instructions.
        {
            MIR_Blocks_Layout* layout = &function->blocks_layout;

            for (Index block_index = 0;
                 block_index < layout->blocks_count - 1;
                 ++block_index)
            {
                MIR_Block* current_block = layout->blocks[block_index];
                MIR_Block* next_block = layout->blocks[block_index + 1];

                MIR_Instruction* last_instruction = current_block->last_instruction;
                if (last_instruction->opcode == MIR_JUMP)
                {
                    ASSERT(last_instruction->uses_count == 1);

                    MIR_Operand* destination_operand = &last_instruction->uses[0];
                    ASSERT(destination_operand->kind == MIR_OPERAND_BLOCK);

                    MIR_Block* destination_block = destination_operand->block;
                    if (destination_block == next_block)
                    {
                        // NOTE(vlad): This jump is redundant, removing it.

                        MIR_Instruction* previous_instruction = last_instruction->previous_instruction;
                        if (previous_instruction == NULL)
                        {
                            // NOTE(vlad): Block has no instructions left, making it empty.

                            ASSERT(current_block->first_instruction == last_instruction);

                            current_block->first_instruction = NULL;
                            current_block->last_instruction = NULL;
                        }
                        else
                        {
                            previous_instruction->next_instruction = NULL;
                            current_block->last_instruction = previous_instruction;
                        }
                    }
                }
            }
        }
    }
}

