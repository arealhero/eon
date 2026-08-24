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
create_virtual_register(Compilation_Context* context)
{
    MIR* mir = &context->mir;
    append_array(context->mir_operands_arena, mir->virtual_registers, Virtual_Register, (Virtual_Register){0});
    return &mir->virtual_registers[mir->virtual_registers_count - 1];
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

    // FIXME(vlad): Handle PHI nodes here.

    for (Index instruction_index = this_ssa_block->instructions_range.start_instruction_index;
         instruction_index < this_ssa_block->instructions_range.end_instruction_index;
         ++instruction_index)
    {
        const Tac_Instruction* ssa_instruction = &ssa_function->instructions[instruction_index];

        switch (ssa_instruction->operation)
        {
            case TAC_NOP:
            {
                // TODO(vlad): Should we emit these?
            } break;

            case TAC_ASSIGN:
            {
                MIR_Instruction* instruction = add_new_instruction_to_mir_block(context, this_block);
                instruction->opcode = MIR_MOVE;

                MIR_Operand* def = add_new_def_operand(instruction);

                MIR_Operand* use = add_new_use_operand(instruction);
                FAIL("[MIR] ASSIGN is not supported yet");
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
            {
                FAIL("[MIR] ADD is not supported yet");
            } break;

            case TAC_SUBTRACT:
            {
                FAIL("[MIR] SUBTRACT is not supported yet");
            } break;

            case TAC_MULTIPLY:
            {
                FAIL("[MIR] MULTIPLY is not supported yet");
            } break;

            case TAC_DIVIDE:
            {
                FAIL("[MIR] DIVIDE is not supported yet");
            } break;

            case TAC_EQUAL:
            {
                FAIL("[MIR] EQUAL is not supported yet");
            } break;

            case TAC_NOT_EQUAL:
            {
                FAIL("[MIR] NOT_EQUAL is not supported yet");
            } break;

            case TAC_LESS:
            {
                FAIL("[MIR] LESS is not supported yet");
            } break;

            case TAC_LESS_OR_EQUAL:
            {
                FAIL("[MIR] LESS_OR_EQUAL is not supported yet");
            } break;

            case TAC_GREATER:
            {
                FAIL("[MIR] GREATER is not supported yet");
            } break;

            case TAC_GREATER_OR_EQUAL:
            {
                FAIL("[MIR] GREATER_OR_EQUAL is not supported yet");
            } break;

            case TAC_LABEL:
            {
                FAIL("[MIR] LABEL is not supported yet");
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
                FAIL("[MIR] SET_PARAMETER is not supported yet");
            } break;

            case TAC_GET_PARAMETER:
            {
                FAIL("[MIR] GET_PARAMETER is not supported yet");
            } break;

            case TAC_CALL:
            {
                FAIL("[MIR] CALL is not supported yet");
            } break;

            case TAC_RETURN:
            {
                FAIL("[MIR] RETURN is not supported yet");
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
             predecessor_index < this_ssa_block->predecessors_count;
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
