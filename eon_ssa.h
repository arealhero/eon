#pragma once

#include "eon_forward_declarations.h"

maybe_unused internal void construct_ssa_from_cfg(struct Compilation_Context* context);

maybe_unused internal void find_unused_ssa_assignments(struct Compilation_Context* context);
maybe_unused internal void perform_constant_folding(struct Compilation_Context* context);
maybe_unused internal void remove_unreachable_jumps(struct Compilation_Context* context);
