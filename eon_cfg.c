#include "eon_cfg.h"

#include "eon_compilation_context.h"
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
    block->instructions_range = instructions_range;

    Cfg_Block_Id id = {0};
    id.index = this_block_index;
    return id;
}

internal inline void
add_cfg_edge(Tac_Function* tac_function,
             const Cfg_Block_Id source_block_id,
             const Cfg_Block_Id destination_block_id)
{
    Cfg_Block* source_block = get_cfg_block_by_id(tac_function, source_block_id);

    append_array(source_block->edges_arena,
                 source_block->edges,
                 Cfg_Block_Id,
                 destination_block_id);
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

internal inline Cfg_Block*
get_cfg_block_by_id(Tac_Function* tac_function, const Cfg_Block_Id id)
{
    ASSERT(0 <= id.index && id.index < tac_function->cfg_blocks_count);
    return &tac_function->cfg_blocks[id.index];
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
    Tac* tac = &context->tac;

    // NOTE(vlad): Creating basic blocks for TAC functions.

    Cfg_Block_Id* label_index_to_cfg_block_id = NULL;

    const Size total_labels_count = tac->labels_count;
    if (total_labels_count > 0)
    {
        label_index_to_cfg_block_id = allocate_array(context->scratch_arena,
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

        ASSERT(block_start_instruction_index == tac_function->instructions_count);

        {
            ASSERT(entry_block_id.index != tac_function->cfg_blocks_count);
            ASSERT(entry_block_id.index == 0);
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

                ASSERT(label_index_to_cfg_block_id[label_id.index].index == 0);
                label_index_to_cfg_block_id[label_id.index] = block_id;
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
                    const Cfg_Block_Id destination_block_id = label_index_to_cfg_block_id[destination_label_id.index];

                    add_cfg_edge(tac_function, source_block_id, destination_block_id);
                } break;

                case TAC_JUMP_IF_TRUE:
                case TAC_JUMP_IF_FALSE:
                {
                    const Tac_Label_Id destination_label_id = last_instruction->destination.label_id;
                    const Cfg_Block_Id destination_block_id = label_index_to_cfg_block_id[destination_label_id.index];

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
swap_cfg_block_ids(Tac_Function* tac_function,
                   const Cfg_Block_Id old_id,
                   const Cfg_Block_Id new_id)
{
    for (Index block_index = 0;
         block_index < tac_function->cfg_blocks_count;
         ++block_index)
    {
        Cfg_Block* block = &tac_function->cfg_blocks[block_index];

        for (Index edge_index = 0;
             edge_index < block->edges_count;
             ++edge_index)
        {
            Cfg_Block_Id* reachable_block_id = &block->edges[edge_index];

            if (reachable_block_id->index == old_id.index)
            {
                *reachable_block_id = new_id;
            }
        }
    }
}

internal const Ast_Statement*
find_statement_in_code_block_by_tac_instruction_index(const Ast_Code_Block* code_block,
                                                      const Index tac_instruction_index)
{
    for (Index statement_index = 0;
         statement_index < code_block->statements_count;
         ++statement_index)
    {
        const Ast_Statement* statement = &code_block->statements[statement_index];
        const Tac_Instructions_Range* statement_range = &statement->tac_instructions_range;

        const Bool instruction_is_in_range = statement_range->start_instruction_index <= tac_instruction_index
                                             && tac_instruction_index < statement_range->end_instruction_index;

        if (!instruction_is_in_range)
        {
            continue;
        }

        switch (statement->kind)
        {
            case AST_STATEMENT_UNDEFINED:
            {
                UNREACHABLE();
            } break;

            case AST_STATEMENT_VARIABLE_DEFINITION:
            case AST_STATEMENT_ASSIGNMENT:
            case AST_STATEMENT_RETURN:
            case AST_STATEMENT_CALL:
            {
                return statement;
            } break;

            case AST_STATEMENT_WHILE:
            {
                const Ast_While_Statement* while_loop = &statement->while_statement;
                const Ast_Code_Block* loop_body = &while_loop->body;

                const Ast_Statement* found_statement_in_loop_body = find_statement_in_code_block_by_tac_instruction_index(loop_body,
                                                                                                                         tac_instruction_index);
                if (found_statement_in_loop_body)
                {
                    return found_statement_in_loop_body;
                }

                return statement;
            } break;

            case AST_STATEMENT_IF:
            {
                const Ast_If_Statement* if_loop = &statement->if_statement;

                const Ast_Code_Block* then_body = &if_loop->if_statements;
                const Ast_Statement* found_statement_in_then_body = find_statement_in_code_block_by_tac_instruction_index(then_body,
                                                                                                                         tac_instruction_index);
                if (found_statement_in_then_body)
                {
                    return found_statement_in_then_body;
                }

                const Ast_Code_Block* else_body = &if_loop->else_statements;
                const Ast_Statement* found_statement_in_else_body = find_statement_in_code_block_by_tac_instruction_index(else_body,
                                                                                                                         tac_instruction_index);
                if (found_statement_in_else_body)
                {
                    return found_statement_in_else_body;
                }

                return statement;
            } break;
        }

        UNREACHABLE();
    }

    UNREACHABLE();
}

internal const Ast_Statement*
find_statement_by_tac_instructions_range(Compilation_Context* context,
                                         const Tac_Instructions_Range* instructions_range)
{
    Tac* tac = &context->tac;

    const Tac_Function* tac_function = get_tac_function_by_label(tac, instructions_range->function_label_id);

    Index first_non_automatic_instruction_index = -1;
    for (Index instruction_index = instructions_range->start_instruction_index;
         instruction_index < instructions_range->end_instruction_index;
         ++instruction_index)
    {
        const Tac_Instruction* instruction = &tac_function->instructions[instruction_index];
        if (!instruction->was_automatically_inserted)
        {
            first_non_automatic_instruction_index = instruction_index;
            break;
        }
    }

    if (first_non_automatic_instruction_index == -1)
    {
        return NULL;
    }

    const Ast_Function_Definition* ast_function = tac_function->ast_function_definition;
    const Ast_Code_Block* body = &ast_function->body;

    const Ast_Statement* found_statement = find_statement_in_code_block_by_tac_instruction_index(body, first_non_automatic_instruction_index);
    ASSERT(found_statement != NULL);

    return found_statement;
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
        for (Index i = 0;
             i < tac_function->cfg_blocks_count;
             ++i)
        {
            if (!reachability_info[i].was_reached)
            {
                unreachable_blocks_count += 1;

                Cfg_Block_Id this_block_id = {0};
                this_block_id.index = i;
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
        Index next_free_block_index = -1;
        while (true)
        {
            for (Index i = next_free_block_index + 1;
                 i < tac_function->cfg_blocks_count;
                 ++i)
            {
                if (!reachability_info[i].was_reached)
                {
                    next_free_block_index = i;
                    break;
                }
            }

            ASSERT(next_free_block_index != -1);

            Index next_reachable_block_index = next_free_block_index + 1;
            for (;
                 next_reachable_block_index < tac_function->cfg_blocks_count;
                 next_reachable_block_index += 1)
            {
                if (reachability_info[next_reachable_block_index].was_reached)
                {
                    break;
                }
            }

            if (next_reachable_block_index == tac_function->cfg_blocks_count)
            {
                const Size reachable_blocks_count = tac_function->cfg_blocks_count - unreachable_blocks_count;

                for (Index block_index = reachable_blocks_count;
                     block_index < tac_function->cfg_blocks_count;
                     ++block_index)
                {
                    Cfg_Block* unreachable_block = &tac_function->cfg_blocks[block_index];
                    release_arena_to_provider(context->arena_provider, unreachable_block->edges_arena);
                }

                tac_function->cfg_blocks_count = reachable_blocks_count;
                break;
            }

            {
                Cfg_Block_Id old_block_id = {0};
                old_block_id.index = next_reachable_block_index;

                Cfg_Block_Id new_block_id = {0};
                new_block_id.index = next_free_block_index;

                swap_cfg_block_ids(tac_function, old_block_id, new_block_id);
            }

            tac_function->cfg_blocks[next_free_block_index] = tac_function->cfg_blocks[next_reachable_block_index];
            reachability_info[next_free_block_index].was_reached = true;
            reachability_info[next_reachable_block_index].was_reached = false;
        }
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}

internal void
compute_postorder_indices(Compilation_Context* context,
                          Tac_Function* tac_function,
                          Cfg_Block_Id* block_ids_in_postorder)
{
    struct Block_Info
    {
        Cfg_Block_Id id;
        Size next_unvisited_edge_index;
    };
    typedef struct Block_Info Block_Info;

    struct Traversal_Stack
    {
        stack(Block_Info, infos);
    };
    typedef struct Traversal_Stack Traversal_Stack;

    Traversal_Stack stack = {0};
    Bool* block_was_visited = allocate_array(context->scratch_arena, tac_function->cfg_blocks_count, Bool);

    Index postorder_index = 0;

    const Cfg_Block_Id entry_block_id = {0};

    {
        Block_Info entry_block_info = {0};
        entry_block_info.id = entry_block_id;
        entry_block_info.next_unvisited_edge_index = 0;
        stack_push(context->scratch_arena, stack.infos, Block_Info, entry_block_info);

        block_was_visited[entry_block_id.index] = true;
    }

    while (stack.infos_count > 0)
    {
        Block_Info* this_block_info = stack_top(stack.infos);

        Cfg_Block* this_block = get_cfg_block_by_id(tac_function, this_block_info->id);
        if (this_block_info->next_unvisited_edge_index < this_block->edges_count)
        {
            const Cfg_Block_Id next_block_id = this_block->edges[this_block_info->next_unvisited_edge_index++];

            if (!block_was_visited[next_block_id.index])
            {
                Block_Info next_block_info = {0};
                next_block_info.id = next_block_id;
                next_block_info.next_unvisited_edge_index = 0;

                stack_push(context->scratch_arena, stack.infos, Block_Info, next_block_info);
            }
        }
        else
        {
            block_ids_in_postorder[postorder_index] = this_block_info->id;
            this_block->postorder_index = postorder_index;
            postorder_index += 1;

            stack_pop(stack.infos);
        }
    }

    ASSERT(block_ids_in_postorder[postorder_index - 1].index == entry_block_id.index);
}

internal void
compute_cfg_dominators(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        // TODO(vlad): Reuse this memory?
        Cfg_Block_Id* block_ids_in_postorder = allocate_array(context->scratch_arena,
                                                              tac_function->cfg_blocks_count,
                                                              Cfg_Block_Id);
        compute_postorder_indices(context, tac_function, block_ids_in_postorder);

        const Cfg_Block_Id entry_block_id = {0};
        Cfg_Block* entry_block = get_cfg_block_by_id(tac_function, entry_block_id);

        // NOTE(vlad): Entry block dominates itself by definition.
        entry_block->immediate_dominator_id = entry_block_id;

        Bool dominator_has_changed = true;
        while (dominator_has_changed)
        {
            dominator_has_changed = false;

            // for (Index block_index = cfg->blocks_)
        }
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}
