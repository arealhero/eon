#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include "eon_lexer.h"

enum Expression_Type
{
    EXPRESSION_UNDEFINED = 0,

    EXPRESSION_BINARY,
    EXPRESSION_UNARY,
    EXPRESSION_LITERAL,
    EXPRESSION_GROUPING,
};
typedef enum Expression_Type Expression_Type;

struct Expression;

struct Binary_Expression
{
    Token operator;
    struct Expression* lhs;
    struct Expression* rhs;
};
typedef struct Binary_Expression Binary_Expression;

struct Unary_Expression
{
    Token operator;
    struct Expression* expression;
};
typedef struct Unary_Expression Unary_Expression;

struct Literal_Expression
{
    Token token;
};
typedef struct Literal_Expression Literal_Expression;

struct Grouping_Expression
{
    struct Expression* expression;
};
typedef struct Grouping_Expression Grouping_Expression;

struct Expression
{
    Expression_Type type;

    union
    {
        Unary_Expression* unary;
        Binary_Expression* binary;
        Literal_Expression* literal;
        Grouping_Expression* grouping;
    };
};
typedef struct Expression Expression;

internal String_View convert_expression_to_prn(Arena* const restrict arena,
                                               const Expression* const restrict expression);
internal void expression_destroy(Expression* expression);

struct Parser
{
    Lexer* lexer;
    Token previous_token;
    Token current_token;
};
typedef struct Parser Parser;

internal void parser_create(Parser* parser, Lexer* lexer);
internal Expression* parser_parse(Parser* parser);
internal void parser_destroy(Parser* parser);
