#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include "eon_lexer.h"
#include "eon_errors.h"

#include "eon_ast.h"
#include "eon_compilation_context.h"

struct Parser
{
    Compilation_Context* context;

    // FIXME(vlad): Move this to Compilation_Context.
    Errors* errors;

    Lexer* lexer;
    Token current_token;
    Token lookahead_token;
};
typedef struct Parser Parser;

internal void create_parser(Parser* parser, Lexer* lexer, Compilation_Context* context, Errors* errors);
internal Bool parse_ast(Parser* parser);
internal void destroy_parser(Parser* parser);
