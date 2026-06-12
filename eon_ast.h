#pragma once

#include "eon_forward_declarations.h"

#include "eon_lexer.h"

struct Ast_Type;
struct Ast_Expression;
struct Ast_Statement;

struct Ast_Identifier
{
    Token token;
    Symbol_Id symbol_id;
};
typedef struct Ast_Identifier Ast_Identifier;

struct Ast_Number
{
    Token token;
    Bool is_a_floating_point_number; // FIXME(vlad): Fill this in.
};
typedef struct Ast_Number Ast_Number;

struct Ast_String_Literal
{
    Token token;
    String_View value;
};
typedef struct Ast_String_Literal Ast_String_Literal;

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
    AST_EXPRESSION_LESS,
    AST_EXPRESSION_LESS_OR_EQUAL,
    AST_EXPRESSION_GREATER,
    AST_EXPRESSION_GREATER_OR_EQUAL,

    // NOTE(vlad): Unary expressions.
    AST_EXPRESSION_NEGATE,
    AST_EXPRESSION_DEREFERENCE,
    AST_EXPRESSION_ADDRESS_OF,

    AST_EXPRESSION_CALL,
};
typedef enum Ast_Expression_Type Ast_Expression_Kind;

struct Ast_Unary_Expression
{
    Token operator;
    struct Ast_Expression* operand;
};
typedef struct Ast_Unary_Expression Ast_Unary_Expression;

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

    array(struct Ast_Expression*, arguments);
};
typedef struct Ast_Call Ast_Call;

struct Ast_Expression
{
    Source_Location location;

    Ast_Expression_Kind kind;

    Type_Id type_id;

    union
    {
        Ast_Number number;
        Ast_String_Literal string_literal;
        Ast_Identifier identifier;

        Ast_Unary_Expression unary_expression;
        Ast_Binary_Expression binary_expression;

        Ast_Call call;
    };
};
typedef struct Ast_Expression Ast_Expression;

// NOTE(vlad): Types.

enum Ast_Type_Kind
{
    AST_TYPE_UNDEFINED = 0,
    AST_TYPE_NAME,
    AST_TYPE_POINTER,
    AST_TYPE_FUNCTION,
    AST_TYPE_OMITTED,
};
typedef enum Ast_Type_Kind Ast_Type_Kind;

struct Ast_Named_Type
{
    Token token;
};
typedef struct Ast_Named_Type Ast_Named_Type;

struct Ast_Pointer_Type
{
    Token token; // TODO(vlad): Do we need it here? It's just a token for a *.
    struct Ast_Type* pointed_to;
};
typedef struct Ast_Pointer_Type Ast_Pointer_Type;

struct Ast_Function_Parameter
{
    Ast_Identifier name;
    struct Ast_Type* type;

    Bool has_default_value;
    Ast_Expression default_value;
};
typedef struct Ast_Function_Parameter Ast_Function_Parameter;

struct Ast_Function_Type
{
    array(Ast_Function_Parameter, parameters);

    struct Ast_Type* return_type;
};
typedef struct Ast_Function_Type Ast_Function_Type;

struct Ast_Type
{
    Ast_Type_Kind kind;

    Source_Location location;

    Bool is_mutable;

    Symbol_Id symbol_id; // FIXME(vlad): Move to 'named_type'?
    Type_Id type_id;

    union
    {
        Ast_Named_Type named_type;
        Ast_Pointer_Type pointer;
        Ast_Function_Type function;
    };
};
typedef struct Ast_Type Ast_Type;

// NOTE(vlad): Statements.

struct Ast_Code_Block
{
    // FIXME(vlad): Change to 'location' with a length.
    Source_Location end_location;

    Lexical_Scope_Id lexical_scope_id;

    Type_Id return_type_id;
    Bool every_path_returns;

    array(struct Ast_Statement, statements);
};
typedef struct Ast_Code_Block Ast_Code_Block;

enum Ast_Statement_Kind
{
    AST_STATEMENT_UNDEFINED = 0,

    AST_STATEMENT_VARIABLE_DEFINITION,
    AST_STATEMENT_ASSIGNMENT,
    AST_STATEMENT_RETURN,
    AST_STATEMENT_WHILE,
    AST_STATEMENT_IF,
    AST_STATEMENT_CALL,
};
typedef enum Ast_Statement_Kind Ast_Statement_Kind;

struct Ast_Variable_Definition
{
    Ast_Identifier name;
    Ast_Type* type;

    Bool has_initial_value;
    Ast_Expression initial_value;
};
typedef struct Ast_Variable_Definition Ast_Variable_Definition;

struct Ast_Assignment
{
    Ast_Expression lhs;
    Ast_Expression rhs;
};
typedef struct Ast_Assignment Ast_Assignment;

struct Ast_Return_Statement
{
    Bool is_empty;
    union
    {
        Ast_Expression expression;
        Source_Location empty_expression_location;
    };
};
typedef struct Ast_Return_Statement Ast_Return_Statement;

struct Ast_If_Statement
{
    Ast_Expression condition;
    Ast_Code_Block if_statements; // TODO(vlad): Rename to 'if_branch'?
    Ast_Code_Block else_statements; // TODO(vlad): Rename to 'else_branch'?
};
typedef struct Ast_If_Statement Ast_If_Statement;

struct Ast_While_Statement
{
    Ast_Expression condition;
    Ast_Code_Block body;
};
typedef struct Ast_While_Statement Ast_While_Statement;

struct Ast_Call_Statement
{
    Ast_Expression call_expression;
};
typedef struct Ast_Call_Statement Ast_Call_Statement;

struct Ast_Statement
{
    Ast_Statement_Kind kind;
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

    Ast_Code_Block body;
};
typedef struct Ast_Function_Definition Ast_Function_Definition;

struct Ast
{
    array(Ast_Function_Definition, function_definitions);
};
typedef struct Ast Ast;

