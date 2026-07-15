#include "eon_lexer.h"

#include "eon_compilation_context.h"

#include <eon/io.h>

internal void
create_lexer(Lexer* lexer, Compilation_Context* context)
{
    lexer->context = context;
    lexer->code = context->source_file.code;
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
        { .type = TOKEN_ARROW, .lexeme = string_view("->"), },
        { .type = TOKEN_RETURN, .lexeme = string_view("return"), },
        { .type = TOKEN_BREAK, .lexeme = string_view("break"), },
        { .type = TOKEN_CONTINUE, .lexeme = string_view("continue"), },
        { .type = TOKEN_WILDCARD, .lexeme = string_view("_"), },

        { .type = TOKEN_MUTABLE, .lexeme = string_view("mutable"), },
    };

    lexer->keywords_count = NUMBER_OF_STATIC_ARRAY_ELEMENTS(keywords);
    lexer->keywords = allocate_array(context->keywords_arena, lexer->keywords_count, Keyword);
    for (Index i = 0;
         i < lexer->keywords_count;
         ++i)
    {
        lexer->keywords[i] = keywords[i];
    }
}

internal inline char
get_current_character(Lexer* lexer)
{
    ASSERT(lexer->current_index < lexer->code.length);
    return lexer->code.data[lexer->current_index];
}

// TODO(vlad): Change the return type to 'void'?
internal inline Bool
move_to_next_character(Lexer* lexer)
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
consume_current_character(Lexer* lexer)
{
    ASSERT(lexer->current_index < lexer->code.length);
    char current_char = get_current_character(lexer);
    move_to_next_character(lexer);
    return current_char;
}

internal Bool
consume_current_character_if_matched(Lexer* lexer, const char expected_char)
{
    if (lexer->current_index == lexer->code.length) { return false; }
    if (get_current_character(lexer) != expected_char) { return false; }

    move_to_next_character(lexer);
    return true;
}

internal inline void
create_token(Lexer* lexer, Token* token, const Token_Type type)
{
    const Size lexeme_length = lexer->current_index - lexer->lexeme_start_index;

    token->type = type;
    token->lexeme = (String_View) {
        .data   = lexer->code.data + lexer->lexeme_start_index,
        .length = lexeme_length,
    };
    token->location.offset_in_bytes = lexer->lexeme_start_index;
    token->location.length_in_bytes = lexeme_length;
    token->location.line = lexer->current_line;
    token->location.column = lexer->current_column - token->lexeme.length;
}

internal inline Bool
is_a_part_of_identifier_tail(const char c)
{
    return is_ascii_letter(c)
        || is_ascii_digit(c)
        || c == '_';
}

internal inline Bool
is_not_a_newline(const char c)
{
    return c != '\n';
}

typedef Bool (*Character_Predicate)(const char lookahead_char);

internal void
consume_characters_while_preciate_is_true(Lexer* lexer,
                                          const Character_Predicate predicate)
{
    while (lexer->current_index < lexer->code.length)
    {
        const char lookahead_char = get_current_character(lexer);

        if (predicate(lookahead_char))
        {
            move_to_next_character(lexer);
        }
        else
        {
            break;
        }
    }
}

internal Bool
get_next_token(Lexer* lexer, Token* token)
{
    if (lexer->current_index >= lexer->code.length)
    {
        lexer->lexeme_start_index = lexer->current_index;
        create_token(lexer, token, TOKEN_EOF);
        return true;
    }

    ASSERT(lexer->current_index < lexer->code.length);

    // FIXME(vlad): Remove 'lexeme_start_index' from Lexer and make this a local variable.
    lexer->lexeme_start_index = lexer->current_index;

    while (lexer->current_index < lexer->code.length)
    {
        // FIXME(vlad): Do not advance here?
        const char current_char = consume_current_character(lexer);

        if (is_ascii_letter(current_char) || current_char == '_')
        {
            consume_characters_while_preciate_is_true(lexer,
                                                      is_a_part_of_identifier_tail);

            create_token(lexer, token, TOKEN_IDENTIFIER);

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

        if (is_ascii_digit(current_char))
        {
            consume_characters_while_preciate_is_true(lexer, is_ascii_digit);

            if (lexer->current_index < lexer->code.length
                && get_current_character(lexer) == '.')
            {
                move_to_next_character(lexer);
                consume_characters_while_preciate_is_true(lexer, is_ascii_digit);
            }

            create_token(lexer, token, TOKEN_NUMBER);
            return true;
        }

        switch (current_char)
        {
            case '(': { create_token(lexer, token, TOKEN_LEFT_PAREN); return true; } break;
            case ')': { create_token(lexer, token, TOKEN_RIGHT_PAREN); return true; } break;
            case '{': { create_token(lexer, token, TOKEN_LEFT_BRACE); return true; } break;
            case '}': { create_token(lexer, token, TOKEN_RIGHT_BRACE); return true; } break;
            case '[': { create_token(lexer, token, TOKEN_LEFT_BRACKET); return true; } break;
            case ']': { create_token(lexer, token, TOKEN_RIGHT_BRACKET); return true; } break;
            case ':': { create_token(lexer, token, TOKEN_COLON); return true; } break;
            case ';': { create_token(lexer, token, TOKEN_SEMICOLON); return true; } break;
            case ',': { create_token(lexer, token, TOKEN_COMMA); return true; } break;
            case '+': { create_token(lexer, token, TOKEN_PLUS); return true; } break;
            case '*': { create_token(lexer, token, TOKEN_STAR); return true; } break;
            case '&': { create_token(lexer, token, TOKEN_AMPERSAND); return true; } break;

            case '!':
            {
                if (consume_current_character_if_matched(lexer, '='))
                {
                    create_token(lexer, token, TOKEN_NOT_EQUAL);
                }
                else
                {
                    create_token(lexer, token, TOKEN_NOT);
                }
                return true;
            } break;

            case '=':
            {
                if (consume_current_character_if_matched(lexer, '='))
                {
                    create_token(lexer, token, TOKEN_EQUAL);
                }
                else
                {
                    create_token(lexer, token, TOKEN_ASSIGN);
                }
                return true;
            } break;

            case '-':
            {
                if (consume_current_character_if_matched(lexer, '>'))
                {
                    create_token(lexer, token, TOKEN_ARROW);
                }
                else
                {
                    create_token(lexer, token, TOKEN_MINUS);
                }
                return true;
            } break;

            case '<':
            {
                if (consume_current_character_if_matched(lexer, '='))
                {
                    create_token(lexer, token, TOKEN_LESS_OR_EQUAL);
                }
                else
                {
                    create_token(lexer, token, TOKEN_LESS);
                }
                return true;
            } break;

            case '>':
            {
                if (consume_current_character_if_matched(lexer, '='))
                {
                    create_token(lexer, token, TOKEN_GREATER_OR_EQUAL);
                }
                else
                {
                    create_token(lexer, token, TOKEN_GREATER);
                }
                return true;
            } break;

            case '"':
            {
                // XXX(vlad): Remove multiline strings support?
                while (lexer->current_index < lexer->code.length)
                {
                    if (consume_current_character_if_matched(lexer, '"'))
                    {
                        create_token(lexer, token, TOKEN_STRING);
                        return true;
                    }
                    else if (consume_current_character_if_matched(lexer, '\n'))
                    {
                        lexer->current_column = 0; // TODO(vlad): 1?
                        lexer->current_line += 1;
                    }

                    move_to_next_character(lexer);
                }
            } break;

            case '/':
            {
                if (consume_current_character_if_matched(lexer, '/'))
                {
                    // NOTE(vlad): Single line comment, skipping this line.
                    consume_characters_while_preciate_is_true(lexer,
                                                              is_not_a_newline);
                    lexer->lexeme_start_index = lexer->current_index + 1;
                }
                else if (consume_current_character_if_matched(lexer, '*'))
                {
                    // NOTE(vlad): Block comment.
                    Size comment_depth = 1;

                    while (lexer->current_index < lexer->code.length)
                    {
                        if (consume_current_character_if_matched(lexer, '*')
                            && consume_current_character_if_matched(lexer, '/'))
                        {
                            comment_depth -= 1;
                            if (comment_depth == 0)
                            {
                                break;
                            }
                        }

                        if (consume_current_character_if_matched(lexer, '/')
                            && consume_current_character_if_matched(lexer, '*'))
                        {
                            comment_depth += 1;
                        }

                        move_to_next_character(lexer);
                    }
                }
                else
                {
                    create_token(lexer, token, TOKEN_SLASH);
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
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location.offset_in_bytes = lexer->current_index;
                error.location.length_in_bytes = 1;
                error.location.line = lexer->current_line;
                error.location.column = lexer->current_column - 1;
                error.text = string_view("Unexpected character encountered");

                emit_diagnostic_message(lexer->context, &error);

                // TODO(vlad): Do we really need this?
                lexer->lexeme_start_index = lexer->current_index;

                // FIXME(vlad): Optionally trigger a debug trap?
                // FAIL();

                // TODO(vlad): Try to recover from this error?
                return false;
            } break;
        }
    }

    create_token(lexer, token, TOKEN_EOF);
    return true;
}

internal void
destroy_lexer(Lexer* lexer)
{
    UNUSED(lexer);
}
