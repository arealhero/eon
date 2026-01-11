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
    ssize line;
    ssize column;
};
typedef struct Token Token;

struct Lexer
{
    String_View grammar;

    ssize current_index;
    ssize current_line;
    ssize current_column;
};
typedef struct Lexer Lexer;

internal void lexer_create(Lexer* lexer, String_View grammar);
internal bool32 lexer_get_next_token(Arena* scratch, Lexer* lexer, Token* token);
internal void lexer_destroy(Lexer* lexer);
