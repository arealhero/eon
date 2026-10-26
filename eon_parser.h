#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include "eon_lexer.h"

#include "eon_forward_declarations.h"

struct Parser
{
    Token current_token;
    Token lookahead_token;

    Source_Location previous_token_location;
};
typedef struct Parser Parser;

maybe_unused internal void parse_ast(struct Compilation_Context* context);
