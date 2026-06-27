#pragma once

#include <eon/containers.h>

#include "eon_forward_declarations.h"
#include "eon_tac.h"

struct Phi_Node
{
    Tac_Variable_Id destination;
    array(Tac_Variable_Id, previous_variables);
};
typedef struct Phi_Node Phi_Node;

struct Cfg_Block
{
    Arena* edges_arena;
    Arena* predecessors_arena;
    Arena* dominance_frontier_arena;
    Arena* dominated_block_ids_arena;
    Arena* phi_nodes_arena;

    Tac_Instructions_Range instructions_range;

    array(Cfg_Block_Id, edges);
    array(Cfg_Block_Id, predecessors);
    array(Cfg_Block_Id, dominance_frontier);
    array(Cfg_Block_Id, dominated_block_ids);

    array(Phi_Node, phi_nodes);

    Index postorder_index;
    Cfg_Block_Id immediate_dominator_id;
};
typedef struct Cfg_Block Cfg_Block;

maybe_unused internal void construct_cfg_from_tac(struct Compilation_Context* context);
maybe_unused internal void remove_unreachable_cfg_blocks(struct Compilation_Context* context);

// TODO(vlad): Merge these into 'construct_ssa_from_cfg()'?
maybe_unused internal void compute_cfg_dominators(struct Compilation_Context* context);
maybe_unused internal void compute_cfg_dominance_frontiers(struct Compilation_Context* context);
maybe_unused internal void insert_phi_nodes(struct Compilation_Context* context);
maybe_unused internal void set_tac_variable_versions(struct Compilation_Context* context);

maybe_unused internal inline void free_cfg_block(struct Compilation_Context* context, Cfg_Block* block);

maybe_unused internal inline Cfg_Block* get_cfg_block_by_id(Tac_Function* tac_function, const Cfg_Block_Id id);
maybe_unused internal inline Bool cfg_block_is_empty(const Cfg_Block* block);
