#include "eon_cfg.h"

#include "eon_compilation_context.h"
#include "eon_tac.h"

internal inline Cfg_Block_Id
create_cfg_block(Compilation_Context* context,
                 const Tac_Instructions_Range instructions_range)
{
    Cfg* cfg = &context->cfg;
    append_array(context->cfg_blocks_arena, cfg->blocks, Cfg_Block, (Cfg_Block){0});

    Cfg_Block* block = &cfg->blocks[cfg->blocks_count - 1];
    block->instructions_range = instructions_range;

    Cfg_Block_Id id = {0};
    id.index = cfg->blocks_count - 1;
    return id;
}

internal inline void
add_cfg_edge(Compilation_Context* context,
             const Cfg_Block_Id source_block_id,
             const Cfg_Block_Id destination_block_id)
{
    ASSERT(source_block_id.index != INVALID_CFG_BLOCK_INDEX);
    ASSERT(destination_block_id.index != INVALID_CFG_BLOCK_INDEX);

    Cfg* cfg = &context->cfg;
    append_array(context->cfg_edges_arena, cfg->edges, Cfg_Edge, (Cfg_Edge){0});

    Cfg_Edge* edge = &cfg->edges[cfg->edges_count - 1];
    edge->source_block_id = source_block_id;
    edge->destination_block_id = destination_block_id;
}

internal inline void
add_cfg_fall_through_edge_if_needed(Compilation_Context* context, const Cfg_Block_Id source_block_id)
{
    ASSERT(source_block_id.index != INVALID_CFG_BLOCK_INDEX);

    Cfg* cfg = &context->cfg;

    const Cfg_Block* block = &cfg->blocks[source_block_id.index];
    const Tac_Function_Label_Id block_function_id = block->instructions_range.function_label_id;
    const Tac_Function* tac_function = get_tac_function_by_label(&context->tac, block_function_id);

    const Index next_instruction_index = block->instructions_range.end_instruction_index;
    if (next_instruction_index == tac_function->instructions_count)
    {
        return;
    }

    for (Index successor_block_index = source_block_id.index + 1;
         successor_block_index < cfg->blocks_count;
         ++successor_block_index)
    {
        const Cfg_Block* candidate_block = &cfg->blocks[successor_block_index];
        const Tac_Instructions_Range* candidate_instructions_range = &candidate_block->instructions_range;
        const Tac_Function_Label_Id candidate_block_function_id = candidate_instructions_range->function_label_id;

        ASSERT(candidate_block_function_id.index == block_function_id.index);

        if (candidate_instructions_range->start_instruction_index <= next_instruction_index
            && next_instruction_index < candidate_instructions_range->end_instruction_index)
        {
            Cfg_Block_Id destination_block_id = {0};
            destination_block_id.index = successor_block_index;

            add_cfg_edge(context, source_block_id, destination_block_id);
            return;
        }
    }

    UNREACHABLE();
}

internal inline Cfg_Block*
get_cfg_block_by_id(Cfg* cfg, const Cfg_Block_Id id)
{
    ASSERT(0 <= id.index && id.index < cfg->blocks_count);
    ASSERT(id.index != INVALID_CFG_BLOCK_INDEX);
    return &cfg->blocks[id.index];
}

internal inline Bool
tac_operation_is_a_cfg_block_terminator(const Tac_Operation operation)
{
    switch (operation)
    {
        case TAC_JUMP:
        case TAC_JUMP_IF_TRUE:
        case TAC_JUMP_IF_FALSE:
        case TAC_RETURN:
        {
            return true;
        } break;

        case TAC_NOP:
        case TAC_ASSIGN:
        case TAC_GET_ADDRESS:
        case TAC_LOAD_BY_ADDRESS:
        case TAC_STORE_BY_ADDRESS:
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
        case TAC_LABEL:
        case TAC_SET_PARAMETER:
        case TAC_GET_PARAMETER:
        case TAC_CALL:
        {
            return false;
        } break;
    }

    UNREACHABLE();
}

internal inline Bool
cfg_block_is_empty(const Cfg_Block* block)
{
    const Tac_Instructions_Range* range = &block->instructions_range;
    ASSERT(range->start_instruction_index <= range->end_instruction_index);
    return range->start_instruction_index == range->end_instruction_index;
}

internal void
construct_cfg_from_tac(Compilation_Context* context)
{
    Cfg* cfg = &context->cfg;
    Tac* tac = &context->tac;

    // NOTE(vlad): Creating a sentinel CFG block.
    {
        const Cfg_Block_Id id = create_cfg_block(context, (Tac_Instructions_Range){0});
        ASSERT(id.index == INVALID_CFG_BLOCK_INDEX);
    }

    // NOTE(vlad): Creating basic blocks for TAC functions.

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        const Tac_Function* tac_function = &tac->functions[function_index];

        Index block_start_instruction_index = 0;

        for (Index instruction_index = 0;
             instruction_index < tac_function->instructions_count;
             ++instruction_index)
        {
            const Tac_Instruction* instruction = &tac_function->instructions[instruction_index];

            if (instruction->operation == TAC_LABEL && instruction_index != block_start_instruction_index)
            {
              // NOTE(vlad): Closing current CFG block.

              Tac_Instructions_Range range = {0};
              range.function_label_id = tac_function->label_id;
              range.start_instruction_index = block_start_instruction_index;
              range.end_instruction_index = instruction_index;

              create_cfg_block(context, range);
              block_start_instruction_index = instruction_index;
            }

            if (tac_operation_is_a_cfg_block_terminator(instruction->operation))
            {
                Tac_Instructions_Range range = {0};
                range.function_label_id = tac_function->label_id;
                range.start_instruction_index = block_start_instruction_index;
                range.end_instruction_index = instruction_index + 1;

                create_cfg_block(context, range);
                block_start_instruction_index = instruction_index + 1;
            }
        }

        ASSERT(block_start_instruction_index == tac_function->instructions_count);
    }

    // NOTE(vlad): Populating label_id to cfg_block_id map.

    Cfg_Block_Id* label_index_to_cfg_block_id = NULL;

    // TODO(vlad): Map 'tac->function_labels_count' as well?
    const Size total_labels_count = tac->labels_count;
    if (total_labels_count > INVALID_TAC_INDEX + 1)
    {
        label_index_to_cfg_block_id = allocate_array(context->scratch_arena,
                                                  total_labels_count,
                                                  Cfg_Block_Id);
#if DEBUG_BUILD
        for (Index i = 0;
             i < total_labels_count;
             ++i)
        {
            ASSERT(label_index_to_cfg_block_id[i].index == INVALID_CFG_BLOCK_INDEX);
        }
#endif
    }

    for (Index block_index = INVALID_CFG_BLOCK_INDEX + 1;
         block_index < cfg->blocks_count;
         ++block_index)
    {
        const Cfg_Block* block = &cfg->blocks[block_index];
        const Tac_Function* tac_function = get_tac_function_by_label(tac, block->instructions_range.function_label_id);

        const Tac_Instruction* instruction = &tac_function->instructions[block->instructions_range.start_instruction_index];
        if (instruction->operation == TAC_LABEL)
        {
            const Tac_Label_Id label_id = instruction->destination.label_id;
            ASSERT(0 < label_id.index && label_id.index <= total_labels_count);

            Cfg_Block_Id block_id = {0};
            block_id.index = block_index;

            ASSERT(label_index_to_cfg_block_id[label_id.index].index == INVALID_CFG_BLOCK_INDEX);
            label_index_to_cfg_block_id[label_id.index] = block_id;
        }
    }

#if DEBUG_BUILD
    for (Index label_index = INVALID_TAC_INDEX + 1;
         label_index < total_labels_count;
         ++label_index)
    {
        ASSERT(label_index_to_cfg_block_id[label_index].index != INVALID_CFG_BLOCK_INDEX);
    }
#endif

    // NOTE(vlad): Wiring CFG edges.

    for (Index this_block_index = INVALID_CFG_BLOCK_INDEX + 1;
         this_block_index < cfg->blocks_count;
         ++this_block_index)
    {
        Cfg_Block* block = &cfg->blocks[this_block_index];
        const Tac_Function* tac_function = get_tac_function_by_label(tac, block->instructions_range.function_label_id);

        const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
        ASSERT(block->instructions_range.start_instruction_index <= last_instruction_index);

        const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];

        Cfg_Block_Id source_block_id = {0};
        source_block_id.index = this_block_index;

        switch (last_instruction->operation)
        {
            case TAC_JUMP:
            {
                const Tac_Label_Id destination_label_id = last_instruction->destination.label_id;
                const Cfg_Block_Id destination_block_id = label_index_to_cfg_block_id[destination_label_id.index];
                ASSERT(destination_block_id.index != INVALID_CFG_BLOCK_INDEX);

                add_cfg_edge(context, source_block_id, destination_block_id);
            } break;

            case TAC_JUMP_IF_TRUE:
            case TAC_JUMP_IF_FALSE:
            {
                const Tac_Label_Id destination_label_id = last_instruction->destination.label_id;
                const Cfg_Block_Id destination_block_id = label_index_to_cfg_block_id[destination_label_id.index];
                ASSERT(destination_block_id.index != INVALID_CFG_BLOCK_INDEX);

                add_cfg_edge(context, source_block_id, destination_block_id);
                add_cfg_fall_through_edge_if_needed(context, source_block_id);
            } break;

            case TAC_RETURN:
            {
                // NOTE(vlad): This block does not have successors.
            } break;

            case TAC_NOP:
            case TAC_ASSIGN:
            case TAC_GET_ADDRESS:
            case TAC_LOAD_BY_ADDRESS:
            case TAC_STORE_BY_ADDRESS:
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
            case TAC_LABEL:
            case TAC_SET_PARAMETER:
            case TAC_GET_PARAMETER:
            case TAC_CALL:
            {
                add_cfg_fall_through_edge_if_needed(context, source_block_id);
            } break;
        }
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}
