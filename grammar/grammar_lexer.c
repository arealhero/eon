#include "grammar_lexer.h"

#include "grammar_log.h"

internal inline bool32
is_letter(const char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

internal inline bool32
is_digit(const char c)
{
    return '0' <= c && c <= '9';
}

internal inline char
lexer_peek(Lexer* lexer)
{
    ASSERT(lexer->current_index < lexer->grammar.length);
    return lexer->grammar.data[lexer->current_index];
}

internal inline char
lexer_lookahead(Lexer* lexer)
{
    if (lexer->current_index + 1 < lexer->grammar.length)
    {
        return lexer->grammar.data[lexer->current_index + 1];
    }

    return '\0';
}

// TODO(vlad): Change the return type to 'void'?
internal inline bool32
lexer_advance(Lexer* lexer)
{
    if (lexer->current_index < lexer->grammar.length)
    {
        if (lexer_peek(lexer) == '\n')
        {
            lexer->current_line += 1;
            lexer->current_column = 0;
        }
        else
        {
            lexer->current_column += 1;
        }

        lexer->current_index += 1;
        return true;
    }

    return false;
}

internal void
lexer_create_token(Lexer* lexer,
                   Token* token,
                   const ssize lexeme_start_index,
                   const Token_Type type)
{
    token->type = type;
    token->lexeme.data = lexer->grammar.data + lexeme_start_index;
    token->lexeme.length = lexer->current_index - lexeme_start_index + 1;
    token->line = lexer->current_line;
    token->column = lexer->current_column - token->lexeme.length + 1;

    lexer_advance(lexer);
}

internal void
lexer_create(Lexer* lexer, String_View grammar)
{
    lexer->grammar = grammar;

    lexer->current_index = 0;
    lexer->current_line = 0;
    lexer->current_column = 0;
}

internal bool32
lexer_get_next_token(Arena* scratch, Lexer* lexer, Token* token)
{
    if (lexer->current_index >= lexer->grammar.length)
    {
        lexer_create_token(lexer, token, lexer->current_index, TOKEN_EOF);
        return true;
    }

    while (lexer->current_index < lexer->grammar.length)
    {
        const ssize lexeme_start_index = lexer->current_index;
        const char current_char = lexer_peek(lexer);

        if (is_letter(current_char))
        {
            // NOTE(vlad): Parsing identifier.
            while (lexer->current_index < lexer->grammar.length)
            {
                const char lookahead_char = lexer_lookahead(lexer);

                if (is_letter(lookahead_char)
                    || is_digit(lookahead_char)
                    || lookahead_char == '_')
                {
                    ASSERT(lexer_advance(lexer));
                }
                else
                {
                    break;
                }
            }

            lexer_create_token(lexer, token, lexeme_start_index, TOKEN_NON_TERMINAL);
            return true;
        }

        switch (current_char)
        {
            case '|': { lexer_create_token(lexer, token, lexeme_start_index, TOKEN_OR); return true; } break;
            case ':': { lexer_create_token(lexer, token, lexeme_start_index, TOKEN_COLON); return true; } break;
            case ';': { lexer_create_token(lexer, token, lexeme_start_index, TOKEN_SEMICOLON); return true; } break;

            case '/':
            {
                const char lookahead_char = lexer_lookahead(lexer);

                if (lookahead_char == '/')
                {
                    // NOTE(vlad): Line comment encountered, skipping until EOF or a new line.
                    while (lexer_lookahead(lexer) != '\n' && lexer_lookahead(lexer) != '\0')
                    {
                        lexer_advance(lexer);
                    }

                    if (lexer_lookahead(lexer) == '\n')
                    {
                        lexer_advance(lexer);
                        lexer_advance(lexer);
                    }
                }
                else
                {
                    printf("ERROR HERE: Unknown character encountered: '%c'\n", current_char);
                    show_grammar_error(scratch,
                                       lexer->grammar,
                                       lexer->current_line,
                                       lexer->current_column,
                                       1);
                    return false;
                }
            } break;

            case '"':
            {
                const ssize terminal_start_line = lexer->current_line;
                const ssize terminal_start_column = lexer->current_column;

                // TODO(vlad): Support escaped quotes.
                char lookahead_char = lexer_lookahead(lexer);
                while (lookahead_char != '"'
                       && lookahead_char != '\0'
                       && lookahead_char != '\n')
                {
                    lexer_advance(lexer);
                    lookahead_char = lexer_lookahead(lexer);
                }

                if (lookahead_char == '\0' || lookahead_char == '\n')
                {
                    printf("ERROR: Terminal was not closed\n");
                    show_grammar_error(scratch,
                                       lexer->grammar,
                                       terminal_start_line,
                                       terminal_start_column,
                                       1);
                    return false;
                }

                lexer_advance(lexer);
                lexer_create_token(lexer, token, lexeme_start_index, TOKEN_TERMINAL);
                return true;
            } break;

            case ' ':
            case '\n':
            {
                lexer_advance(lexer);
            } break;

            default:
            {
                printf("ERROR THERE: Unknown character encountered: '%c'\n", current_char);
                show_grammar_error(scratch,
                                   lexer->grammar,
                                   lexer->current_line,
                                   lexer->current_column,
                                   1);
                return false;
            } break;
        }
    }

    lexer_create_token(lexer, token, lexer->current_index, TOKEN_EOF);
    return true;
}

internal void
lexer_destroy(Lexer* lexer)
{
    UNUSED(lexer);
}
