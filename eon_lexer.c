#include "eon_lexer.h"

#include <stdlib.h>
#include <string.h> // FIXME(vlad): Remove this header.

internal inline bool32
lexer_is_a_keyword(const Token* token)
{
    switch (token->tag)
    {
        case TOKEN_CHAR:
        case TOKEN_NUMBER:
        case TOKEN_IDENTIFIER:
        {
            return false;
        } break;

        case TOKEN_TRUE:
        case TOKEN_FALSE:
        {
            return true;
        } break;
    }
}

internal Token*
lexer_get_keyword(Lexer* lexer, const char* lexeme)
{
    for (ssize i = 0;
         i < lexer->keywords_count;
         ++i)
    {
        if (strcmp(lexer->keywords[i].lexeme, lexeme) == 0)
        {
            return &lexer->keywords[i];
        }
    }

    return NULL;
}

internal Token*
lexer_get_identifier(Lexer* lexer, const char* lexeme)
{
    for (ssize i = 0;
         i < lexer->identifiers_count;
         ++i)
    {
        if (strcmp(lexer->identifiers[i].lexeme, lexeme) == 0)
        {
            return &lexer->identifiers[i];
        }
    }

    return NULL;
}

internal inline bool32
lexer_is_digit(const char c)
{
    return '0' <= c && c <= '9';
}
internal inline bool32
lexer_is_letter(const char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

internal void
lexer_add_keyword(Lexer* lexer, const Token keyword) // TODO(vlad): Get 'keyword' by pointer?
{
    ASSERT(lexer_is_a_keyword(&keyword));

    if (lexer->keywords_count >= lexer->keywords_capacity)
    {
        lexer->keywords_capacity = MAX(1, 2 * lexer->keywords_capacity);
        lexer->keywords = (Token*) realloc(lexer->keywords, size_of(Token) * lexer->keywords_capacity);
    }

    lexer->keywords[lexer->keywords_count++] = keyword;
}

internal void
lexer_add_identifier(Lexer* lexer, const Token identifier) // TODO(vlad): Get 'identifier' by pointer?
{
    ASSERT(identifier.tag == TOKEN_IDENTIFIER);

    if (lexer->identifiers_count >= lexer->identifiers_capacity)
    {
        lexer->identifiers_capacity = MAX(1, 2 * lexer->identifiers_capacity);
        lexer->identifiers = (Token*) realloc(lexer->identifiers, size_of(Token) * lexer->identifiers_capacity);
    }

    lexer->identifiers[lexer->identifiers_count++] = identifier;
}

internal void
lexer_show_error_at_peek(Lexer* lexer)
{
    printf("\n%s\n", lexer->input);
    for (ssize i = 0;
         i < lexer->peek_index;
         ++i)
    {
        printf(" ");
    }
    printf("^\n");
}

internal bool32
lexer_advance(Lexer* lexer)
{
    if (lexer->peek_index >= lexer->input_length)
    {
        lexer_show_error_at_peek(lexer);
        puts("[lexer] Error: unexpected EOF reached.");
        return false;
    }

    lexer->peek_index += 1;
    return true;
}

internal inline char
lexer_peek(Lexer* lexer)
{
    ASSERT(lexer->peek_index < lexer->input_length);
    return lexer->input[lexer->peek_index];
}

internal void
lexer_create(Lexer* lexer, const char* input)
{
    lexer->input = input;
    lexer->input_length = strlen(input);

    lexer->line = 1;
    lexer->peek_index = 0;

    lexer_add_keyword(lexer, (Token){ .tag = TOKEN_TRUE, .lexeme = "true" });
    lexer_add_keyword(lexer, (Token){ .tag = TOKEN_FALSE, .lexeme = "false" });
}

internal bool32
lexer_scan(Lexer* lexer, Token* next_token)
{
    // NOTE(vlad): Skipping whitespaces.
    {
        while (true)
        {
            const char peek = lexer_peek(lexer);
            if (peek == '\n')
            {
                lexer->line += 1;
            }
            else if (peek != ' ' && peek != '\t')
            {
                break;
            }
        }
    }

    if (lexer_is_digit(lexer_peek(lexer)))
    {
        s64 value = 0;
        do
        {
            value = 10 * value + (lexer_peek(lexer) - '0');
            if (!lexer_advance(lexer)) { return false; }
        }
        while (lexer_is_digit(lexer_peek(lexer)));

        next_token->tag = TOKEN_NUMBER;
        next_token->number = value;

        return true;
    }

    if (lexer_is_letter(lexer_peek(lexer)))
    {
        char* identifier = calloc(sizeof(char), lexer->input_length - lexer->peek_index - 1);
        ssize index = 0;

        do
        {
            identifier[index++] = lexer_peek(lexer);
            if (!lexer_advance(lexer)) { return false; }
        }
        while (lexer_is_digit(lexer_peek(lexer))
               || lexer_is_letter(lexer_peek(lexer)));

        {
            Token* keyword = lexer_get_keyword(lexer, identifier);
            if (keyword != NULL)
            {
                next_token = keyword;
                return true;
            }
        }

        {
            Token* existing_identifier = lexer_get_identifier(lexer, identifier);
            if (existing_identifier != NULL)
            {
                next_token = existing_identifier;
                return true;
            }
        }

        next_token->tag = TOKEN_IDENTIFIER;
        next_token->lexeme = identifier;

        lexer_add_identifier(lexer, *next_token);
        return true;
    }

    // FIXME(vlad): Test that 'lexer_peek(lexer)' is an operator like '+', '-' and such.
    next_token->tag = TOKEN_CHAR;
    next_token->c = lexer_peek(lexer);

    return true;
}

internal void
lexer_destroy(Lexer* lexer)
{
    if (lexer->keywords)
    {
        free(lexer->keywords);
    }

    if (lexer->identifiers)
    {
        for (ssize i = 0;
             i < lexer->identifiers_count;
             ++i)
        {
            free((void*)lexer->identifiers[i].lexeme);
        }
        free(lexer->identifiers);
    }
}
