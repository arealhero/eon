#include "eon_parser.h"

#include <eon/io.h>

// FIXME(vlad): Report syntax errors here.

internal Bool
parser_get_next_token(Parser* parser)
{
    if (parser->current_token.type != TOKEN_UNDEFINED)
    {
        // NOTE(vlad): Current token was not consumed yet.
        return true;
    }

    if (!lexer_get_next_token(parser->lexer, &parser->current_token))
    {
        return false;
    }

    return true;
}

internal inline void
parser_consume_token(Parser* parser)
{
    parser->current_token = (Token){0};
}

internal Bool
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

internal Bool
parser_get_and_consume_token_with_type(Parser* parser,
                                       const Token_Type expected_type)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser_try_to_consume_token_with_type(parser, expected_type))
    {
        return true;
    }

    println("Parser error: Expected type {}, found {}",
            expected_type, parser->current_token.type);

    return false;
}

internal void
parser_create(Arena* arena, Parser* parser, Lexer* lexer)
{
    parser->lexer = lexer;
    parser->current_token = (Token){0};

    const Builtin_Type builtin_types[] = {
        { .lexeme = string_view("void"), .type = AST_TYPE_VOID, },
        { .lexeme = string_view("Int32"), .type = AST_TYPE_INT_32, },
    };
    const Size builtin_types_count = size_of(builtin_types) / size_of(builtin_types[0]);

    parser->builtin_types = allocate_uninitialized_array(arena,
                                                         builtin_types_count,
                                                         Builtin_Type);
    parser->builtin_types_count = builtin_types_count;

    for (Index i = 0;
         i < builtin_types_count;
         ++i)
    {
        parser->builtin_types[i] = builtin_types[i];
    }
}

internal Bool
parse_identifier(Parser* parser,
                 Ast_Identifier* identifier)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        println("Expected identifier, got {}", parser->current_token.type);
        // TODO(vlad): Report parsing error.
        FAIL("Expected identifier");
        return false;
    }

    identifier->token = parser->current_token;
    parser_consume_token(parser);

    return true;
}

internal Bool parse_function_type(Arena* arena,
                                  Parser* parser,
                                  Ast_Type* type);

internal Bool
parse_type(Arena* arena,
           Parser* parser,
           Ast_Type* type)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_LEFT_PAREN:
        {
            return parse_function_type(arena, parser, type);
        } break;

        case TOKEN_IDENTIFIER:
        {
            type->type = AST_TYPE_USER_DEFINED;
            type->name.token = parser->current_token;
            parser_consume_token(parser);

            for (Index i = 0;
                 i < parser->builtin_types_count;
                 ++i)
            {
                Builtin_Type* builtin_type = &parser->builtin_types[i];
                if (strings_are_equal(builtin_type->lexeme, type->name.token.lexeme))
                {
                    type->type = builtin_type->type;
                    break;
                }
            }

            return true;
        } break;

        default:
        {
            println("Failed to parse type");
            return false;
        };
    }
}

internal Bool
parse_argument_declaration(Arena* arena,
                           Parser* parser,
                           Ast_Function_Argument* argument)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        return false;
    }

    argument->name.token = parser->current_token;
    parser_consume_token(parser);

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    argument->type = allocate(arena, Ast_Type);
    return parse_type(arena, parser, argument->type);
}

internal Bool
parse_arguments_declaration(Arena* arena,
                            Parser* parser,
                            Ast_Function_Arguments* arguments)
{
    while (true)
    {
        Ast_Function_Argument argument = {0};
        if (!parse_argument_declaration(arena, parser, &argument))
        {
            return false;
        }

        if (arguments->arguments_count == arguments->arguments_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const Size new_capacity = MAX(1, 2 * arguments->arguments_capacity);
            arguments->arguments = reallocate(arena,
                                              arguments->arguments,
                                              Ast_Function_Argument,
                                              arguments->arguments_capacity,
                                              new_capacity);
            arguments->arguments_capacity = new_capacity;
        }

        arguments->arguments[arguments->arguments_count] = argument;
        arguments->arguments_count += 1;

        if (!parser_get_next_token(parser))
        {
            return false;
        }

        if (parser->current_token.type != TOKEN_COMMA)
        {
            return true;
        }

        parser_consume_token(parser);
    }
}

internal Bool
parse_function_type(Arena* arena,
                    Parser* parser,
                    Ast_Type* type)
{
    type->type = AST_TYPE_FUNCTION;

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_LEFT_PAREN))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_RIGHT_PAREN)
    {
        if (!parse_arguments_declaration(arena, parser, &type->arguments))
        {
            return false;
        }
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN))
    {
        return false;
    }

    // XXX(vlad): Make the return type optional for 'main'?
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_ARROW))
    {
        return false;
    }

    type->return_type = allocate(arena, Ast_Type);
    return parse_type(arena, parser, type->return_type);
}

internal Bool
parse_variable_declaration(Arena* arena,
                           Parser* parser,
                           Ast_Variable_Declaration* declaration)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        return false;
    }

    declaration->name.token = parser->current_token;
    parser_consume_token(parser);

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    declaration->type = allocate(arena, Ast_Type);
    if (!parse_type(arena, parser, declaration->type))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_statement(Arena* arena, Parser* parser, Ast_Statement* statement)
{
    statement->type = AST_STATEMENT_VARIABLE_DECLARATION;
    return parse_variable_declaration(arena, parser, &statement->variable_declaration);
}

internal Bool
parse_statements(Arena* arena, Parser* parser, Ast_Statements* statements)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    while (parser->current_token.type != TOKEN_RIGHT_BRACE)
    {
        Ast_Statement statement = {0};
        if (!parse_statement(arena, parser, &statement))
        {
            return false;
        }

        if (statements->statements_count == statements->statements_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const Size new_capacity = MAX(1, 2 * statements->statements_capacity);
            statements->statements = reallocate(arena,
                                                statements->statements,
                                                Ast_Statement,
                                                statements->statements_capacity,
                                                new_capacity);
            statements->statements_capacity = new_capacity;
        }

        statements->statements[statements->statements_count] = statement;
        statements->statements_count += 1;

        if (!parser_get_next_token(parser))
        {
            return false;
        }
    }

    return true;
}

internal Bool
parse_function_body(Arena* arena, Parser* parser, Ast_Statements* statements)
{
    return parse_statements(arena, parser, statements);
}

internal Bool
parse_function_definition(Arena* arena,
                          Parser* parser,
                          Ast_Function_Definition* function_definition)
{
    if (!parse_identifier(parser, &function_definition->name))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    function_definition->type = allocate(arena, Ast_Type);
    if (!parse_function_type(arena, parser, function_definition->type))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_ASSIGN))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_LEFT_BRACE))
    {
        return false;
    }

    if (!parse_function_body(arena, parser, &function_definition->statements))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_RIGHT_BRACE))
    {
        return false;
    }

    return true;
}

internal Bool
parser_parse(Arena* arena, Parser* parser, Ast* ast)
{
    do
    {
        Ast_Function_Definition function_definition = {0};
        if (!parse_function_definition(arena, parser, &function_definition))
        {
            println("Failed to parse function definition.");
            return false;
        }

        if (ast->function_definitions_count == ast->function_definitions_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const Size new_capacity = MAX(1, 2 * ast->function_definitions_capacity);
            ast->function_definitions = reallocate(arena,
                                                   ast->function_definitions,
                                                   Ast_Function_Definition,
                                                   ast->function_definitions_capacity,
                                                   new_capacity);
            ast->function_definitions_capacity = new_capacity;
        }

        ast->function_definitions[ast->function_definitions_count] = function_definition;
        ast->function_definitions_count += 1;

        // NOTE(vlad): Prefetching the next token to check if its type is EOF.
        if (!parser_get_next_token(parser))
        {
            return false;
        }
    }
    while (parser->current_token.type != TOKEN_EOF);

    return true;
}

internal void
parser_destroy(Parser* parser)
{
    UNUSED(parser);
}
