#pragma once

#include <eon/common.h>
#include <eon/string.h>

enum Token_Type
{
    TOKEN_UNDEFINED = 0,

    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_COLON, TOKEN_SEMICOLON,
    TOKEN_NOT,

    TOKEN_ASSIGN,

    TOKEN_EQUAL, TOKEN_NOT_EQUAL,
    TOKEN_LESS, TOKEN_LESS_OR_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_OR_EQUAL,

    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // NOTE(vlad): Reserved keywords.
    TOKEN_FOR, TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_TRUE, TOKEN_FALSE,

    TOKEN_EOF,
};
typedef enum Token_Type Token_Type;

internal String_View token_type_to_string(const Token_Type type);

struct Token
{
    Token_Type type;
    String_View lexeme;

    Size line;
    Size column;
};
typedef struct Token Token;

struct Keyword
{
    Token_Type type;
    String_View lexeme;
};
typedef struct Keyword Keyword;

struct Lexer
{
    String_View code;

    Size lexeme_start_index;
    Size current_index;

    Size current_line;
    Size current_column;

    Keyword* keywords;
    Size keywords_count;
};
typedef struct Lexer Lexer;

internal void lexer_create(Lexer* lexer, const String_View code);
internal Bool lexer_get_next_token(Lexer* lexer, Token* token);
internal void lexer_destroy(Lexer* lexer);
