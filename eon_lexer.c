#include "eon_lexer.h"

#include <eon/io.h>

#include "eon_log.h"

#include <stdlib.h>

maybe_unused internal String_View
token_type_to_string(const Token_Type type)
{
#define ADD_TOKEN(type) case type: return string_view(#type)
    switch (type)
    {
        ADD_TOKEN(TOKEN_UNDEFINED);

        ADD_TOKEN(TOKEN_LEFT_PAREN);
        ADD_TOKEN(TOKEN_RIGHT_PAREN);
        ADD_TOKEN(TOKEN_LEFT_BRACE);
        ADD_TOKEN(TOKEN_RIGHT_BRACE);
        ADD_TOKEN(TOKEN_LEFT_BRACKET);
        ADD_TOKEN(TOKEN_RIGHT_BRACKET);

        ADD_TOKEN(TOKEN_COMMA);
        ADD_TOKEN(TOKEN_DOT);
        ADD_TOKEN(TOKEN_MINUS);
        ADD_TOKEN(TOKEN_PLUS);
        ADD_TOKEN(TOKEN_SLASH);
        ADD_TOKEN(TOKEN_STAR);
        ADD_TOKEN(TOKEN_COLON);
        ADD_TOKEN(TOKEN_SEMICOLON);

        ADD_TOKEN(TOKEN_NOT);

        ADD_TOKEN(TOKEN_ASSIGN);

        ADD_TOKEN(TOKEN_EQUAL);
        ADD_TOKEN(TOKEN_NOT_EQUAL);
        ADD_TOKEN(TOKEN_LESS);
        ADD_TOKEN(TOKEN_LESS_OR_EQUAL);
        ADD_TOKEN(TOKEN_GREATER);
        ADD_TOKEN(TOKEN_GREATER_OR_EQUAL);

        ADD_TOKEN(TOKEN_IDENTIFIER);
        ADD_TOKEN(TOKEN_STRING);
        ADD_TOKEN(TOKEN_NUMBER);

        ADD_TOKEN(TOKEN_FOR);
        ADD_TOKEN(TOKEN_IF);
        ADD_TOKEN(TOKEN_ELSE);
        ADD_TOKEN(TOKEN_WHILE);
        ADD_TOKEN(TOKEN_TRUE);
        ADD_TOKEN(TOKEN_FALSE);
        ADD_TOKEN(TOKEN_ARROW);

        ADD_TOKEN(TOKEN_EOF);
    }
#undef ADD_TOKEN
}

internal void
lexer_create(Lexer* lexer, const String_View code)
{
    lexer->code.data = code.data;
    lexer->code.length = code.length;
    lexer->lexeme_start_index = 0;
    lexer->current_index = 0;
    lexer->current_line = 0;
    lexer->current_column = 0;

    const Keyword keywords[] = {
        { .type = TOKEN_FOR,   .lexeme = string_view("for"), },
        { .type = TOKEN_IF,    .lexeme = string_view("if"), },
        { .type = TOKEN_ELSE,  .lexeme = string_view("else"), },
        { .type = TOKEN_WHILE, .lexeme = string_view("while"), },
        { .type = TOKEN_TRUE,  .lexeme = string_view("true"), },
        { .type = TOKEN_FALSE, .lexeme = string_view("false"), },
        { .type = TOKEN_ARROW, .lexeme = string_view("->"), }
    };

    lexer->keywords_count = size_of(keywords) / size_of(keywords[0]);
    lexer->keywords = (Keyword*) calloc((USize)lexer->keywords_count, sizeof(Keyword));
    for (Index i = 0;
         i < lexer->keywords_count;
         ++i)
    {
        lexer->keywords[i] = keywords[i];
    }
}

internal inline char
lexer_peek(Lexer* lexer)
{
    ASSERT(lexer->current_index < lexer->code.length);
    return lexer->code.data[lexer->current_index];
}

// TODO(vlad): Change the return type to 'void'?
internal inline Bool
lexer_advance(Lexer* lexer)
{
    if (lexer->current_index < lexer->code.length)
    {
        lexer->current_index += 1;
        lexer->current_column += 1;
        return true;
    }

    return false;
}

internal inline char
lexer_get_current_character_and_advance(Lexer* lexer)
{
    ASSERT(lexer->current_index < lexer->code.length);
    char current_char = lexer_peek(lexer);
    lexer_advance(lexer);
    return current_char;
}

internal Bool
lexer_match_and_optionally_advance(Lexer* lexer, const char expected_char)
{
    if (lexer->current_index == lexer->code.length) { return false; }
    if (lexer_peek(lexer) != expected_char) { return false; }

    lexer_advance(lexer);
    return true;
}

internal inline void
lexer_create_token(Lexer* lexer, Token* token, const Token_Type type)
{
    token->type = type;
    token->lexeme = (String_View) {
        .data   = lexer->code.data + lexer->lexeme_start_index,
        .length = lexer->current_index - lexer->lexeme_start_index,
    };
    token->line = lexer->current_line;
    token->column = lexer->current_column - token->lexeme.length;
}

internal inline Bool
lexer_is_letter(const char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

internal inline Bool
lexer_is_digit(const char c)
{
    return ('0' <= c && c <= '9');
}

internal Bool
lexer_get_next_token(Lexer* lexer, Token* token)
{
    if (lexer->current_index >= lexer->code.length)
    {
        lexer_create_token(lexer, token, TOKEN_EOF);
        return true;
    }

    ASSERT(lexer->current_index < lexer->code.length);

    // FIXME(vlad): Remove 'lexeme_start_index' from Lexer and make this a local variable.
    lexer->lexeme_start_index = lexer->current_index;

    while (lexer->current_index < lexer->code.length)
    {
        // FIXME(vlad): Do not advance here?
        const char current_char = lexer_get_current_character_and_advance(lexer);

        if (lexer_is_letter(current_char) || current_char == '_')
        {
            while (lexer->current_index < lexer->code.length)
            {
                const char lookahead_char = lexer_peek(lexer);

                if (lexer_is_letter(lookahead_char)
                    || lexer_is_digit(lookahead_char)
                    || lookahead_char == '_')
                {
                    lexer_advance(lexer);
                }
                else
                {
                    break;
                }
            }

            lexer_create_token(lexer, token, TOKEN_IDENTIFIER);

            for (Index i = 0;
                 i < lexer->keywords_count;
                 ++i)
            {
                const Keyword* keyword = &lexer->keywords[i];
                if (strings_are_equal(token->lexeme, keyword->lexeme))
                {
                    token->type = keyword->type;
                    break;
                }
            }

            return true;
        }

        if (lexer_is_digit(current_char))
        {
            while (lexer->current_index < lexer->code.length)
            {
                const char lookahead_char = lexer_peek(lexer);

                if (lexer_is_digit(lookahead_char))
                {
                    lexer_advance(lexer);
                }
                else
                {
                    break;
                }
            }

            lexer_create_token(lexer, token, TOKEN_NUMBER);
            return true;
        }

        switch (current_char)
        {
            case '(': { lexer_create_token(lexer, token, TOKEN_LEFT_PAREN); return true; } break;
            case ')': { lexer_create_token(lexer, token, TOKEN_RIGHT_PAREN); return true; } break;
            case '{': { lexer_create_token(lexer, token, TOKEN_LEFT_BRACE); return true; } break;
            case '}': { lexer_create_token(lexer, token, TOKEN_RIGHT_BRACE); return true; } break;
            case '[': { lexer_create_token(lexer, token, TOKEN_LEFT_BRACKET); return true; } break;
            case ']': { lexer_create_token(lexer, token, TOKEN_RIGHT_BRACKET); return true; } break;
            case ':': { lexer_create_token(lexer, token, TOKEN_COLON); return true; } break;
            case ';': { lexer_create_token(lexer, token, TOKEN_SEMICOLON); return true; } break;
            case ',': { lexer_create_token(lexer, token, TOKEN_COMMA); return true; } break;
            case '+': { lexer_create_token(lexer, token, TOKEN_PLUS); return true; } break;
            case '*': { lexer_create_token(lexer, token, TOKEN_STAR); return true; } break;

            case '!':
            {
                if (lexer_match_and_optionally_advance(lexer, '='))
                {
                    lexer_create_token(lexer, token, TOKEN_NOT_EQUAL);
                }
                else
                {
                    lexer_create_token(lexer, token, TOKEN_NOT);
                }
                return true;
            } break;

            case '=':
            {
                if (lexer_match_and_optionally_advance(lexer, '='))
                {
                    lexer_create_token(lexer, token, TOKEN_EQUAL);
                }
                else
                {
                    lexer_create_token(lexer, token, TOKEN_ASSIGN);
                }
                return true;
            } break;

            case '-':
            {
                if (lexer_match_and_optionally_advance(lexer, '>'))
                {
                    lexer_create_token(lexer, token, TOKEN_ARROW);
                }
                else
                {
                    lexer_create_token(lexer, token, TOKEN_MINUS);
                }
                return true;
            } break;

            case '<':
            {
                if (lexer_match_and_optionally_advance(lexer, '='))
                {
                    lexer_create_token(lexer, token, TOKEN_LESS_OR_EQUAL);
                }
                else
                {
                    lexer_create_token(lexer, token, TOKEN_LESS);
                }
                return true;
            } break;

            case '>':
            {
                if (lexer_match_and_optionally_advance(lexer, '='))
                {
                    lexer_create_token(lexer, token, TOKEN_GREATER_OR_EQUAL);
                }
                else
                {
                    lexer_create_token(lexer, token, TOKEN_GREATER);
                }
                return true;
            } break;

            case '"':
            {
                // XXX(vlad): Remove multiline strings support?
                while (lexer->current_index < lexer->code.length)
                {
                    if (lexer_match_and_optionally_advance(lexer, '"'))
                    {
                        lexer_create_token(lexer, token, TOKEN_STRING);
                        return true;
                    }
                    else if (lexer_match_and_optionally_advance(lexer, '\n'))
                    {
                        lexer->current_column = 0; // TODO(vlad): 1?
                        lexer->current_line += 1;
                    }

                    lexer_advance(lexer);
                }
            } break;

            case '/':
            {
                if (lexer_match_and_optionally_advance(lexer, '/'))
                {
                    // NOTE(vlad): Single line comment, skipping this line.
                    while (lexer->current_index < lexer->code.length)
                    {
                        const char lookahead_char = lexer_peek(lexer);

                        if (lookahead_char != '\n')
                        {
                            lexer_advance(lexer);
                        }
                        else
                        {
                            lexer->lexeme_start_index = lexer->current_index + 1;
                            break;
                        }
                    }
                }
                else if (lexer_match_and_optionally_advance(lexer, '*'))
                {
                    // NOTE(vlad): Block comment.
                    Size comment_depth = 1;

                    while (lexer->current_index < lexer->code.length)
                    {
                        if (lexer_match_and_optionally_advance(lexer, '*')
                            && lexer_match_and_optionally_advance(lexer, '/'))
                        {
                            comment_depth -= 1;
                            if (comment_depth == 0)
                            {
                                break;
                            }
                        }

                        if (lexer_match_and_optionally_advance(lexer, '/')
                            && lexer_match_and_optionally_advance(lexer, '*'))
                        {
                            comment_depth += 1;
                        }

                        lexer_advance(lexer);
                    }
                }
                else
                {
                    lexer_create_token(lexer, token, TOKEN_SLASH);
                    return true;
                }
            } break;

            case ' ':
            case '\r':
            case '\t':
            {
                lexer->lexeme_start_index = lexer->current_index;
                // NOTE(vlad): Ignoring whitespaces.
            } break;

            case '\n':
            {
                lexer->current_column = 0;
                lexer->current_line += 1;
                lexer->lexeme_start_index = lexer->current_index;
            } break;

            default:
            {
                log_print_code_line_with_highlighting(string_view("Error: unexpected character encountered: "),
                                                      lexer->code,
                                                      lexer->current_column - 1,
                                                      1);
                println("Current column: {}", lexer->current_column);
                lexer->lexeme_start_index = lexer->current_index;

                // FIXME(vlad): Optionally trigger debug a trap?
                // FAIL();

                // TODO(vlad): Try to recover from this error?
                return false;
            } break;
        }
    }

    lexer_create_token(lexer, token, TOKEN_EOF);
    return true;
}

internal void
lexer_destroy(Lexer* lexer)
{
    if (lexer->keywords)
    {
        free(lexer->keywords);
    }
}
