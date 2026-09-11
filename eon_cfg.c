#include "eon_cfg.h"

#include "eon_compilation_context.h"
#include "eon_tac.h"

#include <eon/build_info.h>

internal inline Cfg_Block_Id
create_cfg_block(Compilation_Context* context,
                 Tac_Function* tac_function,
                 Tac_Instruction* first_instruction,
                 Tac_Instruction* last_instruction)
{
    ASSERT(first_instruction != NULL);
    ASSERT(last_instruction != NULL);

    ASSERT(first_instruction->previous_instruction == NULL);

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

    block->first_tac_instruction = first_instruction;
    block->last_tac_instruction = last_instruction;
    block->next_after_last_tac_instruction = last_instruction->next_instruction;

    // NOTE(vlad): We break instructions connectivity here so every CFG block's last_instruction becomes last
    //             in the sense that 'last_instruction->next_instruction' is NULL.
    if (last_instruction->next_instruction != NULL)
    {
        Tac_Instruction* next_instruction = last_instruction->next_instruction;
        last_instruction->next_instruction = NULL;
        next_instruction->previous_instruction = NULL;
    }

    block->immediate_dominator_id.index = INVALID_CFG_BLOCK_INDEX;

    Cfg_Block_Id id = {0};
    id.index = this_block_index;
    return id;
}

internal inline Bool
cfg_block_ids_are_equal(const Cfg_Block_Id lhs, const Cfg_Block_Id rhs)
{
    return lhs.index == rhs.index;
}

// FIXME(vlad): Move to 'eon_tac.h' or to 'eon_forward_declarations.h'.
internal inline Bool
tac_label_ids_are_equal(const Tac_Label_Id lhs, const Tac_Label_Id rhs)
{
    return lhs.index == rhs.index;
}

// FIXME(vlad): Move to 'eon_tac.h' or to 'eon_forward_declarations.h'.
internal inline Bool
tac_function_label_ids_are_equal(const Tac_Function_Label_Id lhs, const Tac_Function_Label_Id rhs)
{
    return lhs.index == rhs.index;
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
            remove_last_array_element(block->edges, Cfg_Block_Id);
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

        ASSERT(block->phi_nodes_count == 0);
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
            remove_last_array_element(block->predecessors, Cfg_Block_Id);

            for (Index phi_node_index = 0;
                 phi_node_index < block->phi_nodes_count;
                 ++phi_node_index)
            {
                Phi_Node* phi_node = &block->phi_nodes[phi_node_index];
                phi_node->previous_variables[predecessor_index] = phi_node->previous_variables[last_predecessor_index];
                remove_last_array_element(phi_node->previous_variables, Tac_Variable_Id);
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
add_cfg_fall_through_edge_if_needed(Compilation_Context* context,
                                    Tac_Function* tac_function,
                                    const Cfg_Block_Id source_block_id)
{
    Cfg_Block* source_block = get_cfg_block_by_id(tac_function, source_block_id);

    ASSERT(source_block->last_tac_instruction != NULL);
    Tac_Instruction* next_instruction = source_block->next_after_last_tac_instruction;
    if (next_instruction == NULL)
    {
        return;
    }

    for (Index successor_block_index = source_block_id.index + 1;
         successor_block_index < tac_function->cfg_blocks_count;
         ++successor_block_index)
    {
        Cfg_Block* candidate_block = &tac_function->cfg_blocks[successor_block_index];

        if (candidate_block->first_tac_instruction == next_instruction)
        {
            Cfg_Block_Id destination_block_id = {0};
            destination_block_id.index = successor_block_index;

            // NOTE(vlad): Inserting explicit jump and label.
            {
                Tac_Label_Id label_id = {0};

                if (candidate_block->first_tac_instruction->operation == TAC_LABEL)
                {
                    ASSERT(candidate_block->first_tac_instruction->destination.kind == TAC_OPERAND_LABEL);
                    label_id = candidate_block->first_tac_instruction->destination.label_id;
                }
                else
                {
                    label_id = create_tac_label(context);

                    Tac_Instruction* label = allocate(context->tac_instructions_arena, Tac_Instruction);
                    label->was_automatically_inserted = true;
                    label->next_instruction = candidate_block->first_tac_instruction;
                    candidate_block->first_tac_instruction->previous_instruction = label;
                    candidate_block->first_tac_instruction = label;

                    label->operation = TAC_LABEL;
                    label->destination.kind = TAC_OPERAND_LABEL;
                    label->destination.label_id = label_id;

                    append_array(context->tac_label_to_cfg_block_map_arena,
                                 context->tac.label_index_to_cfg_block_id_map,
                                 Cfg_Block_Id,
                                 destination_block_id);
                }

                ASSERT(label_id.index != INVALID_TAC_INDEX);

                Tac_Instruction* jump = allocate(context->tac_instructions_arena, Tac_Instruction);
                jump->was_automatically_inserted = true;
                jump->previous_instruction = source_block->last_tac_instruction;
                source_block->last_tac_instruction->next_instruction = jump;
                source_block->last_tac_instruction = jump;

                jump->operation = TAC_JUMP;
                jump->destination.kind = TAC_OPERAND_LABEL;
                jump->destination.label_id = label_id;

                source_block->next_after_last_tac_instruction = candidate_block->first_tac_instruction;
            }

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
        ensure_array_has_enough_capacity(context->tac_label_to_cfg_block_map_arena,
                                         tac->label_index_to_cfg_block_id_map,
                                         Cfg_Block_Id,
                                         total_labels_count);

        Cfg_Block_Id invalid_block_id = {0};
        invalid_block_id.index = INVALID_CFG_BLOCK_INDEX;

        for (Index label_index = 0;
             label_index < total_labels_count;
             ++label_index)
        {
            append_array(context->tac_label_to_cfg_block_map_arena,
                         tac->label_index_to_cfg_block_id_map,
                         Cfg_Block_Id,
                         invalid_block_id);
        }
    }

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        Cfg_Block_Id entry_block_id = {0};
        entry_block_id.index = tac_function->cfg_blocks_count;

        {
            Tac_Instruction* current_instruction = tac_function->first_instruction;
            Tac_Instruction* block_start_instruction = current_instruction;

            while (current_instruction != NULL)
            {
                if (current_instruction->operation == TAC_LABEL && current_instruction != block_start_instruction)
                {
                    // NOTE(vlad): Closing current CFG block.
                    create_cfg_block(context, tac_function, block_start_instruction, current_instruction->previous_instruction);
                    block_start_instruction = current_instruction;
                    current_instruction = current_instruction->next_instruction;
                }
                else if (tac_operation_is_a_cfg_block_terminator(current_instruction->operation))
                {
                    Tac_Instruction* next_instruction = current_instruction->next_instruction;
                    create_cfg_block(context, tac_function, block_start_instruction, current_instruction);
                    block_start_instruction = next_instruction;
                    current_instruction = next_instruction;
                }
                else
                {
                    current_instruction = current_instruction->next_instruction;
                }
            }

            if (block_start_instruction != NULL)
            {
                create_cfg_block(context, tac_function, block_start_instruction, tac_function->last_instruction);
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

            ASSERT(block->first_tac_instruction != NULL);

            const Tac_Instruction* instruction = block->first_tac_instruction;
            if (instruction->operation == TAC_LABEL)
            {
                const Tac_Label_Id label_id = instruction->destination.label_id;
                ASSERT(INVALID_TAC_INDEX < label_id.index && label_id.index <= total_labels_count);

                Cfg_Block_Id block_id = {0};
                block_id.index = block_index;

                ASSERT(tac->label_index_to_cfg_block_id_map[label_id.index].index == INVALID_CFG_BLOCK_INDEX);
                tac->label_index_to_cfg_block_id_map[label_id.index] = block_id;
            }
        }

        // NOTE(vlad): Wiring CFG edges.

        for (Index this_block_index = 0;
             this_block_index < tac_function->cfg_blocks_count;
             ++this_block_index)
        {
            Cfg_Block* block = &tac_function->cfg_blocks[this_block_index];

            const Tac_Instruction* last_instruction = block->last_tac_instruction;
            ASSERT(last_instruction != NULL);

            Cfg_Block_Id source_block_id = {0};
            source_block_id.index = this_block_index;

            switch (last_instruction->operation)
            {
                case TAC_JUMP:
                {
                    const Tac_Label_Id destination_label_id = last_instruction->destination.label_id;
                    const Cfg_Block_Id destination_block_id = tac->label_index_to_cfg_block_id_map[destination_label_id.index];
                    ASSERT(destination_block_id.index != INVALID_CFG_BLOCK_INDEX);

                    add_cfg_edge(tac_function, source_block_id, destination_block_id);
                } break;

                case TAC_JUMP_IF_FALSE:
                {
                    const Tac_Label_Id destination_label_id = last_instruction->destination.label_id;
                    const Cfg_Block_Id destination_block_id = tac->label_index_to_cfg_block_id_map[destination_label_id.index];
                    ASSERT(destination_block_id.index != INVALID_CFG_BLOCK_INDEX);

                    add_cfg_edge(tac_function, source_block_id, destination_block_id);
                    add_cfg_fall_through_edge_if_needed(context, tac_function, source_block_id);
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
                    // TODO(vlad): Should this be unreachable?
                    add_cfg_fall_through_edge_if_needed(context, tac_function, source_block_id);
                } break;
            }
        }

        // NOTE(vlad): We should not use first_instruction and last_instruction after CFG construction.
        tac_function->first_instruction = NULL;
        tac_function->last_instruction = NULL;
    }

    for (Index label_index = INVALID_TAC_INDEX + 1;
         label_index < total_labels_count;
         ++label_index)
    {
        ASSERT(tac->label_index_to_cfg_block_id_map[label_index].index != INVALID_CFG_BLOCK_INDEX);
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
        const Ast_Statement* statement = NULL;
        for (const Tac_Instruction* current_instruction = block->first_tac_instruction;
             current_instruction != NULL;
             current_instruction = current_instruction->next_instruction)
        {
            if (!current_instruction->was_automatically_inserted)
            {
                ASSERT(current_instruction->ast_statement != NULL);
                statement = current_instruction->ast_statement;
                break;
            }
        }

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

        this_block_reachability_info->diagnostic_message_was_emitted = true;
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
remove_last_cfg_block(Compilation_Context* context, Tac_Function* tac_function)
{
    ASSERT(tac_function->cfg_blocks_count != 0);

    Cfg_Block_Id last_block_id = {0};
    last_block_id.index = tac_function->cfg_blocks_count - 1;

    Cfg_Block* last_block = get_cfg_block_by_id(tac_function, last_block_id);

    for (Index edge_index = 0;
         edge_index < last_block->edges_count;
         ++edge_index)
    {
        const Cfg_Block_Id successor_id = last_block->edges[edge_index];

        Cfg_Block* successor = get_cfg_block_by_id(tac_function, successor_id);
        remove_predecessor(successor, last_block_id);
    }

    for (Index predecessor_index = 0;
         predecessor_index < last_block->predecessors_count;
         ++predecessor_index)
    {
        const Cfg_Block_Id predecessor_id = last_block->predecessors[predecessor_index];

        Cfg_Block* predecessor = get_cfg_block_by_id(tac_function, predecessor_id);
        remove_edge(predecessor, last_block_id);
    }

    Tac* tac = &context->tac;

    ASSERT(tac->label_index_to_cfg_block_id_map != NULL);

    for (Index label_index = tac_function->first_tac_label_index;
         label_index < tac_function->last_tac_label_index;
         ++label_index)
    {
        Cfg_Block_Id* block_id = &tac->label_index_to_cfg_block_id_map[label_index];

        if (cfg_block_ids_are_equal(*block_id, last_block_id))
        {
            block_id->index = INVALID_CFG_BLOCK_INDEX;
            // TODO(vlad): Can we break here?
        }
    }

    free_cfg_block(context, last_block);

    remove_last_array_element(tac_function->cfg_blocks, Cfg_Block);
}

internal void
swap_cfg_blocks(Tac* tac,
                Tac_Function* tac_function,
                const Cfg_Block_Id first_block_id,
                const Cfg_Block_Id second_block_id)
{
    Cfg_Block* first_block = get_cfg_block_by_id(tac_function, first_block_id);
    Cfg_Block* second_block = get_cfg_block_by_id(tac_function, second_block_id);

    // FIXME(vlad): Use first and second block edges and predecessors instead of traversing every block.
    for (Index block_index = 0;
         block_index < tac_function->cfg_blocks_count;
         ++block_index)
    {
        Cfg_Block_Id block_id = {0};
        block_id.index = block_index;

        Cfg_Block* block = get_cfg_block_by_id(tac_function, block_id);

        // NOTE(vlad): Renaming edges.
        {
            Index index_of_first_block_in_edges = -1;
            Index index_of_second_block_in_edges = -1;

            for (Index edge_index = 0;
                 edge_index < block->edges_count;
                 ++edge_index)
            {
                const Cfg_Block_Id edge = block->edges[edge_index];

                if (cfg_block_ids_are_equal(edge, first_block_id))
                {
                    ASSERT(index_of_first_block_in_edges == -1);
                    index_of_first_block_in_edges = edge_index;
                }

                if (cfg_block_ids_are_equal(edge, second_block_id))
                {
                    ASSERT(index_of_second_block_in_edges == -1);
                    index_of_second_block_in_edges = edge_index;
                }
            }

            if (index_of_first_block_in_edges != -1)
            {
                block->edges[index_of_first_block_in_edges] = second_block_id;
            }

            if (index_of_second_block_in_edges != -1)
            {
                block->edges[index_of_second_block_in_edges] = first_block_id;
            }
        }

        // NOTE(vlad): Renaming predecessors.
        {
            Index index_of_first_block_in_predecessors = -1;
            Index index_of_second_block_in_predecessors = -1;

            for (Index predecessor_index = 0;
                 predecessor_index < block->predecessors_count;
                 ++predecessor_index)
            {
                const Cfg_Block_Id predecessor = block->predecessors[predecessor_index];

                if (cfg_block_ids_are_equal(predecessor, first_block_id))
                {
                    ASSERT(index_of_first_block_in_predecessors == -1);
                    index_of_first_block_in_predecessors = predecessor_index;
                }

                if (cfg_block_ids_are_equal(predecessor, second_block_id))
                {
                    ASSERT(index_of_second_block_in_predecessors == -1);
                    index_of_second_block_in_predecessors = predecessor_index;
                }
            }

            if (index_of_first_block_in_predecessors != -1)
            {
                block->predecessors[index_of_first_block_in_predecessors] = second_block_id;
            }

            if (index_of_second_block_in_predecessors != -1)
            {
                block->predecessors[index_of_second_block_in_predecessors] = first_block_id;
            }
        }
    }

    ASSERT(tac->label_index_to_cfg_block_id_map != NULL);

    for (Index label_index = tac_function->first_tac_label_index;
         label_index < tac_function->last_tac_label_index;
         ++label_index)
    {
        Cfg_Block_Id* linked_block_id = &tac->label_index_to_cfg_block_id_map[label_index];
        if (cfg_block_ids_are_equal(*linked_block_id, first_block_id))
        {
            *linked_block_id = second_block_id;
        }
        else if (cfg_block_ids_are_equal(*linked_block_id, second_block_id))
        {
            *linked_block_id = first_block_id;
        }
    }

    const Cfg_Block temp = *first_block;
    *first_block = *second_block;
    *second_block = temp;
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

                Cfg_Block_Id unreachable_block_id = {0};
                unreachable_block_id.index = this_block_index;
                emit_diagnostic_message_about_dead_code_if_needed(context,
                                                                  tac_function,
                                                                  unreachable_block_id,
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

                    swap_cfg_blocks(tac, tac_function, old_block_id, new_block_id);
                }

                reachability_info[unreachable_block_index].was_reached = true;
                reachability_info[reachable_block_index].was_reached = false;
            }

            ASSERT(tac_function->cfg_blocks_count - unreachable_block_index == unreachable_blocks_count);

            for (Index i = 0;
                 i < unreachable_blocks_count;
                 ++i)
            {
                remove_last_cfg_block(context, tac_function);
            }
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
