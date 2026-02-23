#pragma once

#include <eon/memory.h>

#include "eon_parser.h"

internal Bool create_lexical_scopes_and_infer_types(Arena* lexical_scopes_arena, Ast* ast);
