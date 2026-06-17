#pragma once

#include "eon_forward_declarations.h"

struct Tac_Instructions_Range
{
    Tac_Function_Label_Id function_label_id;
    Index start_instruction_index;
    Index end_instruction_index; // NOTE(vlad): This index is not included.
};
typedef struct Tac_Instructions_Range Tac_Instructions_Range;

struct Cfg_Block
{
    Tac_Instructions_Range instructions_range;
};
typedef struct Cfg_Block Cfg_Block;

struct Cfg_Edge
{
    Cfg_Block_Id source_block_id;
    Cfg_Block_Id destination_block_id;
};
typedef struct Cfg_Edge Cfg_Edge;

struct Cfg
{
    array(Cfg_Block, blocks);
    array(Cfg_Edge, edges);
};
typedef struct Cfg Cfg;

maybe_unused internal void construct_cfg_from_tac(struct Compilation_Context* context);
maybe_unused internal void remove_unreachable_cfg_blocks(struct Compilation_Context* context);

maybe_unused internal inline Cfg_Block* get_cfg_block_by_id(Cfg* cfg, const Cfg_Block_Id id);
maybe_unused internal inline Bool cfg_block_is_empty(const Cfg_Block* block);
