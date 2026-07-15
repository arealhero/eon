#include "eon_ssa.h"

#include "eon_cfg.h"
#include "eon_compilation_context.h"
#include "eon_lexical_scopes.h"
#include "eon_tac.h"

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
                block_was_visited[next_block_id.index] = true;

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

        // NOTE(vlad): Entry block must be last in postorder traversal.
        ASSERT(entry_block->postorder_index == tac_function->cfg_blocks_count - 1);

        // NOTE(vlad): Entry block dominates itself by definition.
        entry_block->immediate_dominator_id = entry_block_id;

        Bool dominator_has_changed = true;
        while (dominator_has_changed)
        {
            dominator_has_changed = false;

            for (Index postorder_index = tac_function->cfg_blocks_count - 2;
                 postorder_index >= 0;
                 --postorder_index)
            {
                const Cfg_Block_Id this_block_id = block_ids_in_postorder[postorder_index];
                Cfg_Block* this_block = get_cfg_block_by_id(tac_function, this_block_id);

                Cfg_Block* new_immediate_dominator = NULL;
                Index new_immediate_dominator_index = -1;

                Index predecessor_index = 0;
                for (;
                     predecessor_index < this_block->predecessors_count;
                     ++predecessor_index)
                {
                    const Cfg_Block_Id predecessor_block_id = this_block->predecessors[predecessor_index];
                    Cfg_Block* predecessor_block = get_cfg_block_by_id(tac_function, predecessor_block_id);

                    if (predecessor_block->immediate_dominator_id.index != INVALID_IMMEDIATE_DOMINATOR_INDEX)
                    {
                        new_immediate_dominator = predecessor_block;
                        new_immediate_dominator_index = predecessor_block_id.index;
                        break;
                    }
                }

                // NOTE(vlad): We should be able to find new immediate dominator in connected CFGs.
                //             If this CFG is not connected then we failed to remove all unreachable blocks.
                ASSERT(new_immediate_dominator != NULL);

                // NOTE(vlad): Intersecting with all other processed predecessors.

                predecessor_index += 1;

                for (;
                     predecessor_index < this_block->predecessors_count;
                     ++predecessor_index)
                {
                    const Cfg_Block_Id predecessor_block_id = this_block->predecessors[predecessor_index];
                    Cfg_Block* predecessor_block = get_cfg_block_by_id(tac_function, predecessor_block_id);

                    if (predecessor_block->immediate_dominator_id.index != INVALID_IMMEDIATE_DOMINATOR_INDEX)
                    {
                        // NOTE(vlad): Intersecting 'predecessor_block' and 'new_immediate_dominator'.
                        Cfg_Block* finger1 = predecessor_block;
                        Index finger1_index = predecessor_block_id.index;

                        Cfg_Block* finger2 = new_immediate_dominator;

                        while (finger1 != finger2)
                        {
                            while (finger1->postorder_index < finger2->postorder_index)
                            {
                                finger1_index = finger1->immediate_dominator_id.index;
                                finger1 = get_cfg_block_by_id(tac_function, finger1->immediate_dominator_id);
                            }

                            while (finger2->postorder_index < finger1->postorder_index)
                            {
                                finger2 = get_cfg_block_by_id(tac_function, finger2->immediate_dominator_id);
                            }
                        }

                        new_immediate_dominator = finger1;
                        new_immediate_dominator_index = finger1_index;
                    }
                }

                if (this_block->immediate_dominator_id.index != new_immediate_dominator_index)
                {
                    this_block->immediate_dominator_id.index = new_immediate_dominator_index;
                    dominator_has_changed = true;
                }
            }
        }
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}

internal void
compute_cfg_dominance_frontiers(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        for (Index block_index = 0;
             block_index < tac_function->cfg_blocks_count;
             ++block_index)
        {
            Cfg_Block_Id this_block_id = {0};
            this_block_id.index = block_index;

            Cfg_Block* this_block = get_cfg_block_by_id(tac_function, this_block_id);

            if (this_block->predecessors_count < 2)
            {
                continue;
            }

            for (Index predecessor_index = 0;
                 predecessor_index < this_block->predecessors_count;
                 ++predecessor_index)
            {
                Cfg_Block_Id runner_id = this_block->predecessors[predecessor_index];

                while (runner_id.index != this_block->immediate_dominator_id.index)
                {
                    Cfg_Block* runner = get_cfg_block_by_id(tac_function, runner_id);

                    // NOTE(vlad): Adding 'runner' to this block's dominance frontier.
                    {
                        Bool should_add_this_block_to_dominance_frontier = true;

                        for (Index frontier_index = 0;
                             frontier_index < runner->dominance_frontier_count;
                             ++frontier_index)
                        {
                            const Cfg_Block_Id frontier_element_id = runner->dominance_frontier[frontier_index];

                            if (frontier_element_id.index == this_block_id.index)
                            {
                                should_add_this_block_to_dominance_frontier = false;
                                break;
                            }
                        }

                        if (should_add_this_block_to_dominance_frontier)
                        {
                            append_array(runner->dominance_frontier_arena,
                                         runner->dominance_frontier,
                                         Cfg_Block_Id,
                                         this_block_id);
                        }
                    }

                    runner_id = runner->immediate_dominator_id;
                }
            }
        }
    }
}

internal void
insert_phi_nodes(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        for (Index variable_index = tac_function->first_tac_variable_index;
             variable_index < tac_function->last_tac_variable_index;
             ++variable_index)
        {
            Tac_Variable_Id this_variable_id = {0};
            this_variable_id.index = variable_index;

            {
                Tac_Variable* this_variable = get_tac_variable_by_id(tac, this_variable_id);
                if (this_variable->is_temporary)
                {
                    continue;
                }
            }

            struct Block_Ids_Stack
            {
                stack(Cfg_Block_Id, ids);
            };
            typedef struct Block_Ids_Stack Block_Ids_Stack;

            Block_Ids_Stack blocks_that_need_phi_nodes = {0};
            ensure_array_has_enough_capacity(context->scratch_arena,
                                             blocks_that_need_phi_nodes.ids,
                                             Cfg_Block_Id,
                                             tac_function->cfg_blocks_count);

            for (Index block_index = 0;
                 block_index < tac_function->cfg_blocks_count;
                 ++block_index)
            {
                Cfg_Block_Id block_id = {0};
                block_id.index = block_index;

                Cfg_Block* block = get_cfg_block_by_id(tac_function, block_id);
                const Tac_Instructions_Range* instructions_range = &block->instructions_range;

                ASSERT(instructions_range->function_label_id.index == tac_function->label_id.index);

                for (Index instruction_index = instructions_range->start_instruction_index;
                     instruction_index < instructions_range->end_instruction_index;
                     ++instruction_index)
                {
                    const Tac_Instruction* instruction = &tac_function->instructions[instruction_index];

                    if (instruction->destination.kind == TAC_OPERAND_VARIABLE
                        && instruction->destination.variable_id.index == variable_index)
                    {
                        stack_push(context->scratch_arena, blocks_that_need_phi_nodes.ids, Cfg_Block_Id, block_id);
                        break;
                    }
                }
            }

            Bool* block_has_phi_node_for_this_variable = allocate_array(context->scratch_arena,
                                                                        tac_function->cfg_blocks_count,
                                                                        Bool);

            while (blocks_that_need_phi_nodes.ids_count > 0)
            {
                const Cfg_Block_Id this_block_id = *stack_top(blocks_that_need_phi_nodes.ids);
                stack_pop(blocks_that_need_phi_nodes.ids);

                Cfg_Block* this_block = get_cfg_block_by_id(tac_function, this_block_id);

                for (Index frontier_index = 0;
                     frontier_index < this_block->dominance_frontier_count;
                     ++frontier_index)
                {
                    const Cfg_Block_Id frontier_block_id = this_block->dominance_frontier[frontier_index];

                    if (block_has_phi_node_for_this_variable[frontier_block_id.index])
                    {
                        continue;
                    }

                    Cfg_Block* frontier_block = get_cfg_block_by_id(tac_function, frontier_block_id);

                    Phi_Node phi_node = {0};
                    phi_node.destination = this_variable_id;
                    phi_node.previous_variables = allocate_array(context->phi_node_arguments_arena,
                                                                 frontier_block->predecessors_count,
                                                                 Tac_Variable_Id);
                    phi_node.previous_variables_count = frontier_block->predecessors_count;

                    for (Index previous_variable_index = 0;
                         previous_variable_index < phi_node.previous_variables_count;
                         ++previous_variable_index)
                    {
                        phi_node.previous_variables[previous_variable_index].ssa_version = SSA_VERSION_UNSET;
                    }

                    append_array(frontier_block->phi_nodes_arena, frontier_block->phi_nodes, Phi_Node, phi_node);

                    block_has_phi_node_for_this_variable[frontier_block_id.index] = true;

                    stack_push(context->scratch_arena, blocks_that_need_phi_nodes.ids, Cfg_Block_Id, frontier_block_id);
                }
            }
        }
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}

internal void
build_dominator_tree(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        for (Index block_index = ENTRY_BLOCK_INDEX + 1;
             block_index < tac_function->cfg_blocks_count;
             ++block_index)
        {
            Cfg_Block_Id block_id = {0};
            block_id.index = block_index;

            Cfg_Block* block = get_cfg_block_by_id(tac_function, block_id);

            if (block->immediate_dominator_id.index != INVALID_IMMEDIATE_DOMINATOR_INDEX)
            {
                Cfg_Block* immediate_dominator_block = get_cfg_block_by_id(tac_function, block->immediate_dominator_id);
                append_array(immediate_dominator_block->dominated_block_ids_arena,
                             immediate_dominator_block->dominated_block_ids,
                             Cfg_Block_Id,
                             block_id);
            }
        }
    }
}

struct Tac_Variable_Renaming_Info
{
    Arena* versions_arena;

    Index next_version;
    stack(Index, versions);
};
typedef struct Tac_Variable_Renaming_Info Tac_Variable_Renaming_Info;

struct Tac_Renaming_Info
{
    Tac_Variable_Renaming_Info* variables;
};
typedef struct Tac_Renaming_Info Tac_Renaming_Info;

internal Index
push_new_tac_variable_version(Tac_Renaming_Info* info, const Tac_Variable_Id variable_id)
{
    ASSERT(variable_id.index != INVALID_TAC_INDEX);

    Tac_Variable_Renaming_Info* variable_info = &info->variables[variable_id.index];

    const Index version = ++variable_info->next_version;
    stack_push(variable_info->versions_arena, variable_info->versions, Index, version);
    return version;
}

internal Index
get_tac_variable_version(Tac_Renaming_Info* info, const Tac_Variable_Id variable_id)
{
    ASSERT(variable_id.index != INVALID_TAC_INDEX);

    Tac_Variable_Renaming_Info* variable_info = &info->variables[variable_id.index];
    if (variable_info->versions_count == 0)
    {
        return SSA_VERSION_UNDEFINED;
    }

    return *stack_top(variable_info->versions);
}

internal void
pop_tac_variable_version(Tac_Renaming_Info* info, const Tac_Variable_Id variable_id)
{
    ASSERT(variable_id.index != INVALID_TAC_INDEX);

    Tac_Variable_Renaming_Info* variable_info = &info->variables[variable_id.index];

    ASSERT(variable_info->versions_count != 0);
    stack_pop(variable_info->versions);
}

internal void
set_tac_variable_versions_in_cfg_block(Compilation_Context* context,
                                       Tac_Renaming_Info* renaming_info,
                                       Tac_Function* tac_function,
                                       Cfg_Block_Id this_block_id)
{
    struct
    {
        stack(Tac_Variable_Id, encountered_variable_ids);
    } encountered_variables = {0};

    Cfg_Block* block = get_cfg_block_by_id(tac_function, this_block_id);

    ensure_array_has_enough_capacity(context->scratch_arena,
                                     encountered_variables.encountered_variable_ids,
                                     Tac_Variable_Id,
                                     block->phi_nodes_count + block->instructions_range.end_instruction_index - block->instructions_range.start_instruction_index);

    for (Index phi_node_index = 0;
         phi_node_index < block->phi_nodes_count;
         ++phi_node_index)
    {
        Phi_Node* phi_node = &block->phi_nodes[phi_node_index];

        {
            const Tac_Variable_Id destination_id = phi_node->destination;
            Tac_Variable* destination = get_tac_variable_by_id(&context->tac, destination_id);
            ASSERT(!destination->is_temporary);
        }

        const Index version = push_new_tac_variable_version(renaming_info, phi_node->destination);
        phi_node->destination.ssa_version = version;
        stack_push(context->scratch_arena,
                   encountered_variables.encountered_variable_ids,
                   Tac_Variable_Id,
                   phi_node->destination);
    }

    ASSERT(tac_function->label_id.index == block->instructions_range.function_label_id.index);

    for (Index instruction_index = block->instructions_range.start_instruction_index;
         instruction_index < block->instructions_range.end_instruction_index;
         ++instruction_index)
    {
        Tac_Instruction* instruction = &tac_function->instructions[instruction_index];

        if (instruction->first_argument.kind == TAC_OPERAND_VARIABLE)
        {
            instruction->first_argument.variable_id.ssa_version = get_tac_variable_version(renaming_info, instruction->first_argument.variable_id);
        }

        if (instruction->second_argument.kind == TAC_OPERAND_VARIABLE)
        {
            instruction->second_argument.variable_id.ssa_version = get_tac_variable_version(renaming_info, instruction->second_argument.variable_id);
        }

        if (instruction->destination.kind == TAC_OPERAND_VARIABLE)
        {
            const Index version = push_new_tac_variable_version(renaming_info, instruction->destination.variable_id);
            instruction->destination.variable_id.ssa_version = version;
            stack_push(context->scratch_arena,
                       encountered_variables.encountered_variable_ids,
                       Tac_Variable_Id,
                       instruction->destination.variable_id);
        }
    }

    for (Index successor_index = 0;
         successor_index < block->edges_count;
         ++successor_index)
    {
        const Cfg_Block_Id successor_id = block->edges[successor_index];
        Cfg_Block* successor = get_cfg_block_by_id(tac_function, successor_id);

        Index this_block_index_in_list_of_predecessors = -1;
        for (Index predecessor_index = 0;
             predecessor_index < successor->predecessors_count;
             ++predecessor_index)
        {
            const Cfg_Block_Id predecessor_id = successor->predecessors[predecessor_index];
            if (predecessor_id.index == this_block_id.index)
            {
                this_block_index_in_list_of_predecessors = predecessor_index;
                break;
            }
        }

        ASSERT(this_block_index_in_list_of_predecessors != -1);

        for (Index successor_phi_node_index = 0;
             successor_phi_node_index < successor->phi_nodes_count;
             ++successor_phi_node_index)
        {
            Phi_Node* phi_node = &successor->phi_nodes[successor_phi_node_index];

            const Tac_Variable_Id destination_id = phi_node->destination;
            const Index destination_version = get_tac_variable_version(renaming_info, destination_id);

            if (destination_version != SSA_VERSION_UNDEFINED)
            {
                phi_node->previous_variables[this_block_index_in_list_of_predecessors] = destination_id;
                phi_node->previous_variables[this_block_index_in_list_of_predecessors].ssa_version = destination_version;
            }
        }
    }

    for (Index dominated_block_index = 0;
         dominated_block_index < block->dominated_block_ids_count;
         ++dominated_block_index)
    {
        const Cfg_Block_Id dominated_block_id = block->dominated_block_ids[dominated_block_index];
        set_tac_variable_versions_in_cfg_block(context, renaming_info, tac_function, dominated_block_id);
    }

    while (encountered_variables.encountered_variable_ids_count > 0)
    {
        const Tac_Variable_Id encountered_variable_id = *stack_top(encountered_variables.encountered_variable_ids);
        stack_pop(encountered_variables.encountered_variable_ids);

        pop_tac_variable_version(renaming_info, encountered_variable_id);
    }
}

internal void
set_tac_variable_versions(Compilation_Context* context)
{
    build_dominator_tree(context);

    Tac* tac = &context->tac;

    Tac_Renaming_Info renaming_info = {0};
    renaming_info.variables = allocate_array(context->scratch_arena, tac->variables_count, Tac_Variable_Renaming_Info);

    for (Index variable_index = INVALID_TAC_INDEX + 1;
         variable_index < tac->variables_count;
         ++variable_index)
    {
        Tac_Variable_Renaming_Info* variable_info = &renaming_info.variables[variable_index];
        variable_info->versions_arena = context->scratch_arena; // TODO(vlad): Request separate arenas from provider?
    }

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        const Cfg_Block_Id entry_block_id = {0};
        set_tac_variable_versions_in_cfg_block(context, &renaming_info, tac_function, entry_block_id);
    }

    for (Index variable_index = INVALID_TAC_INDEX + 1;
         variable_index < tac->variables_count;
         ++variable_index)
    {
        Tac_Variable_Renaming_Info* variable_info = &renaming_info.variables[variable_index];

        Tac_Variable_Id variable_id = {0};
        variable_id.index = variable_index;

        Tac_Variable* variable = get_tac_variable_by_id(tac, variable_id);

        variable->max_ssa_version = variable_info->next_version;
    }

    request_arena_reset(context->arena_provider, context->scratch_arena);
}

internal void
construct_ssa_from_cfg(Compilation_Context* context)
{
    remove_unreachable_cfg_blocks(context);
    compute_cfg_dominators(context);
    compute_cfg_dominance_frontiers(context);
    insert_phi_nodes(context);
    set_tac_variable_versions(context);
}

struct Version_Info
{
    Bool was_used;
    Bool was_used_outside_of_phi_nodes;
    Bool was_assigned_to;
    Bool declared_in_phi_node;

    union
    {
        Phi_Node* phi_node;
        Tac_Instruction_Id instruction_id;
    };
};
typedef struct Version_Info Version_Info;

struct Ssa_Variable_Versions_Info
{
    Size versions_count;
    Version_Info* version_infos;
};
typedef struct Ssa_Variable_Versions_Info Ssa_Variable_Versions_Info;

internal void
emit_diagnostic_message_about_unused_ssa_version(Compilation_Context* context,
                                                 const Ssa_Variable_Versions_Info* variable_versions_info,
                                                 const Tac_Variable_Id ssa_variable_id)
{
    const Ssa_Variable_Versions_Info* this_variable_info = &variable_versions_info[ssa_variable_id.index];
    const Version_Info* this_version_info = &this_variable_info->version_infos[ssa_variable_id.ssa_version];

    if (this_version_info->declared_in_phi_node)
    {
        const Phi_Node* phi_node = this_version_info->phi_node;

        // NOTE(vlad): Sanity check.
        ASSERT(phi_node->destination.index == ssa_variable_id.index);
        ASSERT(phi_node->destination.ssa_version == ssa_variable_id.ssa_version);

        // FIXME(vlad): Sort versions in PHI nodes before emitting diagnostic messages.
        for (Index argument_index = 0;
             argument_index < phi_node->previous_variables_count;
             ++argument_index)
        {
            const Tac_Variable_Id argument_id = phi_node->previous_variables[argument_index];
            ASSERT(argument_id.index != INVALID_TAC_INDEX);
            ASSERT(argument_id.ssa_version != SSA_VERSION_UNDEFINED);

            const Ssa_Variable_Versions_Info* argument_info = &variable_versions_info[argument_id.index];
            const Version_Info* argument_version_info = &argument_info->version_infos[argument_id.ssa_version];

            if (!argument_version_info->was_used_outside_of_phi_nodes)
            {
                emit_diagnostic_message_about_unused_ssa_version(context, variable_versions_info, argument_id);
            }
        }
    }
    else
    {
        const Tac_Instruction_Id instruction_id = this_version_info->instruction_id;

        const Tac_Function* tac_function = get_tac_function_by_label(&context->tac, instruction_id.function_label_id);
        const Ast_Function_Definition* ast_function = tac_function->ast_function_definition;

        const Ast_Statement* statement = find_statement_in_code_block_by_tac_instruction_index(&ast_function->body,
                                                                                               instruction_id.instruction_index);
        ASSERT(statement);

        Diagnostic_Message error = {0};
        error.level = MESSAGE_LEVEL_ERROR;
        error.location = statement->start_location;
        error.text = string_view("This assignment is unused");

        emit_diagnostic_message(context, &error);
    }
}

internal void
find_unused_ssa_assignments(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    Ssa_Variable_Versions_Info* infos = allocate_array(context->scratch_arena, tac->variables_count, Ssa_Variable_Versions_Info);

    for (Index variable_index = INVALID_TAC_INDEX + 1;
         variable_index < tac->variables_count;
         ++variable_index)
    {
        Ssa_Variable_Versions_Info* version_info = &infos[variable_index];

        Tac_Variable_Id variable_id = {0};
        variable_id.index = variable_index;

        Tac_Variable* variable = get_tac_variable_by_id(tac, variable_id);

        version_info->versions_count = variable->max_ssa_version + 1;
        version_info->version_infos = allocate_array(context->scratch_arena, version_info->versions_count, Version_Info);
    }

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        for (Index this_block_index = 0;
             this_block_index < tac_function->cfg_blocks_count;
             ++this_block_index)
        {
            Cfg_Block_Id this_block_id = {0};
            this_block_id.index = this_block_index;

            const Cfg_Block* this_block = get_cfg_block_by_id(tac_function, this_block_id);

            for (Index phi_node_index = 0;
                 phi_node_index < this_block->phi_nodes_count;
                 ++phi_node_index)
            {
                Phi_Node* phi_node = &this_block->phi_nodes[phi_node_index];

                {
                    ASSERT(phi_node->destination.ssa_version != SSA_VERSION_UNDEFINED);

                    const Tac_Variable_Id destination_id = phi_node->destination;
                    ASSERT(destination_id.ssa_version != SSA_VERSION_UNDEFINED);

                    Ssa_Variable_Versions_Info* destination_info = &infos[destination_id.index];
                    Version_Info* this_version_info = &destination_info->version_infos[destination_id.ssa_version];

                    this_version_info->declared_in_phi_node = true;
                    this_version_info->phi_node = phi_node;
                }

                for (Index phi_argument_index = 0;
                     phi_argument_index < phi_node->previous_variables_count;
                     ++phi_argument_index)
                {
                    const Tac_Variable_Id argument_id = phi_node->previous_variables[phi_argument_index];

                    if (argument_id.ssa_version == SSA_VERSION_UNSET)
                    {
                        continue;
                    }

                    ASSERT(argument_id.ssa_version != SSA_VERSION_UNDEFINED);

                    Ssa_Variable_Versions_Info* argument_info = &infos[argument_id.index];
                    argument_info->version_infos[argument_id.ssa_version].was_used = true;
                }
            }
        }

        for (Index instruction_index = 0;
             instruction_index < tac_function->instructions_count;
             ++instruction_index)
        {
            Tac_Instruction* instruction = &tac_function->instructions[instruction_index];

            if (instruction->destination.kind == TAC_OPERAND_VARIABLE)
            {
                const Tac_Variable_Id variable_id = instruction->destination.variable_id;
                ASSERT(variable_id.ssa_version != SSA_VERSION_UNDEFINED);

                Ssa_Variable_Versions_Info* info = &infos[variable_id.index];

                Tac_Instruction_Id instruction_id = {0};
                instruction_id.function_label_id = tac_function->label_id;
                instruction_id.instruction_index = instruction_index;

                info->version_infos[variable_id.ssa_version].instruction_id = instruction_id;
                info->version_infos[variable_id.ssa_version].was_assigned_to = true;
            }

            if (instruction->first_argument.kind == TAC_OPERAND_VARIABLE)
            {
                const Tac_Variable_Id variable_id = instruction->first_argument.variable_id;
                ASSERT(variable_id.ssa_version != SSA_VERSION_UNDEFINED);

                Ssa_Variable_Versions_Info* info = &infos[variable_id.index];
                info->version_infos[variable_id.ssa_version].was_used = true;
                info->version_infos[variable_id.ssa_version].was_used_outside_of_phi_nodes = true;
            }

            if (instruction->second_argument.kind == TAC_OPERAND_VARIABLE)
            {
                const Tac_Variable_Id variable_id = instruction->second_argument.variable_id;
                ASSERT(variable_id.ssa_version != SSA_VERSION_UNDEFINED);

                Ssa_Variable_Versions_Info* info = &infos[variable_id.index];
                info->version_infos[variable_id.ssa_version].was_used = true;
                info->version_infos[variable_id.ssa_version].was_used_outside_of_phi_nodes = true;
            }
        }
    }

    for (Index variable_index = INVALID_TAC_INDEX + 1;
         variable_index < tac->variables_count;
         ++variable_index)
    {
        Tac_Variable_Id variable_id = {0};
        variable_id.index = variable_index;

        const Tac_Variable* variable = get_tac_variable_by_id(tac, variable_id);
        if (variable->is_temporary)
        {
            // XXX(vlad): We should probably report that the result of some expression is unused.
            continue;
        }

        Ssa_Variable_Versions_Info* info = &infos[variable_index];
        Bool variable_was_never_used = true;
        Bool variable_was_reassigned = false;

        for (Index version = SSA_VERSION_UNDEFINED + 1;
             version < info->versions_count;
             ++version)
        {
            const Version_Info* version_info = &info->version_infos[version];

            if (version_info->was_used_outside_of_phi_nodes)
            {
                variable_was_never_used = false;
                break;
            }

            variable_was_reassigned = (version != SSA_VERSION_UNDEFINED + 1) && version_info->was_assigned_to;
        }

        if (variable_was_never_used)
        {
            const Symbol* symbol = get_symbol_by_id(context, variable->symbol_id);

            Diagnostic_Message error = {0};
            error.level = MESSAGE_LEVEL_ERROR;
            error.location = symbol->location;

            if (variable_was_reassigned)
            {
                error.text = string_view("This variable was set but not used");
            }
            else
            {
                error.text = string_view("This variable was never used");
            }

            emit_diagnostic_message(context, &error);

            continue;
        }

        for (Index version = SSA_VERSION_UNDEFINED + 1;
             version < info->versions_count;
             ++version)
        {
            const Version_Info* version_info = &info->version_infos[version];

            if (!version_info->was_used)
            {
                Tac_Variable_Id ssa_id = {0};
                ssa_id.index = variable_index;
                ssa_id.ssa_version = version;

                emit_diagnostic_message_about_unused_ssa_version(context, infos, ssa_id);
            }
        }
    }
}

internal void
perform_constant_folding(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        Bool constant_was_folded = false;
        do
        {
            constant_was_folded = false;

            for (Index instruction_index = 0;
                 instruction_index < tac_function->instructions_count;
                 ++instruction_index)
            {
                Tac_Instruction* instruction = &tac_function->instructions[instruction_index];

                const Bool can_fold =
                    instruction->destination.kind == TAC_OPERAND_VARIABLE
                    && instruction->first_argument.kind == TAC_OPERAND_CONSTANT
                    && instruction->second_argument.kind == TAC_OPERAND_CONSTANT
                    && (instruction->operation == TAC_ADD
                        || instruction->operation == TAC_SUBTRACT
                        || instruction->operation == TAC_MULTIPLY
                        || instruction->operation == TAC_DIVIDE
                        || instruction->operation == TAC_EQUAL
                        || instruction->operation == TAC_NOT_EQUAL
                        || instruction->operation == TAC_LESS
                        || instruction->operation == TAC_LESS_OR_EQUAL
                        || instruction->operation == TAC_GREATER
                        || instruction->operation == TAC_GREATER_OR_EQUAL);

                if (!can_fold)
                {
                    continue;
                }

                const Tac_Variable_Id destination_id = instruction->destination.variable_id;
                Tac_Variable* destination = get_tac_variable_by_id(tac, destination_id);

                // TODO(vlad): Support constant folding for non-temporary variables if optimisations are enabled. We
                //             then also would need to fix PHI nodes (temporary variables cannot have PHI nodes, thus we
                //             do not bother checking them).
                if (!destination->is_temporary)
                {
                    continue;
                }

                Tac_Constant* first_argument = get_tac_constant_by_id(tac, instruction->first_argument.constant_id);
                Tac_Constant* second_argument = get_tac_constant_by_id(tac, instruction->second_argument.constant_id);

                ASSERT(first_argument->kind == second_argument->kind);

                Tac_Constant folded_constant = {0};

                const Tac_Constant_Kind argument_kind = first_argument->kind;

                switch (instruction->operation)
                {
                    // FIXME(vlad): Add overflow/underflow detection.
#define DECLARE_ARITHMETIC_CASE(operation, operator)                    \
                    case operation:                                     \
                    {                                                   \
                        folded_constant.kind = argument_kind;           \
                                                                        \
                        switch (argument_kind)                          \
                        {                                               \
                            case TAC_CONSTANT_UNDEFINED:                \
                            case TAC_CONSTANT_BOOLEAN:                  \
                            {                                           \
                                UNREACHABLE();                          \
                            } break;                                    \
                                                                        \
                            case TAC_CONSTANT_INT8:                     \
                            case TAC_CONSTANT_INT16:                    \
                            case TAC_CONSTANT_INT32:                    \
                            case TAC_CONSTANT_INT64:                    \
                            case TAC_CONSTANT_UINT8:                    \
                            case TAC_CONSTANT_UINT16:                   \
                            case TAC_CONSTANT_UINT32:                   \
                            case TAC_CONSTANT_UINT64:                   \
                            {                                           \
                                folded_constant.kind = argument_kind;   \
                                folded_constant.integer_value = first_argument->integer_value operator second_argument->integer_value; \
                            } break;                                    \
                                                                        \
                            case TAC_CONSTANT_FLOAT32:                  \
                            {                                           \
                                folded_constant.kind = argument_kind;   \
                                folded_constant.float32_value = first_argument->float32_value operator second_argument->float32_value; \
                            } break;                                    \
                                                                        \
                            case TAC_CONSTANT_FLOAT64:                  \
                            {                                           \
                                folded_constant.kind = argument_kind;   \
                                folded_constant.float64_value = first_argument->float64_value operator second_argument->float64_value; \
                            } break;                                    \
                        }                                               \
                    } break

                    DECLARE_ARITHMETIC_CASE(TAC_ADD, +);
                    DECLARE_ARITHMETIC_CASE(TAC_SUBTRACT, -);
                    DECLARE_ARITHMETIC_CASE(TAC_MULTIPLY, *);
                    DECLARE_ARITHMETIC_CASE(TAC_DIVIDE, /);

#undef DECLARE_ARITHMETIC_CASE

#define DECLARE_COMPARISON_CASE(operation, operator)                    \
                    case operation:                                     \
                    {                                                   \
                        folded_constant.kind = TAC_CONSTANT_BOOLEAN;    \
                                                                        \
                        switch (argument_kind)                          \
                        {                                               \
                            case TAC_CONSTANT_UNDEFINED:                \
                            {                                           \
                                UNREACHABLE();                          \
                            } break;                                    \
                                                                        \
                            case TAC_CONSTANT_BOOLEAN:                  \
                            {                                           \
                                folded_constant.boolean_value = first_argument->boolean_value operator second_argument->boolean_value; \
                            } break;                                    \
                                                                        \
                            case TAC_CONSTANT_INT8:                     \
                            case TAC_CONSTANT_INT16:                    \
                            case TAC_CONSTANT_INT32:                    \
                            case TAC_CONSTANT_INT64:                    \
                            case TAC_CONSTANT_UINT8:                    \
                            case TAC_CONSTANT_UINT16:                   \
                            case TAC_CONSTANT_UINT32:                   \
                            case TAC_CONSTANT_UINT64:                   \
                            {                                           \
                                folded_constant.integer_value = first_argument->integer_value operator second_argument->integer_value; \
                            } break;                                    \
                                                                        \
                            case TAC_CONSTANT_FLOAT32:                  \
                            {                                           \
                                folded_constant.float32_value = first_argument->float32_value operator second_argument->float32_value; \
                            } break;                                    \
                                                                        \
                            case TAC_CONSTANT_FLOAT64:                  \
                            {                                           \
                                folded_constant.float64_value = first_argument->float64_value operator second_argument->float64_value; \
                            } break;                                    \
                        }                                               \
                    } break

                    DECLARE_COMPARISON_CASE(TAC_EQUAL, ==);
                    DECLARE_COMPARISON_CASE(TAC_NOT_EQUAL, !=);

                    DECLARE_COMPARISON_CASE(TAC_LESS, <);
                    DECLARE_COMPARISON_CASE(TAC_LESS_OR_EQUAL, <=);
                    DECLARE_COMPARISON_CASE(TAC_GREATER, >);
                    DECLARE_COMPARISON_CASE(TAC_GREATER_OR_EQUAL, >=);

#undef DECLARE_COMPARISON_CASE

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }

                first_argument = NULL;
                second_argument = NULL;

                const Tac_Constant_Id folded_constant_id = create_tac_constant(context);
                {
                    Tac_Constant* created_constant = get_tac_constant_by_id(tac, folded_constant_id);
                    *created_constant = folded_constant;
                }

                // TODO(vlad): Improve the asymptotic complexity of this code.
                for (Index following_instruction_index = instruction_index + 1;
                     following_instruction_index < tac_function->instructions_count;
                     ++following_instruction_index)
                {
                    Tac_Instruction* following_instruction = &tac_function->instructions[following_instruction_index];

                    {
                        Tac_Operand* argument = &following_instruction->first_argument;

                        if (argument->kind == TAC_OPERAND_VARIABLE
                            && argument->variable_id.index == destination_id.index
                            && argument->variable_id.ssa_version == destination_id.ssa_version)
                        {
                            argument->kind = TAC_OPERAND_CONSTANT;
                            argument->constant_id = folded_constant_id;
                        }
                    }

                    {
                        Tac_Operand* argument = &following_instruction->second_argument;

                        if (argument->kind == TAC_OPERAND_VARIABLE
                            && argument->variable_id.index == destination_id.index
                            && argument->variable_id.ssa_version == destination_id.ssa_version)
                        {
                            argument->kind = TAC_OPERAND_CONSTANT;
                            argument->constant_id = folded_constant_id;
                        }
                    }
                }

                instruction->operation = TAC_NOP;
                instruction->destination.kind = TAC_OPERAND_NONE;
                instruction->first_argument.kind = TAC_OPERAND_NONE;
                instruction->second_argument.kind = TAC_OPERAND_NONE;

                constant_was_folded = true;
            }
        }
        while (constant_was_folded);
    }
}

internal void
remove_unreachable_jumps(Compilation_Context* context)
{
    Tac* tac = &context->tac;

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        Tac_Function* tac_function = &tac->functions[function_index];

        for (Index this_block_index = 0;
             this_block_index < tac_function->cfg_blocks_count;
             ++this_block_index)
        {
            Cfg_Block_Id this_block_id = {0};
            this_block_id.index = this_block_index;

            Cfg_Block* this_block = get_cfg_block_by_id(tac_function, this_block_id);

            const Tac_Instructions_Range* instructions_range = &this_block->instructions_range;

            for (Index instruction_index = instructions_range->start_instruction_index;
                 instruction_index < instructions_range->end_instruction_index;
                 ++instruction_index)
            {
                Tac_Instruction* instruction = &tac_function->instructions[instruction_index];

                if (instruction->operation != TAC_JUMP_IF_TRUE && instruction->operation != TAC_JUMP_IF_FALSE)
                {
                    continue;
                }

                ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);

                const Tac_Operand* condition = &instruction->first_argument;
                if (condition->kind != TAC_OPERAND_CONSTANT)
                {
                    continue;
                }

                const Tac_Constant* constant = get_tac_constant_by_id(tac, condition->constant_id);
                ASSERT(constant->kind == TAC_CONSTANT_BOOLEAN);

                switch (instruction->operation)
                {
                    case TAC_JUMP_IF_TRUE:
                    {
                        if (constant->boolean_value)
                        {
                            // NOTE(vlad): Removing fall through edge.

                            const Index next_instruction_index = this_block->instructions_range.end_instruction_index;
                            ASSERT(next_instruction_index != tac_function->instructions_count);

                            for (Index successor_block_index = this_block_id.index + 1;
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

                                    Cfg_Block* destination_block = get_cfg_block_by_id(tac_function, destination_block_id);

                                    remove_edge(this_block, destination_block_id);
                                    remove_predecessor(destination_block, this_block_id);
                                }
                            }
                        }
                        else
                        {
                            const Tac_Label_Id label_id = instruction->destination.label_id;
                            const Cfg_Block_Id destination_block_id = tac->label_index_to_cfg_block_id_map[label_id.index];

                            Cfg_Block* destination_block = get_cfg_block_by_id(tac_function, destination_block_id);

                            remove_edge(this_block, destination_block_id);
                            remove_predecessor(destination_block, this_block_id);

                            instruction->operation = TAC_NOP;
                            instruction->destination.kind = TAC_OPERAND_NONE;
                            instruction->first_argument.kind = TAC_OPERAND_NONE;
                            instruction->second_argument.kind = TAC_OPERAND_NONE;
                        }
                    } break;

                    case TAC_JUMP_IF_FALSE:
                    {
                        if (constant->boolean_value)
                        {
                            const Tac_Label_Id label_id = instruction->destination.label_id;
                            const Cfg_Block_Id destination_block_id = tac->label_index_to_cfg_block_id_map[label_id.index];

                            Cfg_Block* destination_block = get_cfg_block_by_id(tac_function, destination_block_id);

                            remove_edge(this_block, destination_block_id);
                            remove_predecessor(destination_block, this_block_id);
                        }
                        else
                        {
                            // NOTE(vlad): Removing fall through edge.

                            const Index next_instruction_index = this_block->instructions_range.end_instruction_index;
                            ASSERT(next_instruction_index != tac_function->instructions_count);

                            for (Index successor_block_index = this_block_id.index + 1;
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

                                    Cfg_Block* destination_block = get_cfg_block_by_id(tac_function, destination_block_id);

                                    remove_edge(this_block, destination_block_id);
                                    remove_predecessor(destination_block, this_block_id);
                                }
                            }
                        }

                        instruction->operation = TAC_NOP;
                        instruction->destination.kind = TAC_OPERAND_NONE;
                        instruction->first_argument.kind = TAC_OPERAND_NONE;
                        instruction->second_argument.kind = TAC_OPERAND_NONE;
                    } break;

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }
            }
        }
    }
}
