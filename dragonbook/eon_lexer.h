#pragma once

#include <eon/common.h>

enum Token_Tag
{
    TOKEN_CHAR,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_TRUE,
    TOKEN_FALSE,
};
typedef enum Token_Tag Token_Tag;

struct Token
{
    Token_Tag tag;
    union
    {
        char c;
        s64 number;
        const char* lexeme;
    };
};
typedef struct Token Token;

struct Lexer
{
    const char* input;
    ssize input_length;

    ssize line;
    ssize peek_index;

    // TODO(vlad): Use hash map?
    Token* keywords;
    ssize keywords_count;
    ssize keywords_capacity;

    // TODO(vlad): Use hash map?
    Token* identifiers;
    ssize identifiers_count;
    ssize identifiers_capacity;
};
typedef struct Lexer Lexer;

internal void lexer_create(Lexer* lexer, const char* input);
internal bool32 lexer_scan(Lexer* lexer, Token* next_token);
internal void lexer_destroy(Lexer* lexer);
