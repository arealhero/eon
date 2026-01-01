#pragma once

#include "eon_common.h"

struct Parser
{
    const char* expression;
    ssize expression_length;

    ssize lookahead_index;
};
typedef struct Parser Parser;

internal void parser_create(Parser* parser, const char* expression);
internal bool32 parser_parse_expression(Parser* parser);
internal void parser_destroy(Parser* parser);

