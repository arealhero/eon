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

struct Ast_Variable_Declaration
{
    Ast_Identifier name;
    Ast_Type* type;
};
typedef struct Ast_Variable_Declaration Ast_Variable_Declaration;

enum Ast_Statement_Type
{
    AST_UNDEFINED = 0,

    AST_STATEMENT_VARIABLE_DECLARATION,
};
typedef enum Ast_Statement_Type Ast_Statement_Type;

struct Ast_Statement
{
    Ast_Statement_Type type;
    union
    {
        Ast_Variable_Declaration variable_declaration;
    };
};
typedef struct Ast_Statement Ast_Statement;

struct Ast_Statements
{
    Ast_Statement* statements;
    Size statements_count;
    Size statements_capacity;
};
typedef struct Ast_Statements Ast_Statements;

struct Ast_Function_Definition
{
    Ast_Identifier name;
    Ast_Type* type;

    Ast_Statements statements;
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
