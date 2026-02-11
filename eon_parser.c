#include "eon_parser.h"

#include <eon/io.h>

// FIXME(vlad): Report syntax errors here.

internal String_View
parser_token_type_to_string(const Token_Type type)
{
#define ADD_TOKEN(type, string) case type: return string_view(string)
    switch (type)
    {
        ADD_TOKEN(TOKEN_UNDEFINED, "undefined token");

        ADD_TOKEN(TOKEN_LEFT_PAREN, "(");
        ADD_TOKEN(TOKEN_RIGHT_PAREN, ")");
        ADD_TOKEN(TOKEN_LEFT_BRACE, "{");
        ADD_TOKEN(TOKEN_RIGHT_BRACE, "}");
        ADD_TOKEN(TOKEN_LEFT_BRACKET, "[");
        ADD_TOKEN(TOKEN_RIGHT_BRACKET, "]");

        ADD_TOKEN(TOKEN_COMMA, ",");
        ADD_TOKEN(TOKEN_DOT, ".");
        ADD_TOKEN(TOKEN_MINUS, "-");
        ADD_TOKEN(TOKEN_PLUS, "+");
        ADD_TOKEN(TOKEN_SLASH, "/");
        ADD_TOKEN(TOKEN_STAR, "*");
        ADD_TOKEN(TOKEN_COLON, ":");
        ADD_TOKEN(TOKEN_SEMICOLON, ";");

        ADD_TOKEN(TOKEN_NOT, "!");

        ADD_TOKEN(TOKEN_ASSIGN, "=");

        ADD_TOKEN(TOKEN_EQUAL, "==");
        ADD_TOKEN(TOKEN_NOT_EQUAL, "!=");
        ADD_TOKEN(TOKEN_LESS, "<");
        ADD_TOKEN(TOKEN_LESS_OR_EQUAL, "<=");
        ADD_TOKEN(TOKEN_GREATER, ">");
        ADD_TOKEN(TOKEN_GREATER_OR_EQUAL, ">=");

        ADD_TOKEN(TOKEN_IDENTIFIER, "identifier");
        ADD_TOKEN(TOKEN_STRING, "string literal");
        ADD_TOKEN(TOKEN_NUMBER, "number");

        ADD_TOKEN(TOKEN_FOR, "for");
        ADD_TOKEN(TOKEN_IF, "if");
        ADD_TOKEN(TOKEN_ELSE, "else");
        ADD_TOKEN(TOKEN_WHILE, "while");
        ADD_TOKEN(TOKEN_TRUE, "true");
        ADD_TOKEN(TOKEN_FALSE, "false");
        ADD_TOKEN(TOKEN_ARROW, "->");
        ADD_TOKEN(TOKEN_RETURN, "return");

        ADD_TOKEN(TOKEN_EOF, "end of file");
    }
#undef ADD_TOKEN
}

internal Bool
parser_get_next_token(Parser* parser)
{
    if (parser->current_token.type != TOKEN_UNDEFINED)
    {
        // NOTE(vlad): Current token was not consumed yet.
        return true;
    }

    if (!lexer_get_next_token(parser->lexer, &parser->current_token, &parser->errors))
    {
        return false;
    }

    return true;
}

// TODO(vlad): Remove this and use 'parser_try_to_consume_token_with_type' only?
internal inline void
parser_consume_token(Parser* parser)
{
    parser->current_token = (Token){0};
}

internal Bool
parser_ensure_that_current_token_has_type(Parser* parser, const Token_Type expected_type)
{
    ASSERT(parser->current_token.type != TOKEN_UNDEFINED && "Current token is undefined");

    if (parser->current_token.type != expected_type)
    {
        Error error = {0};
        error.filename = parser->current_token.filename;
        error.line = parser->current_token.line;
        error.column = parser->current_token.column;
        error.highlight_length = parser->current_token.lexeme.length;
        error.code = parser->lexer->code;

        // TODO(vlad): Use scratch arena instead?
        const String message = format_string(parser->errors.errors_arena,
                                             "Expected {}, found {}",
                                             parser_token_type_to_string(expected_type),
                                             parser_token_type_to_string(parser->current_token.type));
        error.message = string_view(message);

        add_error(&parser->errors, &error);

        return false;
    }

    return true;
}

internal Bool
parser_try_to_consume_token_with_type(Parser* parser, const Token_Type expected_type)
{
    if (!parser_ensure_that_current_token_has_type(parser, expected_type))
    {
        return false;
    }

    parser_consume_token(parser);
    return true;
}

internal Bool
parser_get_and_consume_token_with_type(Parser* parser,
                                       const Token_Type expected_type)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (!parser_try_to_consume_token_with_type(parser, expected_type))
    {
        return false;
    }

    return true;
}

internal void
parser_create(Arena* parser_arena, Arena* errors_arena, Parser* parser, Lexer* lexer)
{
    parser->errors = (Errors){0};
    parser->errors.errors_arena = errors_arena;

    parser->lexer = lexer;
    parser->current_token = (Token){0};

    const Builtin_Type builtin_types[] = {
        { .lexeme = string_view("void"), .type = AST_TYPE_VOID, },
        { .lexeme = string_view("Int32"), .type = AST_TYPE_INT_32, },
    };
    const Size builtin_types_count = size_of(builtin_types) / size_of(builtin_types[0]);

    parser->builtin_types = allocate_uninitialized_array(parser_arena,
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
parse_identifier(Parser* parser, Ast_Identifier* identifier)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (!parser_ensure_that_current_token_has_type(parser, TOKEN_IDENTIFIER))
    {
        return false;
    }

    identifier->token = parser->current_token;
    parser_consume_token(parser);

    return true;
}

internal Bool parse_function_type(Arena* parser_arena, Parser* parser, Ast_Type* type);
internal Bool parse_pointer_type(Arena* parser_arena, Parser* parser, Ast_Type* type);

internal Bool
parse_type(Arena* parser_arena,
           Parser* parser,
           Ast_Type* type)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    // TODO(vlad): Change to 'parser_try_to_consume_token_with_type'?
    switch (parser->current_token.type)
    {
        case TOKEN_LEFT_PAREN:
        {
            return parse_function_type(parser_arena, parser, type);
        } break;

        case TOKEN_STAR:
        {
            return parse_pointer_type(parser_arena, parser, type);
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
            // TODO(vlad): Ensure that token has one of these types and report an error if it does not.
            println("Failed to parse type");
            return false;
        };
    }
}

internal Bool
parse_argument_declaration(Arena* parser_arena,
                           Parser* parser,
                           Ast_Function_Argument* argument)
{
    if (!parse_identifier(parser, &argument->name))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    argument->type = allocate(parser_arena, Ast_Type);
    return parse_type(parser_arena, parser, argument->type);
}

internal Bool
parse_arguments_declaration(Arena* parser_arena,
                            Parser* parser,
                            Ast_Function_Arguments* arguments)
{
    while (true)
    {
        Ast_Function_Argument argument = {0};
        if (!parse_argument_declaration(parser_arena, parser, &argument))
        {
            return false;
        }

        if (arguments->arguments_count == arguments->arguments_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const Size new_capacity = MAX(1, 2 * arguments->arguments_capacity);
            arguments->arguments = reallocate(parser_arena,
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
parse_function_type(Arena* parser_arena,
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
        if (!parse_arguments_declaration(parser_arena, parser, &type->arguments))
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

    type->return_type = allocate(parser_arena, Ast_Type);
    return parse_type(parser_arena, parser, type->return_type);
}

internal Bool
parse_pointer_type(Arena* parser_arena, Parser* parser, Ast_Type* type)
{
    type->type = AST_TYPE_POINTER;

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_STAR))
    {
        return false;
    }

    type->pointed_to = allocate(parser_arena, Ast_Type);
    return parse_type(parser_arena, parser, type->pointed_to);
}

internal Bool parse_expression(Arena* parser_arena, Parser* parser, Ast_Expression* expression);

internal Bool
parse_arguments(Arena* parser_arena, Parser* parser, Ast_Call* call)
{
    while (true)
    {
        Ast_Expression* argument = allocate(parser_arena, Ast_Expression);
        if (!parse_expression(parser_arena, parser, argument))
        {
            return false;
        }

        if (call->arguments_count == call->arguments_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const Size new_capacity = MAX(1, 2 * call->arguments_capacity);
            call->arguments = reallocate(parser_arena,
                                                  call->arguments,
                                                  Ast_Expression*,
                                                  call->arguments_capacity,
                                                  new_capacity);
            call->arguments_capacity = new_capacity;
        }

        call->arguments[call->arguments_count] = argument;
        call->arguments_count += 1;

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
parse_primary_expression(Arena* parser_arena, Parser* parser, Ast_Expression* expression)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_NUMBER:
        {
            expression->type = AST_EXPRESSION_NUMBER;
            expression->number.token = parser->current_token;
            parser_consume_token(parser);
            return true;
        } break;

        case TOKEN_STRING:
        {
            expression->type = AST_EXPRESSION_STRING_LITERAL;
            expression->string_literal.token = parser->current_token;

            expression->string_literal.value = expression->string_literal.token.lexeme;
            expression->string_literal.value.data += 1;
            expression->string_literal.value.length -= 2;

            parser_consume_token(parser);
            return true;
        } break;

        case TOKEN_IDENTIFIER:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        {
            expression->type = AST_EXPRESSION_IDENTIFIER;
            expression->identifier.token = parser->current_token;
            parser_consume_token(parser);
            return true;
        } break;

        case TOKEN_LEFT_PAREN:
        {
            parser_consume_token(parser);
            if (!parse_expression(parser_arena, parser, expression))
            {
                return false;
            }
            parser_get_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN);
            return true;
        } break;

        default:
        {
            // TODO(vlad): Ensure that token has one of these types and report an error if it does not.
            println("Failed to parse primary expression");
            return false;
        } break;
    }
}

internal Bool
parse_call_expression(Arena* parser_arena, Parser* parser, Ast_Expression* expression)
{
    if (!parse_primary_expression(parser_arena, parser, expression))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    // TODO(vlad): Test that the primary expression is callable. Although we should
    //             probably do it after parsing.

    if (parser->current_token.type != TOKEN_LEFT_PAREN)
    {
        return true;
    }

    parser_consume_token(parser);

    Ast_Expression* called_expression = allocate(parser_arena, Ast_Expression);
    *called_expression = *expression;

    expression->type = AST_EXPRESSION_CALL;
    expression->call = (Ast_Call){0};

    Ast_Call* call = &expression->call;
    call->called_expression = called_expression;

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_RIGHT_PAREN)
    {
        if (!parse_arguments(parser_arena, parser, call))
        {
            return false;
        }
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN))
    {
        return false;
    }

    return true;
}

internal Bool
parse_multiplicative_expression(Arena* parser_arena, Parser* parser, Ast_Expression* expression)
{
    // TODO(vlad): Use 'expression' here, otherwise we would overallocate.
    Ast_Expression* lhs = allocate(parser_arena, Ast_Expression);
    if (!parse_call_expression(parser_arena, parser, lhs))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_STAR:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser_arena, Ast_Expression);
            if (!parse_multiplicative_expression(parser_arena, parser, rhs))
            {
                return false;
            }

            expression->type = AST_EXPRESSION_MULTIPLY;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_SLASH:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser_arena, Ast_Expression);
            if (!parse_multiplicative_expression(parser_arena, parser, rhs))
            {
                return false;
            }

            expression->type = AST_EXPRESSION_DIVIDE;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        default:
        {
            *expression = *lhs;
            return true;
        }
    }
}

internal Bool
parse_additive_expression(Arena* parser_arena, Parser* parser, Ast_Expression* expression)
{
    // TODO(vlad): Use 'expression' here, otherwise we would overallocate.
    Ast_Expression* lhs = allocate(parser_arena, Ast_Expression);
    if (!parse_multiplicative_expression(parser_arena, parser, lhs))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_PLUS:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser_arena, Ast_Expression);
            if (!parse_additive_expression(parser_arena, parser, rhs))
            {
                return false;
            }

            expression->type = AST_EXPRESSION_ADD;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_MINUS:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser_arena, Ast_Expression);
            if (!parse_additive_expression(parser_arena, parser, rhs))
            {
                return false;
            }

            expression->type = AST_EXPRESSION_SUBTRACT;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        default:
        {
            *expression = *lhs;
            return true;
        }
    }
}

internal Bool
parse_comparison_expression(Arena* parser_arena, Parser* parser, Ast_Expression* expression)
{
    // TODO(vlad): Use 'expression' here, otherwise we would overallocate.
    Ast_Expression* lhs = allocate(parser_arena, Ast_Expression);
    if (!parse_additive_expression(parser_arena, parser, lhs))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_EQUAL:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser_arena, Ast_Expression);
            if (!parse_comparison_expression(parser_arena, parser, rhs))
            {
                return false;
            }

            expression->type = AST_EXPRESSION_EQUAL;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_NOT_EQUAL:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser_arena, Ast_Expression);
            if (!parse_comparison_expression(parser_arena, parser, rhs))
            {
                return false;
            }

            expression->type = AST_EXPRESSION_NOT_EQUAL;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        default:
        {
            *expression = *lhs;
            return true;
        }
    }
}

internal Bool
parse_expression(Arena* parser_arena, Parser* parser, Ast_Expression* expression)
{
    return parse_comparison_expression(parser_arena, parser, expression);
}

internal Bool parse_statements(Arena* parser_arena, Parser* parser, Ast_Statements* statements);

internal Bool
parse_optional_variable_assignment(Arena* parser_arena, Parser* parser, Ast_Variable_Definition* definition)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser->current_token.type == TOKEN_ASSIGN)
    {
        parser_consume_token(parser);

        definition->initialisation_type = AST_INITIALISATION_WITH_VALUE;

        if (!parse_expression(parser_arena, parser, &definition->initial_value))
        {
            return false;
        }
    }
    else
    {
        definition->initialisation_type = AST_INITIALISATION_DEFAULT;
    }

    return true;
}

internal Bool
parse_variable_definition(Arena* parser_arena,
                          Parser* parser,
                          Ast_Identifier* identifier,
                          Ast_Variable_Definition* definition)
{
    definition->name = *identifier;

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    definition->type = allocate(parser_arena, Ast_Type);
    if (parser->current_token.type != TOKEN_ASSIGN)
    {
        if (!parse_type(parser_arena, parser, definition->type))
        {
            return false;
        }
    }
    else
    {
        definition->type->type = AST_TYPE_DEDUCED;
    }

    if (!parse_optional_variable_assignment(parser_arena, parser, definition))
    {
        return false;
    }

    // TODO(vlad): Move the semicolon parsing to 'parse_statement' or 'parse_expression_statement'
    //             or something more high-level.
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_assignment(Arena* parser_arena,
                 Parser* parser,
                 Ast_Identifier* identifier,
                 Ast_Assignment* assignment)
{
    assignment->name = *identifier;

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_ASSIGN))
    {
        return false;
    }

    if (!parse_expression(parser_arena, parser, &assignment->expression))
    {
        return false;
    }

    // TODO(vlad): Move the semicolon parsing to 'parse_statement' or 'parse_expression_statement'
    //             or something more high-level.
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_call_statement(Arena* parser_arena,
                     Parser* parser,
                     Ast_Identifier* identifier,
                     Ast_Call_Statement* call_statement)
{
    Ast_Call* call = &call_statement->call;

    call->called_expression = allocate(parser_arena, Ast_Expression);
    call->called_expression->type = AST_EXPRESSION_IDENTIFIER;
    call->called_expression->identifier = *identifier;

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
        if (!parse_arguments(parser_arena, parser, call))
        {
            return false;
        }
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN))
    {
        return false;
    }

    // TODO(vlad): Move the semicolon parsing to 'parse_statement' or 'parse_expression_statement'
    //             or something more high-level.
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_variable_assignment_or_definition_or_call(Arena* parser_arena,
                                                Parser* parser,
                                                Ast_Statement* statement)
{
    Ast_Identifier identifier = {0};
    if (!parse_identifier(parser, &identifier))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_COLON:
        {
            statement->type = AST_STATEMENT_VARIABLE_DEFINITION;
            return parse_variable_definition(parser_arena, parser, &identifier, &statement->variable_definition);
        } break;

        case TOKEN_ASSIGN:
        {
            statement->type = AST_STATEMENT_ASSIGNMENT;
            return parse_assignment(parser_arena, parser, &identifier, &statement->assignment);
        } break;

        case TOKEN_LEFT_PAREN:
        {
            statement->type = AST_STATEMENT_CALL;
            return parse_call_statement(parser_arena,
                                        parser,
                                        &identifier,
                                        &statement->call_statement);
        } break;

        default:
        {
            FAIL("Failed to parse variable definition or assignment");
        } break;
    }
}

internal Bool
parse_return_statement(Arena* parser_arena, Parser* parser, Ast_Return_Statement* statement)
{
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_RETURN))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    // TODO(vlad): Use 'parser_try_to_consume_token_with_type'? Note that this function
    //             emits an error if token type is not equal to the given one. We don't want
    //             to do that here.
    if (parser->current_token.type == TOKEN_SEMICOLON)
    {
        parser_consume_token(parser);
        statement->is_empty = true;
        return true;
    }

    statement->is_empty = false;

    if (!parse_expression(parser_arena, parser, &statement->expression))
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
parse_code_block(Arena* parser_arena, Parser* parser, Ast_Statements* statements)
{
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_LEFT_BRACE))
    {
        return false;
    }

    if (!parse_statements(parser_arena, parser, statements))
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
parse_while_statement(Arena* parser_arena, Parser* parser, Ast_While_Statement* while_statement)
{
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_WHILE))
    {
        return false;
    }

    if (!parse_expression(parser_arena, parser, &while_statement->condition))
    {
        return false;
    }

    if (!parse_code_block(parser_arena, parser, &while_statement->statements))
    {
        return false;
    }

    return true;
}

internal Bool
parse_if_statement(Arena* parser_arena, Parser* parser, Ast_If_Statement* if_statement)
{
    if (!parser_get_and_consume_token_with_type(parser, TOKEN_IF))
    {
        return false;
    }

    if (!parse_expression(parser_arena, parser, &if_statement->condition))
    {
        return false;
    }

    if (!parse_code_block(parser_arena, parser, &if_statement->if_statements))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_ELSE)
    {
        return true;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_ELSE))
    {
        return false;
    }

    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_IF:
        {
            Ast_Statements* else_statements = &if_statement->else_statements;
            else_statements->statements = allocate(parser_arena, Ast_Statement);
            else_statements->statements_count = 1;
            else_statements->statements_capacity = 1;

            Ast_Statement* next_branch_statement = &else_statements->statements[0];
            next_branch_statement->type = AST_STATEMENT_IF;

            if (!parse_if_statement(parser_arena, parser, &next_branch_statement->if_statement))
            {
                return false;
            }
        } break;

        case TOKEN_LEFT_BRACE:
        {
            if (!parse_code_block(parser_arena, parser, &if_statement->else_statements))
            {
                return false;
            }
        } break;

        default:
        {
            FAIL("Expected 'if' or code block");
        } break;
    }

    return true;
}

internal Bool
parse_statement(Arena* parser_arena, Parser* parser, Ast_Statement* statement)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_IDENTIFIER:
        {
            return parse_variable_assignment_or_definition_or_call(parser_arena, parser, statement);
        } break;

        case TOKEN_RETURN:
        {
            statement->type = AST_STATEMENT_RETURN;
            return parse_return_statement(parser_arena, parser, &statement->return_statement);
        } break;

        case TOKEN_WHILE:
        {
            statement->type = AST_STATEMENT_WHILE;
            return parse_while_statement(parser_arena, parser, &statement->while_statement);
        } break;

        case TOKEN_IF:
        {
            statement->type = AST_STATEMENT_IF;
            return parse_if_statement(parser_arena, parser, &statement->if_statement);
        } break;

        default:
        {
            // TODO(vlad): Ensure that token has one of these types and report an error if it does not.
            println("Failed to parse statement");
            return false;
        } break;
    }
}

internal Bool
parse_statements(Arena* parser_arena, Parser* parser, Ast_Statements* statements)
{
    if (!parser_get_next_token(parser))
    {
        return false;
    }

    while (parser->current_token.type != TOKEN_RIGHT_BRACE)
    {
        Ast_Statement statement = {0};
        if (!parse_statement(parser_arena, parser, &statement))
        {
            return false;
        }

        if (statements->statements_count == statements->statements_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const Size new_capacity = MAX(1, 2 * statements->statements_capacity);
            statements->statements = reallocate(parser_arena,
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
parse_function_definition(Arena* parser_arena,
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

    function_definition->type = allocate(parser_arena, Ast_Type);
    if (!parse_function_type(parser_arena, parser, function_definition->type))
    {
        return false;
    }

    if (!parser_get_and_consume_token_with_type(parser, TOKEN_ASSIGN))
    {
        return false;
    }

    if (!parse_code_block(parser_arena, parser, &function_definition->statements))
    {
        return false;
    }

    return true;
}

internal Bool
parser_parse(Arena* parser_arena, Parser* parser, Ast* ast)
{
    do
    {
        Ast_Function_Definition function_definition = {0};
        if (!parse_function_definition(parser_arena, parser, &function_definition))
        {
            println("Failed to parse function definition");
            return false;
        }

        if (ast->function_definitions_count == ast->function_definitions_capacity)
        {
            // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
            const Size new_capacity = MAX(1, 2 * ast->function_definitions_capacity);
            ast->function_definitions = reallocate(parser_arena,
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
    // XXX(vlad): Clear errors here? Probably not.
    UNUSED(parser);
}
