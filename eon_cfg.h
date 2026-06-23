#pragma once

#include <eon/containers.h>

#include "eon_forward_declarations.h"
#include "eon_tac.h"

struct Cfg_Block
{
    Arena* edges_arena;

    Tac_Instructions_Range instructions_range;

    array(Cfg_Block_Id, edges);

    Index postorder_index;
    Cfg_Block_Id immediate_dominator_id;
};
typedef struct Cfg_Block Cfg_Block;

maybe_unused internal void construct_cfg_from_tac(struct Compilation_Context* context);
maybe_unused internal void remove_unreachable_cfg_blocks(struct Compilation_Context* context);
maybe_unused internal void compute_cfg_dominators(struct Compilation_Context* context);

maybe_unused internal inline Cfg_Block* get_cfg_block_by_id(Tac_Function* tac_function, const Cfg_Block_Id id);
maybe_unused internal inline Bool cfg_block_is_empty(const Cfg_Block* block);
