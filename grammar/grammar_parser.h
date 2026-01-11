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
    ssize identifiers_count;
    ssize identifiers_capacity;
};
typedef struct Ast_Expression Ast_Expression;

struct Ast_Identifier_Definition
{
    Ast_Identifier identifier;

    Ast_Expression* possible_expressions;
    ssize possible_expressions_count;
    ssize possible_expressions_capacity;
};
typedef struct Ast_Identifier_Definition Ast_Identifier_Definition;

struct Ast
{
    Ast_Identifier_Definition* definitions;
    ssize definitions_count;
    ssize definitions_capacity;
};
typedef struct Ast Ast;

struct Parser
{
    Lexer* lexer;
    Token current_token;
};
typedef struct Parser Parser;

internal void parser_create(Parser* parser, Lexer* lexer);
internal bool32 parser_parse(Arena* ast_arena,
                             Arena* scratch,
                             Parser* parser,
                             Ast* ast);
internal void parser_destroy(Parser* parser);
