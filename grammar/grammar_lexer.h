#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

enum Token_Type
{
    TOKEN_UNDEFINED = 0,

    TOKEN_TERMINAL,
    TOKEN_NON_TERMINAL,

    TOKEN_COLON, TOKEN_OR, TOKEN_SEMICOLON,

    // XXX(vlad): Do we want to support these?
    // TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,

    TOKEN_EOF,
};
typedef enum Token_Type Token_Type;

struct Token
{
    Token_Type type;
    String_View lexeme;
    s64 line;
    s64 column;
};
typedef struct Token Token;

struct Lexer
{
    String_View grammar;

    Index current_index;
    s64 current_line;
    s64 current_column;
};
typedef struct Lexer Lexer;

internal void lexer_create(Lexer* lexer, String_View grammar);
internal Bool lexer_get_next_token(Arena* scratch, Lexer* lexer, Token* token);
internal void lexer_destroy(Lexer* lexer);
