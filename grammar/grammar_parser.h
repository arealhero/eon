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
    array(Ast_Identifier, identifiers);
};
typedef struct Ast_Expression Ast_Expression;

struct Ast_Identifier_Definition
{
    Ast_Identifier identifier;

    array(Ast_Expression, possible_expressions);
};
typedef struct Ast_Identifier_Definition Ast_Identifier_Definition;

struct Ast
{
    array(Ast_Identifier_Definition, definitions);
};
typedef struct Ast Ast;

struct Parser
{
    Lexer* lexer;
    Token current_token;
};
typedef struct Parser Parser;

internal void create_parser(Parser* parser, Lexer* lexer);
internal Bool parse_ast(Arena* ast_arena,
                        Arena* scratch,
                        Parser* parser,
                        Ast* ast);
internal void destroy_parser(Parser* parser);
