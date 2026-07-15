#include "eon_cfg.h"

#include "eon_compilation_context.h"
#include "eon_lexical_scopes.h"
#include "eon_tac.h"

internal inline Cfg_Block_Id
create_cfg_block(Compilation_Context* context,
                 Tac_Function* tac_function,
                 const Tac_Instructions_Range instructions_range)
{
    append_array(context->cfg_blocks_arena, tac_function->cfg_blocks, Cfg_Block, (Cfg_Block){0});

    const Index this_block_index = tac_function->cfg_blocks_count - 1;

    Cfg_Block* block = &tac_function->cfg_blocks[this_block_index];

    block->edges_arena = acquire_arena_from_provider(context->arena_provider,
                                                     string_view("cfg-edges"),
                                                     GiB(1),
                                                     MiB(1));
    block->predecessors_arena = acquire_arena_from_provider(context->arena_provider,
                                                            string_view("cfg-predecessors"),
                                                            GiB(1),
                                                            MiB(1));
    block->dominance_frontier_arena = acquire_arena_from_provider(context->arena_provider,
                                                                  string_view("cfg-dominance-frontier"),
                                                                  GiB(1),
                                                                  MiB(1));

    block->phi_nodes_arena = acquire_arena_from_provider(context->arena_provider,
                                                         string_view("cfg-phi-nodes"),
                                                         GiB(1),
                                                         MiB(1));

    block->dominated_block_ids_arena = acquire_arena_from_provider(context->arena_provider,
                                                                   string_view("cfg-dominated-block-ids"),
                                                                   GiB(1),
                                                                   MiB(1));

    block->instructions_range = instructions_range;
    block->immediate_dominator_id.index = INVALID_IMMEDIATE_DOMINATOR_INDEX;

    Cfg_Block_Id id = {0};
    id.index = this_block_index;
    return id;
}

internal void
insert_edge(Cfg_Block* block, const Cfg_Block_Id new_block_id)
{
    Bool should_append_edge = true;
    for (Index edge_index = 0;
         edge_index < block->edges_count;
         ++edge_index)
    {
        const Cfg_Block_Id edge_id = block->edges[edge_index];
        if (edge_id.index == new_block_id.index)
        {
            should_append_edge = false;
            break;
        }
    }

    if (should_append_edge)
    {
        append_array(block->edges_arena,
                     block->edges,
                     Cfg_Block_Id,
                     new_block_id);
    }
}

internal Bool
remove_edge(Cfg_Block* block, const Cfg_Block_Id block_id)
{
    for (Index edge_index = 0;
         edge_index < block->edges_count;
         ++edge_index)
    {
        const Cfg_Block_Id edge_id = block->edges[edge_index];
        if (edge_id.index == block_id.index)
        {
            const Index last_edge_index = block->edges_count - 1;
            block->edges[edge_index] = block->edges[last_edge_index];
            block->edges_count -= 1;
            return true;
        }
    }

    return false;
}

internal void
insert_predecessor(Cfg_Block* block, const Cfg_Block_Id new_block_id)
{
    Bool should_append_predecessor = true;
    for (Index predecessor_index = 0;
         predecessor_index < block->predecessors_count;
         ++predecessor_index)
    {
        const Cfg_Block_Id predecessor_id = block->predecessors[predecessor_index];
        if (predecessor_id.index == new_block_id.index)
        {
            should_append_predecessor = false;
            break;
        }
    }

    if (should_append_predecessor)
    {
        append_array(block->predecessors_arena,
                     block->predecessors,
                     Cfg_Block_Id,
                     new_block_id);
    }
}

internal Bool
remove_predecessor(Cfg_Block* block, const Cfg_Block_Id block_id)
{
    for (Index predecessor_index = 0;
         predecessor_index < block->predecessors_count;
         ++predecessor_index)
    {
        const Cfg_Block_Id predecessor_id = block->predecessors[predecessor_index];
        if (predecessor_id.index == block_id.index)
        {
            const Index last_predecessor_index = block->predecessors_count - 1;
            block->predecessors[predecessor_index] = block->predecessors[last_predecessor_index];
            block->predecessors_count -= 1;

            for (Index phi_node_index = 0;
                 phi_node_index < block->phi_nodes_count;
                 ++phi_node_index)
            {
                Phi_Node* phi_node = &block->phi_nodes[phi_node_index];
                phi_node->previous_variables[predecessor_index] = phi_node->previous_variables[last_predecessor_index];
                phi_node->previous_variables_count -= 1;
            }

            return true;
        }
    }

    return false;
}

internal void
add_cfg_edge(Tac_Function* tac_function,
             const Cfg_Block_Id source_block_id,
             const Cfg_Block_Id destination_block_id)
{
    Cfg_Block* source_block = get_cfg_block_by_id(tac_function, source_block_id);
    Cfg_Block* destination_block = get_cfg_block_by_id(tac_function, destination_block_id);

    insert_edge(source_block, destination_block_id);
    insert_predecessor(destination_block, source_block_id);
}

internal inline void
add_cfg_fall_through_edge_if_needed(Tac_Function* tac_function, const Cfg_Block_Id source_block_id)
{
    const Cfg_Block* block = get_cfg_block_by_id(tac_function, source_block_id);

    const Index next_instruction_index = block->instructions_range.end_instruction_index;
    if (next_instruction_index == tac_function->instructions_count)
    {
        return;
    }

    for (Index successor_block_index = source_block_id.index + 1;
         successor_block_index < tac_function->cfg_blocks_count;
         ++successor_block_index)
    {
        const Cfg_Block* candidate_block = &tac_function->cfg_blocks[successor_block_index];
        const Tac_Instructions_Range* candidate_instructions_range = &candidate_block->instructions_range;

        if (candidate_instructions_range->start_instruction_index <= next_instruction_index
            && next_instruction_index < candidate_instructions_range->end_instruction_index)
        {
            Cfg_Block_Id destination_block_id = {0};
            destination_block_id.index = successor_block_index;

            add_cfg_edge(tac_function, source_block_id, destination_block_id);
            return;
        }
    }

    UNREACHABLE();
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

internal void
construct_cfg_from_tac(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    // NOTE(vlad): Creating basic blocks for TAC functions.

    const Size total_labels_count = tac->labels_count;
    if (total_labels_count > 0)
    {
        tac->label_index_to_cfg_block_id_map = allocate_array(context->tac_label_to_cfg_block_map_arena,
                                                              total_labels_count,
                                                              Cfg_Block_Id);
    }

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        Cfg_Block_Id entry_block_id = {0};
        entry_block_id.index = tac_function->cfg_blocks_count;

        {
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

                    create_cfg_block(context, tac_function, range);
                    block_start_instruction_index = instruction_index;
                }

                if (tac_operation_is_a_cfg_block_terminator(instruction->operation))
                {
                    Tac_Instructions_Range range = {0};
                    range.function_label_id = tac_function->label_id;
                    range.start_instruction_index = block_start_instruction_index;
                    range.end_instruction_index = instruction_index + 1;

                    create_cfg_block(context, tac_function, range);
                    block_start_instruction_index = instruction_index + 1;
                }
            }

            if (block_start_instruction_index < tac_function->instructions_count)
            {
                Tac_Instructions_Range range = {0};
                range.function_label_id = tac_function->label_id;
                range.start_instruction_index = block_start_instruction_index;
                range.end_instruction_index = tac_function->instructions_count;

                create_cfg_block(context, tac_function, range);
            }
        }

        {
            ASSERT(entry_block_id.index != tac_function->cfg_blocks_count);
            ASSERT(entry_block_id.index == ENTRY_BLOCK_INDEX);
        }

        // NOTE(vlad): Populating label_id to cfg_block_id map.

        for (Index block_index = 0;
             block_index < tac_function->cfg_blocks_count;
             ++block_index)
        {
            const Cfg_Block* block = &tac_function->cfg_blocks[block_index];

            const Tac_Instruction* instruction = &tac_function->instructions[block->instructions_range.start_instruction_index];
            if (instruction->operation == TAC_LABEL)
            {
                const Tac_Label_Id label_id = instruction->destination.label_id;
                ASSERT(0 < label_id.index && label_id.index <= total_labels_count);

                Cfg_Block_Id block_id = {0};
                block_id.index = block_index;

                ASSERT(tac->label_index_to_cfg_block_id_map[label_id.index].index == 0);
                tac->label_index_to_cfg_block_id_map[label_id.index] = block_id;
            }
        }

        // NOTE(vlad): Wiring CFG edges.

        for (Index this_block_index = 0;
             this_block_index < tac_function->cfg_blocks_count;
             ++this_block_index)
        {
            Cfg_Block* block = &tac_function->cfg_blocks[this_block_index];

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
                    const Cfg_Block_Id destination_block_id = tac->label_index_to_cfg_block_id_map[destination_label_id.index];

                    add_cfg_edge(tac_function, source_block_id, destination_block_id);
                } break;

                case TAC_JUMP_IF_TRUE:
                case TAC_JUMP_IF_FALSE:
                {
                    const Tac_Label_Id destination_label_id = last_instruction->destination.label_id;
                    const Cfg_Block_Id destination_block_id = tac->label_index_to_cfg_block_id_map[destination_label_id.index];

                    add_cfg_edge(tac_function, source_block_id, destination_block_id);
                    add_cfg_fall_through_edge_if_needed(tac_function, source_block_id);
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
                    add_cfg_fall_through_edge_if_needed(tac_function, source_block_id);
                } break;
            }
        }
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}

struct Cfg_Block_Reachability_Info
{
    Bool was_reached;
    Bool diagnostic_message_was_emitted;
};
typedef struct Cfg_Block_Reachability_Info Cfg_Block_Reachability_Info;

internal void
visit_reachable_blocks(Tac_Function* tac_function,
                       const Cfg_Block_Id this_block_id,
                       Cfg_Block_Reachability_Info* reachability_info)
{
    if (reachability_info[this_block_id.index].was_reached)
    {
        return;
    }

    reachability_info[this_block_id.index].was_reached = true;

    const Cfg_Block* this_block = get_cfg_block_by_id(tac_function, this_block_id);
    for (Index edge_index = 0;
         edge_index < this_block->edges_count;
         ++edge_index)
    {
        const Cfg_Block_Id reachable_block_id = this_block->edges[edge_index];
        visit_reachable_blocks(tac_function, reachable_block_id, reachability_info);
    }
}

internal void
emit_diagnostic_message_about_dead_code_if_needed(Compilation_Context* context,
                                                  Tac_Function* tac_function,
                                                  const Cfg_Block_Id this_block_id,
                                                  Cfg_Block_Reachability_Info* reachability_info)
{
    Cfg_Block_Reachability_Info* this_block_reachability_info = &reachability_info[this_block_id.index];

    if (this_block_reachability_info->was_reached)
    {
        return;
    }

    const Cfg_Block* block = get_cfg_block_by_id(tac_function, this_block_id);

    if (!this_block_reachability_info->diagnostic_message_was_emitted)
    {
        const Ast_Statement* statement = find_statement_by_tac_instructions_range(context, &block->instructions_range);

        // NOTE(vlad): 'statement' is NULL iff all instructions in the code block were inserted automatically during AST
        //             to TAC lowering (like implicit TAC_RETURN in void functions).
        if (statement)
        {
            Diagnostic_Message error = {0};
            error.level = MESSAGE_LEVEL_ERROR;
            error.location = statement->start_location;
            error.text = string_view("This code is unreachable");

            emit_diagnostic_message(context, &error);
        }
    }

    for (Index edge_index = 0;
         edge_index < block->edges_count;
         ++edge_index)
    {
        const Cfg_Block_Id reachable_block_id = block->edges[edge_index];

        Cfg_Block_Reachability_Info* next_block_reachability_info = &reachability_info[reachable_block_id.index];
        next_block_reachability_info->diagnostic_message_was_emitted = true;
        emit_diagnostic_message_about_dead_code_if_needed(context, tac_function, reachable_block_id, reachability_info);
    }
}

internal void
remove_unreachable_cfg_blocks(Compilation_Context* context)
{
    Tac* tac = &context->tac;
    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        // TODO(vlad): Reuse this memory?
        Cfg_Block_Reachability_Info* reachability_info = allocate_array(context->scratch_arena,
                                                                        tac_function->cfg_blocks_count,
                                                                        Cfg_Block_Reachability_Info);

        Cfg_Block_Id entry_cfg_block_id = {0};
        visit_reachable_blocks(tac_function, entry_cfg_block_id, reachability_info);

        Size unreachable_blocks_count = 0;
        for (Index this_block_index = 0;
             this_block_index < tac_function->cfg_blocks_count;
             ++this_block_index)
        {
            if (!reachability_info[this_block_index].was_reached)
            {
                unreachable_blocks_count += 1;

                Cfg_Block_Id this_block_id = {0};
                this_block_id.index = this_block_index;
                emit_diagnostic_message_about_dead_code_if_needed(context,
                                                                  tac_function,
                                                                  this_block_id,
                                                                  reachability_info);
            }
        }

        if (unreachable_blocks_count == 0)
        {
            continue;
        }

        // NOTE(vlad): Removing unreachabile blocks.
        {
            Index unreachable_block_index = -1;

            while (true)
            {
                for (Index i = unreachable_block_index + 1;
                     i < tac_function->cfg_blocks_count;
                     ++i)
                {
                    if (!reachability_info[i].was_reached)
                    {
                        unreachable_block_index = i;
                        break;
                    }
                }

                ASSERT(unreachable_block_index != -1);

                Index reachable_block_index = -1;
                for (Index i = unreachable_block_index + 1;
                     i < tac_function->cfg_blocks_count;
                     ++i)
                {
                    if (reachability_info[i].was_reached)
                    {
                        reachable_block_index = i;
                        break;
                    }
                }

                if (reachable_block_index == -1)
                {
                    break;
                }

                // NOTE(vlad): Swapping reachable block index to a new one.
                {
                    Cfg_Block_Id old_block_id = {0};
                    old_block_id.index = reachable_block_index;

                    Cfg_Block_Id new_block_id = {0};
                    new_block_id.index = unreachable_block_index;

                    for (Index block_index = 0;
                         block_index < tac_function->cfg_blocks_count;
                         ++block_index)
                    {
                        Cfg_Block_Id block_id = {0};
                        block_id.index = block_index;

                        Cfg_Block* block = get_cfg_block_by_id(tac_function, block_id);

                        // NOTE(vlad): Technically we should remove edges to an old block, but since it's unreachable
                        //             there are no such edges.
                        if (remove_edge(block, old_block_id))
                        {
                            insert_edge(block, new_block_id);
                        }

                        Index old_block_index_in_predecessors_list = -1;
                        Index new_block_index_in_predecessors_list = -1;
                        for (Index predecessor_index = 0;
                             predecessor_index < block->predecessors_count;
                             ++predecessor_index)
                        {
                            const Cfg_Block_Id predecessor_block_id = block->predecessors[predecessor_index];

                            if (predecessor_block_id.index == old_block_id.index)
                            {
                                old_block_index_in_predecessors_list = predecessor_index;
                            }

                            if (predecessor_block_id.index == new_block_id.index)
                            {
                                new_block_index_in_predecessors_list = predecessor_index;
                            }
                        }

                        if (old_block_index_in_predecessors_list != -1)
                        {
                            block->predecessors[old_block_index_in_predecessors_list] = new_block_id;
                        }

                        if (new_block_index_in_predecessors_list != -1)
                        {
                            remove_predecessor(block, new_block_id);
                        }

                        for (Index phi_node_index = 0;
                             phi_node_index < block->phi_nodes_count;
                             ++phi_node_index)
                        {
                            const Phi_Node* phi_node = &block->phi_nodes[phi_node_index];
                            ASSERT(phi_node->previous_variables_count == block->predecessors_count);
                        }
                    }

                    // TODO(vlad): Only process labels that belongs to the current TAC function.
                    for (Index label_index = INVALID_TAC_INDEX + 1;
                         label_index < tac->labels_count;
                         ++label_index)
                    {
                        Tac_Label_Id label_id = {0};
                        label_id.index = label_index;

                        const Tac_Label* label = get_tac_label_by_id(tac, label_id);
                        if (label->instruction_id.function_label_id.index != tac_function->label_id.index)
                        {
                            continue;
                        }

                        Cfg_Block_Id* linked_block_id = &tac->label_index_to_cfg_block_id_map[label_index];
                        if (linked_block_id->index == old_block_id.index)
                        {
                            linked_block_id->index = new_block_id.index;
                        }
                        else if (linked_block_id->index == new_block_id.index)
                        {
                            linked_block_id->index = INVALID_TAC_INDEX;
                        }
                    }
                }

                {
                    const Cfg_Block temp = tac_function->cfg_blocks[unreachable_block_index];
                    tac_function->cfg_blocks[unreachable_block_index] = tac_function->cfg_blocks[reachable_block_index];
                    tac_function->cfg_blocks[reachable_block_index] = temp;

                    reachability_info[unreachable_block_index].was_reached = true;
                    reachability_info[reachable_block_index].was_reached = false;
                }
            }

            for (Index i = unreachable_block_index;
                 i < tac_function->cfg_blocks_count;
                 ++i)
            {
                Cfg_Block* unreachable_block = &tac_function->cfg_blocks[i];
                free_cfg_block(context, unreachable_block);
            }
            tac_function->cfg_blocks_count = unreachable_block_index;
        }
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}

internal inline void
free_cfg_block(Compilation_Context* context, Cfg_Block* block)
{
    release_arena_to_provider(context->arena_provider, block->edges_arena);
    release_arena_to_provider(context->arena_provider, block->predecessors_arena);
    release_arena_to_provider(context->arena_provider, block->dominance_frontier_arena);
    release_arena_to_provider(context->arena_provider, block->phi_nodes_arena);
    release_arena_to_provider(context->arena_provider, block->dominated_block_ids_arena);
}

internal inline Cfg_Block*
get_cfg_block_by_id(Tac_Function* tac_function, const Cfg_Block_Id id)
{
    ASSERT(0 <= id.index && id.index < tac_function->cfg_blocks_count);
    return &tac_function->cfg_blocks[id.index];
}

internal inline Bool
cfg_block_is_empty(const Cfg_Block* block)
{
    const Tac_Instructions_Range* range = &block->instructions_range;
    ASSERT(range->start_instruction_index <= range->end_instruction_index);
    return range->start_instruction_index == range->end_instruction_index;
}
