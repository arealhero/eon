#pragma once

#include <eon/common.h>
#include <eon/memory.h>

#include "grammar_lexer.h"

struct Ast_Identifier
{
    Token token;
};
typedef struct Ast_Identifier Ast_Identifier;

struct Ast_Expression
{
    Ast_Identifier* identifiers;
    Size identifiers_count;
    Size identifiers_capacity;
};
typedef struct Ast_Expression Ast_Expression;

struct Ast_Identifier_Definition
{
    Ast_Identifier identifier;

    Ast_Expression* possible_expressions;
    Size possible_expressions_count;
    Size possible_expressions_capacity;
};
typedef struct Ast_Identifier_Definition Ast_Identifier_Definition;

struct Ast
{
    Ast_Identifier_Definition* definitions;
    Size definitions_count;
    Size definitions_capacity;
};
typedef struct Ast Ast;

struct Parser
{
    Lexer* lexer;
    Token current_token;
};
typedef struct Parser Parser;

internal void parser_create(Parser* parser, Lexer* lexer);
internal Bool parser_parse(Arena* ast_arena,
                           Arena* scratch,
                           Parser* parser,
                           Ast* ast);
internal void parser_destroy(Parser* parser);
