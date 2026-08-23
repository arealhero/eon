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
                       const Cfg_Block_Id ssa_block_id,
                       MIR_Block** mir_blocks_map)
{
    if (mir_blocks_map[ssa_block_id.index])
    {
        return mir_blocks_map[ssa_block_id.index];
    }

    Cfg_Block* ssa_block = get_cfg_block_by_id(ssa_function, ssa_block_id);
    ASSERT(ssa_block != NULL);

    MIR_Block* block = create_new_mir_block(context);
    block->successors = allocate_array(context->mir_edges_arena, ssa_block->edges_count, MIR_Block*);
    block->successors_count = ssa_block->edges_count;

    block->predecessors = allocate_array(context->mir_edges_arena, ssa_block->predecessors_count, MIR_Block*);
    block->predecessors_count = ssa_block->predecessors_count;

    for (Index successor_index = 0;
         successor_index < block->successors_count;
         ++successor_index)
    {
    }

    return block;
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
