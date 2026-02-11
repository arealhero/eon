#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include "eon_lexer.h"
#include "eon_errors.h"

struct Ast_Identifier
{
    Token token;
};
typedef struct Ast_Identifier Ast_Identifier;

struct Ast_Number
{
    Token token;
};
typedef struct Ast_Number Ast_Number;

struct Ast_String_Literal
{
    Token token;
    String_View value;
};
typedef struct Ast_String_Literal Ast_String_Literal;

enum Ast_Type_Type
{
    AST_TYPE_UNDEFINED = 0,

    AST_TYPE_DEDUCED,
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
            struct Ast_Type* pointed_to;
        };

        // NOTE(vlad): Trivial types (VOID, INT_32, etc) have no data.
    };
};
typedef struct Ast_Type Ast_Type;

// NOTE(vlad): Expressions.

enum Ast_Expression_Type
{
    AST_EXPRESSION_UNDEFINED = 0,

    // NOTE(vlad): Primary expressions.
    AST_EXPRESSION_NUMBER,
    AST_EXPRESSION_STRING_LITERAL,
    AST_EXPRESSION_IDENTIFIER,

    // NOTE(vlad): Additive expressions.
    AST_EXPRESSION_ADD,
    AST_EXPRESSION_SUBTRACT,

    // NOTE(vlad): Multiplicative expressions.
    AST_EXPRESSION_MULTIPLY,
    AST_EXPRESSION_DIVIDE,

    // NOTE(vlad): Comparisons.
    AST_EXPRESSION_EQUAL,
    AST_EXPRESSION_NOT_EQUAL,

    AST_EXPRESSION_CALL,
};
typedef enum Ast_Expression_Type Ast_Expression_Type;

struct Ast_Expression;

struct Ast_Binary_Expression
{
    Token operator;
    struct Ast_Expression* lhs;
    struct Ast_Expression* rhs;
};
typedef struct Ast_Binary_Expression Ast_Binary_Expression;

struct Ast_Call
{
    struct Ast_Expression* called_expression;

    struct Ast_Expression** arguments;
    Size arguments_count;
    Size arguments_capacity;
};
typedef struct Ast_Call Ast_Call;

struct Ast_Expression
{
    Ast_Expression_Type type;
    union
    {
        Ast_Number number;
        Ast_String_Literal string_literal;
        Ast_Identifier identifier;

        Ast_Binary_Expression binary_expression;

        Ast_Call call;
    };
};
typedef struct Ast_Expression Ast_Expression;

// NOTE(vlad): Statements.
struct Ast_Statement;

struct Ast_Statements
{
    struct Ast_Statement* statements;
    Size statements_count;
    Size statements_capacity;
};
typedef struct Ast_Statements Ast_Statements;

enum Ast_Statement_Type
{
    AST_UNDEFINED = 0,

    AST_STATEMENT_VARIABLE_DEFINITION,
    AST_STATEMENT_ASSIGNMENT,
    AST_STATEMENT_RETURN,
    AST_STATEMENT_WHILE,
    AST_STATEMENT_IF,
    AST_STATEMENT_CALL,
};
typedef enum Ast_Statement_Type Ast_Statement_Type;

enum Ast_Initialisation_Type
{
    AST_INITIALISATION_UNDEFINED = 0,

    AST_INITIALISATION_DEFAULT,
    AST_INITIALISATION_WITH_VALUE,
};
typedef enum Ast_Initialisation_Type Ast_Initialisation_Type;

struct Ast_Variable_Definition
{
    Ast_Identifier name;
    Ast_Type* type;

    Ast_Initialisation_Type initialisation_type;
    Ast_Expression initial_value;
};
typedef struct Ast_Variable_Definition Ast_Variable_Definition;

struct Ast_Assignment
{
    Ast_Identifier name;
    Ast_Expression expression;
};
typedef struct Ast_Assignment Ast_Assignment;

struct Ast_Return_Statement
{
    Bool is_empty;
    Ast_Expression expression;
};
typedef struct Ast_Return_Statement Ast_Return_Statement;

struct Ast_If_Statement
{
    Ast_Expression condition;
    Ast_Statements if_statements;
    Ast_Statements else_statements;
};
typedef struct Ast_If_Statement Ast_If_Statement;

struct Ast_While_Statement
{
    Ast_Expression condition;
    Ast_Statements statements;
};
typedef struct Ast_While_Statement Ast_While_Statement;

struct Ast_Call_Statement
{
    Ast_Call call;
};
typedef struct Ast_Call_Statement Ast_Call_Statement;

struct Ast_Statement
{
    Ast_Statement_Type type;
    union
    {
        Ast_Variable_Definition variable_definition;
        Ast_Return_Statement return_statement;
        Ast_If_Statement if_statement;
        Ast_While_Statement while_statement;
        Ast_Assignment assignment;
        Ast_Call_Statement call_statement;
    };
};
typedef struct Ast_Statement Ast_Statement;

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
    // TODO(vlad): Remove this.
    Errors errors;

    Lexer* lexer;
    Token current_token;

    Builtin_Type* builtin_types;
    Size builtin_types_count;
};
typedef struct Parser Parser;

internal void parser_create(Arena* parser_arena, Arena* errors_arena, Parser* parser, Lexer* lexer);
internal Bool parser_parse(Arena* parser_arena, Parser* parser, Ast* ast);
internal void parser_destroy(Parser* parser);
