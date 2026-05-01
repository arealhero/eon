#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include "eon_lexer.h"
#include "eon_errors.h"

#include "eon_ast.h"

struct Builtin_Type
{
    String_View lexeme;
    Ast_Type_Type type;
};
typedef struct Builtin_Type Builtin_Type;

struct Parser
{
    // TODO(vlad): Move this to AST?
    Errors* errors;

    Lexer* lexer;
    Token current_token;
    Token lookahead_token;

    // TODO(vlad): Move this to AST and point variables' type here instead of creating a new type every time.
    //             Also create something like 'unresolved_types' to be able to iterate over them and resolve
    //             them in 'eon_semantics.h'.
    Builtin_Type* builtin_types;
    Size builtin_types_count;
};
typedef struct Parser Parser;

internal void create_parser(Parser* parser, Arena* parser_arena, Lexer* lexer, Errors* errors);
internal Bool parse_ast(Arena* parser_arena, Parser* parser, Ast* ast);
internal void destroy_parser(Parser* parser);
