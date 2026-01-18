#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include "eon_lexer.h"

struct Ast_Identifier
{
    Token token;
};
typedef struct Ast_Identifier Ast_Identifier;

enum Ast_Type_Type
{
    AST_TYPE_UNDEFINED = 0,

    AST_TYPE_USER_DEFINED,

    AST_TYPE_FUNCTION,
    // TODO(vlad): Add AST_TYPE_STRUCT.
    AST_TYPE_POINTER,

    // NOTE(vlad): Reserved types.
    AST_TYPE_VOID,
    AST_TYPE_INT_32,
    // TODO(vlad): Add other integers: AST_TYPE_INT_8, etc.

    // TODO(vlad): Add unions, arrays, etc.
    // TODO(vlad): Add C types: AST_TYPE_C_STRING, AST_TYPE_C_INT, etc.
};
typedef enum Ast_Type_Type Ast_Type_Type;

struct Ast_Type;

struct Ast_Function_Argument
{
    Ast_Identifier name;
    struct Ast_Type* type;
};
typedef struct Ast_Function_Argument Ast_Function_Argument;

struct Ast_Function_Arguments
{
    Ast_Function_Argument* arguments;
    Size arguments_count;
    Size arguments_capacity;
};
typedef struct Ast_Function_Arguments Ast_Function_Arguments;

struct Ast_Type
{
    Ast_Type_Type type;
    union
    {
        // NOTE(vlad): 'type == AST_TYPE_USER_DEFINED'.
        struct
        {
            Ast_Identifier name;
        };

        // NOTE(vlad): 'type == AST_TYPE_FUNCTION'.
        struct
        {
            Ast_Function_Arguments arguments;
            struct Ast_Type* return_type;
        };

        // NOTE(vlad): 'type == AST_TYPE_POINTER'.
        struct
        {
            struct Ast_Type* pointed_to_type;
        };

        // NOTE(vlad): Trivial types (VOID, INT_32, etc) have no data.
    };
};
typedef struct Ast_Type Ast_Type;

struct Ast_Function_Definition
{
    Ast_Identifier name;
    Ast_Type* type;
};
typedef struct Ast_Function_Definition Ast_Function_Definition;

struct Ast
{
    Ast_Function_Definition* function_definitions;
    Size function_definitions_count;
    Size function_definitions_capacity;
};
typedef struct Ast Ast;

struct Builtin_Type
{
    String_View lexeme;
    Ast_Type_Type type;
};
typedef struct Builtin_Type Builtin_Type;

struct Parser
{
    Lexer* lexer;
    Token current_token;

    Builtin_Type* builtin_types;
    Size builtin_types_count;
};
typedef struct Parser Parser;

internal void parser_create(Arena* arena, Parser* parser, Lexer* lexer);
internal Bool parser_parse(Arena* arena, Parser* parser, Ast* ast);
internal void parser_destroy(Parser* parser);
