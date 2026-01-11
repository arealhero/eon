#include "grammar_parser.h"

#include "grammar_log.h"

internal bool32
parser_get_next_token(Arena* scratch, Parser* parser)
{
    if (parser->current_token.type != TOKEN_UNDEFINED)
    {
        // NOTE(vlad): Current token was not consumed yet.
        return true;
    }

    if (!lexer_get_next_token(scratch, parser->lexer, &parser->current_token))
    {
        return false;
    }

    // printf("Token found: type = %d, lexeme: '%.*s', column = %ld\n",
    //        parser->current_token.type, FORMAT_STRING(parser->current_token.lexeme),
    //        parser->current_token.column);

    return true;
}

internal inline void
parser_consume_token(Parser* parser)
{
    parser->current_token = (Token){0};
}

internal bool32
parser_try_to_consume_token_with_type(Parser* parser,
                                      const Token_Type expected_type)
{
    ASSERT(parser->current_token.type != TOKEN_UNDEFINED && "Tried to consume undefined token.");

    if (parser->current_token.type == expected_type)
    {
        parser_consume_token(parser);
        return true;
    }

    return false;
}

internal bool32
parser_get_and_consume_token_with_type(Arena* scratch,
                                       Parser* parser,
                                       const Token_Type expected_type)
{
    if (!parser_get_next_token(scratch, parser))
    {
        return false;
    }

    if (parser_try_to_consume_token_with_type(parser, expected_type))
    {
        return true;
    }

    printf("Parser error: Expected type %d, found %d\n",
           expected_type, parser->current_token.type);

    return false;
}

internal bool32
parse_identifier(Arena* scratch, Parser* parser, Ast_Identifier* identifier)
{
    if (!parser_get_next_token(scratch, parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_TERMINAL
        && parser->current_token.type != TOKEN_NON_TERMINAL)
    {
        return false;
    }

    identifier->token = parser->current_token;
    parser_consume_token(parser);

    return true;
}

internal bool32
parse_identifier_expressions(Arena* arena,
                             Arena* scratch,
                             Parser* parser,
                             Ast_Identifier_Definition* definition)
{
    while (true)
    {
        Ast_Expression current_expression = {0};

        // NOTE(vlad): Trying to parse next identifier in the expression.
        while (true)
        {
            Ast_Identifier next_identifier = {0};
            if (!parse_identifier(scratch, parser, &next_identifier))
            {
                break;
            }

            if (current_expression.identifiers_count == current_expression.identifiers_capacity)
            {
                const ssize new_capacity = MAX(1, 2 * current_expression.identifiers_capacity);

                const ssize element_size = size_of(current_expression.identifiers[0]);
                const ssize old_size = current_expression.identifiers_capacity * element_size;
                const ssize new_size = new_capacity * element_size;

                current_expression.identifiers = arena_reallocate(arena,
                                                                  as_bytes(current_expression.identifiers),
                                                                  old_size,
                                                                  new_size);
                current_expression.identifiers_capacity = new_capacity;
            }

            current_expression.identifiers[current_expression.identifiers_count] = next_identifier;
            current_expression.identifiers_count += 1;
        }

        const bool32 has_next_possible_expression = parser_try_to_consume_token_with_type(parser, TOKEN_OR);
        const bool32 all_expressions_parsed = !has_next_possible_expression
            && parser_try_to_consume_token_with_type(parser, TOKEN_SEMICOLON);

        if (has_next_possible_expression || all_expressions_parsed)
        {
            if (definition->possible_expressions_count == definition->possible_expressions_capacity)
            {
                const ssize new_capacity = MAX(1, 2 * definition->possible_expressions_capacity);

                const ssize element_size = size_of(definition->possible_expressions[0]);
                const ssize old_size = definition->possible_expressions_capacity * element_size;
                const ssize new_size = new_capacity * element_size;

                definition->possible_expressions = arena_reallocate(arena,
                                                                    as_bytes(definition->possible_expressions),
                                                                    old_size,
                                                                    new_size);
                definition->possible_expressions_capacity = new_capacity;
            }

            definition->possible_expressions[definition->possible_expressions_count] = current_expression;
            definition->possible_expressions_count += 1;

            if (has_next_possible_expression)
            {
                continue;
            }

            if (all_expressions_parsed)
            {
                return true;
            }

            ASSERT(0 && "Unreachable");
        }

        printf("Parser error: Token with unexpected type %d encountered while parsing a production rule\n",
               parser->current_token.type);

        return false;
    }
}

internal bool32
parse_identifier_definition(Arena* arena,
                            Arena* scratch,
                            Parser* parser,
                            Ast_Identifier_Definition* definition)
{
    if (!parse_identifier(scratch, parser, &definition->identifier))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(scratch, parser, TOKEN_COLON))
    {
        return false;
    }

    return parse_identifier_expressions(arena, scratch, parser, definition);
}

internal void
parser_create(Parser* parser, Lexer* lexer)
{
    parser->lexer = lexer;
    parser->current_token = (Token){0};
}

internal bool32
parser_parse(Arena* arena,
             Arena* scratch,
             Parser* parser,
             Ast* ast)
{
    do
    {
        Ast_Identifier_Definition definition = {0};
        if (!parse_identifier_definition(arena, scratch, parser, &definition))
        {
            printf("Unexpected error encountered.\n");
            return false;
        }

        if (ast->definitions_count == ast->definitions_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const ssize new_capacity = MAX(1, 2 * ast->definitions_capacity);

            const ssize element_size = size_of(ast->definitions[0]);
            const ssize old_size = ast->definitions_capacity * element_size;
            const ssize new_size = new_capacity * element_size;

            ast->definitions = arena_reallocate(arena,
                                                as_bytes(ast->definitions),
                                                old_size,
                                                new_size);
            ast->definitions_capacity = new_capacity;
        }

        ast->definitions[ast->definitions_count] = definition;
        ast->definitions_count += 1;

        // NOTE(vlad): Prefetching the next token to check if its type is EOF.
        parser_get_next_token(scratch, parser);
    }
    while (parser->current_token.type != TOKEN_EOF);

    return true;
}

internal void
parser_destroy(Parser* parser)
{
    UNUSED(parser);
}
