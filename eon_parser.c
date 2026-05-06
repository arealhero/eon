#include "eon_parser.h"

#include <eon/io.h>

// FIXME(vlad): Report syntax errors here.

// TODO(vlad): Move to 'eon_lexer.h'?
internal String_View
token_type_to_string(const Token_Type type)
{
#define ADD_TOKEN(type, string) case type: return string_view(string)
    switch (type)
    {
        ADD_TOKEN(TOKEN_UNDEFINED, "UNDEFINED TOKEN");

        ADD_TOKEN(TOKEN_LEFT_PAREN, "'('");
        ADD_TOKEN(TOKEN_RIGHT_PAREN, "')'");
        ADD_TOKEN(TOKEN_LEFT_BRACE, "'{'");
        ADD_TOKEN(TOKEN_RIGHT_BRACE, "'}'");
        ADD_TOKEN(TOKEN_LEFT_BRACKET, "'['");
        ADD_TOKEN(TOKEN_RIGHT_BRACKET, "']'");

        ADD_TOKEN(TOKEN_COMMA, "','");
        ADD_TOKEN(TOKEN_DOT, "'.'");
        ADD_TOKEN(TOKEN_MINUS, "'-'");
        ADD_TOKEN(TOKEN_PLUS, "'+'");
        ADD_TOKEN(TOKEN_SLASH, "'/'");
        ADD_TOKEN(TOKEN_STAR, "'*'");
        ADD_TOKEN(TOKEN_COLON, "':'");
        ADD_TOKEN(TOKEN_SEMICOLON, "';'");

        ADD_TOKEN(TOKEN_NOT, "'!'");
        ADD_TOKEN(TOKEN_AMPERSAND, "'&'");

        ADD_TOKEN(TOKEN_ASSIGN, "'='");

        ADD_TOKEN(TOKEN_EQUAL, "'=='");
        ADD_TOKEN(TOKEN_NOT_EQUAL, "'!='");
        ADD_TOKEN(TOKEN_LESS, "'<'");
        ADD_TOKEN(TOKEN_LESS_OR_EQUAL, "'<='");
        ADD_TOKEN(TOKEN_GREATER, "'>'");
        ADD_TOKEN(TOKEN_GREATER_OR_EQUAL, "'>='");

        ADD_TOKEN(TOKEN_IDENTIFIER, "identifier");
        ADD_TOKEN(TOKEN_STRING, "string literal");
        ADD_TOKEN(TOKEN_NUMBER, "number");

        ADD_TOKEN(TOKEN_FOR, "for");
        ADD_TOKEN(TOKEN_IF, "if");
        ADD_TOKEN(TOKEN_ELSE, "else");
        ADD_TOKEN(TOKEN_WHILE, "while");
        ADD_TOKEN(TOKEN_TRUE, "true");
        ADD_TOKEN(TOKEN_FALSE, "false");
        ADD_TOKEN(TOKEN_ARROW, "'->'");
        ADD_TOKEN(TOKEN_RETURN, "return");
        ADD_TOKEN(TOKEN_WILDCARD, "wildcard ('_')");

        ADD_TOKEN(TOKEN_MUTABLE, "mutable");

        ADD_TOKEN(TOKEN_EOF, "end of file");
    }
#undef ADD_TOKEN
}

internal Bool
parser_fetch_token(Parser* parser)
{
    if (parser->current_token.type != TOKEN_UNDEFINED)
    {
        // NOTE(vlad): Current token was not consumed yet.
        return true;
    }

    if (!get_next_token(parser->lexer, &parser->current_token))
    {
        return false;
    }

    return true;
}

internal Bool
parser_fetch_lookahead_token(Parser* parser)
{
    if (parser->lookahead_token.type != TOKEN_UNDEFINED)
    {
        // NOTE(vlad): Lookahead token was not consumed yet.
        return true;
    }

    if (!get_next_token(parser->lexer, &parser->lookahead_token))
    {
        return false;
    }

    return true;
}

// TODO(vlad): Remove this and use 'parser_try_to_consume_token_with_type' only?
internal inline void
parser_consume_token(Parser* parser)
{
    parser->current_token = parser->lookahead_token;
    parser->lookahead_token = (Token){0};
}

internal Bool
parser_ensure_that_current_token_has_type(Parser* parser, const Token_Type expected_type)
{
    ASSERT(parser->current_token.type != TOKEN_UNDEFINED && "Current token is undefined");

    if (parser->current_token.type != expected_type)
    {
        const Source_Location error_location = parser->current_token.location;
        // FIXME(vlad): Move to 'eon_compilation_context.h' or something.
        const String error_message = format_string(parser->context->error_messages_arena,
                                                   "Expected {}, found {}",
                                                   token_type_to_string(expected_type),
                                                   token_type_to_string(parser->current_token.type));

        emit_error(parser->context, error_location, string_view(error_message));

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
parser_fetch_and_consume_token_with_type(Parser* parser,
                                         const Token_Type expected_type)
{
    if (!parser_fetch_token(parser))
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
create_parser(Parser* parser, Lexer* lexer, Compilation_Context* context)
{
    parser->context = context;

    parser->lexer = lexer;
    parser->current_token = (Token){0};
}

internal Bool
parse_identifier(Parser* parser, Ast_Identifier* identifier)
{
    if (!parser_fetch_token(parser))
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

internal Bool parse_function_type(Parser* parser, Ast_Type* type);
internal Bool parse_pointer_type(Parser* parser, Ast_Type* type);

internal Bool
parse_type(Parser* parser, Ast_Type* type)
{
    if (!parser_fetch_token(parser))
    {
        return false;
    }

    if (parser->current_token.type == TOKEN_MUTABLE)
    {
        type->is_mutable = true;
        parser_consume_token(parser);

        if (!parser_fetch_token(parser))
        {
            return false;
        }
    }

    // TODO(vlad): Change to 'parser_try_to_consume_token_with_type'?
    switch (parser->current_token.type)
    {
        case TOKEN_LEFT_PAREN:
        {
            return parse_function_type(parser, type);
        } break;

        case TOKEN_STAR:
        {
            return parse_pointer_type(parser, type);
        } break;

        case TOKEN_IDENTIFIER:
        case TOKEN_WILDCARD:
        {
            type->kind = AST_TYPE_NAME;
            type->named_type.token = parser->current_token;
            parser_consume_token(parser);
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
parse_parameter_declaration(Parser* parser, Ast_Function_Parameter* parameter)
{
    if (!parse_identifier(parser, &parameter->name))
    {
        return false;
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    parameter->type = allocate(parser->context->ast_arena, Ast_Type);
    parameter->has_default_value = false; // FIXME(vlad): Implement default values for paramters.
    return parse_type(parser, parameter->type);
}

internal Bool
parse_parameters_declaration(Parser* parser, Ast_Function_Type* function)
{
    while (true)
    {
        Ast_Function_Parameter parameter = {0};
        if (!parse_parameter_declaration(parser, &parameter))
        {
            return false;
        }

        grow_array_if_needed(parser->context->ast_arena, function->parameters, Ast_Function_Parameter);
        function->parameters[function->parameters_count++] = parameter;

        if (!parser_fetch_token(parser))
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
parse_function_type(Parser* parser, Ast_Type* type)
{
    type->kind = AST_TYPE_FUNCTION;

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_LEFT_PAREN))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_RIGHT_PAREN)
    {
        if (!parse_parameters_declaration(parser, &type->function))
        {
            return false;
        }
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN))
    {
        return false;
    }

    // XXX(vlad): Make the return type optional for 'main'?
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_ARROW))
    {
        return false;
    }

    type->function.return_type = allocate(parser->context->ast_arena, Ast_Type);
    return parse_type(parser, type->function.return_type);
}

internal Bool
parse_pointer_type(Parser* parser, Ast_Type* type)
{
    type->kind = AST_TYPE_POINTER;

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_STAR))
    {
        return false;
    }

    type->pointer.pointed_to = allocate(parser->context->ast_arena, Ast_Type);
    return parse_type(parser, type->pointer.pointed_to);
}

internal Bool parse_expression(Parser* parser, Ast_Expression* expression);

internal Bool
parse_arguments(Parser* parser, Ast_Call* call)
{
    while (true)
    {
        Ast_Expression* argument = allocate(parser->context->ast_arena, Ast_Expression);
        if (!parse_expression(parser, argument))
        {
            return false;
        }

        grow_array_if_needed(parser->context->ast_arena, call->arguments, Ast_Expression*);
        call->arguments[call->arguments_count++] = argument;

        if (!parser_fetch_token(parser))
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
parse_primary_expression(Parser* parser, Ast_Expression* expression)
{
    if (!parser_fetch_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_NUMBER:
        {
            expression->kind = AST_EXPRESSION_NUMBER;
            expression->number.token = parser->current_token;
            parser_consume_token(parser);
            return true;
        } break;

        case TOKEN_STRING:
        {
            expression->kind = AST_EXPRESSION_STRING_LITERAL;
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
            expression->kind = AST_EXPRESSION_IDENTIFIER;
            expression->identifier.token = parser->current_token;
            parser_consume_token(parser);
            return true;
        } break;

        case TOKEN_LEFT_PAREN:
        {
            parser_consume_token(parser);
            if (!parse_expression(parser, expression))
            {
                return false;
            }

            if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN))
            {
                return false;
            }

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
parse_call_expression(Parser* parser, Ast_Expression* expression)
{
    if (!parse_primary_expression(parser, expression))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    // TODO(vlad): Test that the primary expression is callable. Although we should
    //             probably do it after parsing (e.g. in the type system).

    if (parser->current_token.type != TOKEN_LEFT_PAREN)
    {
        return true;
    }

    parser_consume_token(parser);

    Ast_Expression* called_expression = allocate(parser->context->ast_arena, Ast_Expression);
    *called_expression = *expression;

    expression->kind = AST_EXPRESSION_CALL;
    expression->call = (Ast_Call){0};

    Ast_Call* call = &expression->call;
    call->called_expression = called_expression;

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_RIGHT_PAREN)
    {
        if (!parse_arguments(parser, call))
        {
            return false;
        }
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN))
    {
        return false;
    }

    return true;
}

internal Bool
parse_postfix_unary_expression(Parser* parser, Ast_Expression* expression)
{
    if (!parse_call_expression(parser, expression))
    {
        return false;
    }

    while (true)
    {
        if (!parser_fetch_token(parser))
        {
            return false;
        }

        if (parser->current_token.type != TOKEN_STAR
            && parser->current_token.type != TOKEN_AMPERSAND)
        {
            return true;
        }

        if (!parser_fetch_lookahead_token(parser))
        {
            return false;
        }

        switch (parser->lookahead_token.type)
        {
            case TOKEN_IDENTIFIER:
            case TOKEN_NUMBER:
            case TOKEN_STRING:
            case TOKEN_LEFT_PAREN:
            {
                // NOTE: '*' and '&' can't be unary operators if followed by a '(', identifier, or literal.
                return true;
            } break;

            default:
            {
            } break;
        }

        const Token operator = parser->current_token;
        parser_consume_token(parser);

        Ast_Expression* operand = allocate(parser->context->ast_arena, Ast_Expression);
        *operand = *expression;

        if (operator.type == TOKEN_STAR)
        {
            expression->kind = AST_EXPRESSION_DEREFERENCE;
        }
        else if (operator.type == TOKEN_AMPERSAND)
        {
            expression->kind = AST_EXPRESSION_ADDRESS_OF;
        }
        else
        {
            UNREACHABLE();
        }

        expression->unary_expression.operator = operator;
        expression->unary_expression.operand = operand;
    }
}

internal Bool
parse_prefix_unary_expression(Parser* parser, Ast_Expression* expression)
{
    if (!parser_fetch_token(parser))
    {
        return false;
    }

    if (parser->current_token.type == TOKEN_MINUS)
    {
        const Token operator = parser->current_token;
        parser_consume_token(parser);

        Ast_Expression* operand = allocate(parser->context->ast_arena, Ast_Expression);
        if (!parse_prefix_unary_expression(parser, operand))
        {
            return false;
        }

        expression->kind = AST_EXPRESSION_NEGATE;
        expression->unary_expression.operator = operator;
        expression->unary_expression.operand = operand;
        return true;
    }

    if (!parse_postfix_unary_expression(parser, expression))
    {
        return false;
    }

    return true;
}

internal Bool
parse_multiplicative_expression(Parser* parser, Ast_Expression* expression)
{
    // TODO(vlad): Use 'expression' here, otherwise we would overallocate.
    Ast_Expression* lhs = allocate(parser->context->ast_arena, Ast_Expression);
    if (!parse_prefix_unary_expression(parser, lhs))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_STAR:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_multiplicative_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_MULTIPLY;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_SLASH:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_multiplicative_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_DIVIDE;
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
parse_additive_expression(Parser* parser, Ast_Expression* expression)
{
    // TODO(vlad): Use 'expression' here, otherwise we would overallocate.
    Ast_Expression* lhs = allocate(parser->context->ast_arena, Ast_Expression);
    if (!parse_multiplicative_expression(parser, lhs))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_PLUS:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_additive_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_ADD;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_MINUS:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_additive_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_SUBTRACT;
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
parse_comparison_expression(Parser* parser, Ast_Expression* expression)
{
    // TODO(vlad): Use 'expression' here, otherwise we would overallocate.
    Ast_Expression* lhs = allocate(parser->context->ast_arena, Ast_Expression);
    if (!parse_additive_expression(parser, lhs))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_EQUAL:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_comparison_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_EQUAL;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_NOT_EQUAL:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_comparison_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_NOT_EQUAL;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_LESS:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_comparison_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_LESS;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_LESS_OR_EQUAL:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_comparison_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_LESS_OR_EQUAL;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_GREATER:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_comparison_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_GREATER;
            expression->binary_expression.operator = operator;
            expression->binary_expression.lhs = lhs;
            expression->binary_expression.rhs = rhs;
            return true;
        } break;

        case TOKEN_GREATER_OR_EQUAL:
        {
            const Token operator = parser->current_token;
            parser_consume_token(parser);

            Ast_Expression* rhs = allocate(parser->context->ast_arena, Ast_Expression);
            if (!parse_comparison_expression(parser, rhs))
            {
                return false;
            }

            expression->kind = AST_EXPRESSION_GREATER_OR_EQUAL;
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
parse_expression(Parser* parser, Ast_Expression* expression)
{
    return parse_comparison_expression(parser, expression);
}

// FIXME(vlad): Rename to 'parse_code_block'.
internal Bool parse_statements(Parser* parser, Ast_Code_Block* statements);

internal Bool
parse_optional_variable_assignment(Parser* parser, Ast_Variable_Definition* definition)
{
    if (!parser_fetch_token(parser))
    {
        return false;
    }

    if (parser->current_token.type == TOKEN_ASSIGN)
    {
        parser_consume_token(parser);

        definition->has_initial_value = true;
        if (!parse_expression(parser, &definition->initial_value))
        {
            return false;
        }
    }

    return true;
}

internal Bool
parse_variable_definition(Parser* parser,
                          Ast_Identifier* identifier,
                          Ast_Variable_Definition* definition)
{
    definition->name = *identifier;

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    definition->type = allocate(parser->context->ast_arena, Ast_Type);
    if (parser->current_token.type != TOKEN_ASSIGN)
    {
        if (!parse_type(parser, definition->type))
        {
            return false;
        }
    }
    else
    {
        // FIXME(vlad): Move to 'parse_optional_type()' or something.
        definition->type->kind = AST_TYPE_OMITTED;
    }

    if (!parse_optional_variable_assignment(parser, definition))
    {
        return false;
    }

    // TODO(vlad): Move the semicolon parsing to 'parse_statement' or 'parse_expression_statement'
    //             or something more high-level.
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_assignment(Parser* parser,
                 Ast_Identifier* identifier,
                 Ast_Assignment* assignment)
{
    assignment->name = *identifier;

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_ASSIGN))
    {
        return false;
    }

    if (!parse_expression(parser, &assignment->expression))
    {
        return false;
    }

    // TODO(vlad): Move the semicolon parsing to 'parse_statement' or 'parse_expression_statement'
    //             or something more high-level.
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_call_statement(Parser* parser,
                     Ast_Identifier* identifier,
                     Ast_Call_Statement* call_statement)
{
    Ast_Call* call = &call_statement->call;

    call->called_expression = allocate(parser->context->ast_arena, Ast_Expression);
    call->called_expression->kind = AST_EXPRESSION_IDENTIFIER;
    call->called_expression->identifier = *identifier;

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_LEFT_PAREN))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_RIGHT_PAREN)
    {
        if (!parse_arguments(parser, call))
        {
            return false;
        }
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_RIGHT_PAREN))
    {
        return false;
    }

    // TODO(vlad): Move the semicolon parsing to 'parse_statement' or 'parse_expression_statement'
    //             or something more high-level.
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_variable_assignment_or_definition_or_call(Parser* parser, Ast_Statement* statement)
{
    Ast_Identifier identifier = {0};
    if (!parse_identifier(parser, &identifier))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_COLON:
        {
            statement->type = AST_STATEMENT_VARIABLE_DEFINITION;
            return parse_variable_definition(parser, &identifier, &statement->variable_definition);
        } break;

        case TOKEN_ASSIGN:
        {
            statement->type = AST_STATEMENT_ASSIGNMENT;
            return parse_assignment(parser, &identifier, &statement->assignment);
        } break;

        case TOKEN_LEFT_PAREN:
        {
            statement->type = AST_STATEMENT_CALL;
            return parse_call_statement(parser, &identifier, &statement->call_statement);
        } break;

        default:
        {
            FAIL("Failed to parse variable definition or assignment");
        } break;
    }
}

internal Bool
parse_return_statement(Parser* parser, Ast_Return_Statement* statement)
{
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_RETURN))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
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

    if (!parse_expression(parser, &statement->expression))
    {
        return false;
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_SEMICOLON))
    {
        return false;
    }

    return true;
}

internal Bool
parse_code_block(Parser* parser, Ast_Code_Block* statements)
{
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_LEFT_BRACE))
    {
        return false;
    }

    if (!parse_statements(parser, statements))
    {
        return false;
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_RIGHT_BRACE))
    {
        return false;
    }

    return true;
}

internal Bool
parse_while_statement(Parser* parser, Ast_While_Statement* while_statement)
{
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_WHILE))
    {
        return false;
    }

    if (!parse_expression(parser, &while_statement->condition))
    {
        return false;
    }

    if (!parse_code_block(parser, &while_statement->body))
    {
        return false;
    }

    return true;
}

internal Bool
parse_if_statement(Parser* parser, Ast_If_Statement* if_statement)
{
    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_IF))
    {
        return false;
    }

    if (!parse_expression(parser, &if_statement->condition))
    {
        return false;
    }

    if (!parse_code_block(parser, &if_statement->if_statements))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    if (parser->current_token.type != TOKEN_ELSE)
    {
        return true;
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_ELSE))
    {
        return false;
    }

    if (!parser_fetch_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_IF:
        {
            Ast_Code_Block* else_statements = &if_statement->else_statements;
            else_statements->statements = allocate(parser->context->ast_arena, Ast_Statement);
            else_statements->statements_count = 1;
            else_statements->statements_capacity = 1;

            Ast_Statement* next_branch_statement = &else_statements->statements[0];
            next_branch_statement->type = AST_STATEMENT_IF;

            if (!parse_if_statement(parser, &next_branch_statement->if_statement))
            {
                return false;
            }
        } break;

        case TOKEN_LEFT_BRACE:
        {
            if (!parse_code_block(parser, &if_statement->else_statements))
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
parse_statement(Parser* parser, Ast_Statement* statement)
{
    if (!parser_fetch_token(parser))
    {
        return false;
    }

    switch (parser->current_token.type)
    {
        case TOKEN_IDENTIFIER:
        {
            return parse_variable_assignment_or_definition_or_call(parser, statement);
        } break;

        case TOKEN_RETURN:
        {
            statement->type = AST_STATEMENT_RETURN;
            return parse_return_statement(parser, &statement->return_statement);
        } break;

        case TOKEN_WHILE:
        {
            statement->type = AST_STATEMENT_WHILE;
            return parse_while_statement(parser, &statement->while_statement);
        } break;

        case TOKEN_IF:
        {
            statement->type = AST_STATEMENT_IF;
            return parse_if_statement(parser, &statement->if_statement);
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
parse_statements(Parser* parser, Ast_Code_Block* statements)
{
    if (!parser_fetch_token(parser))
    {
        return false;
    }

    while (parser->current_token.type != TOKEN_RIGHT_BRACE)
    {
        Ast_Statement statement = {0};
        if (!parse_statement(parser, &statement))
        {
            return false;
        }

        grow_array_if_needed(parser->context->ast_arena, statements->statements, Ast_Statement);
        statements->statements[statements->statements_count++] = statement;

        if (!parser_fetch_token(parser))
        {
            return false;
        }
    }

    return true;
}

internal Bool
parse_function_definition(Parser* parser, Ast_Function_Definition* function_definition)
{
    if (!parse_identifier(parser, &function_definition->name))
    {
        return false;
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_COLON))
    {
        return false;
    }

    function_definition->type = allocate(parser->context->ast_arena, Ast_Type);
    if (!parse_function_type(parser, function_definition->type))
    {
        return false;
    }

    if (!parser_fetch_and_consume_token_with_type(parser, TOKEN_ASSIGN))
    {
        return false;
    }

    if (!parse_code_block(parser, &function_definition->body))
    {
        return false;
    }

    return true;
}

internal Bool
parse_ast(Parser* parser)
{
    Ast* ast = &parser->context->ast;

    do
    {
        Ast_Function_Definition function_definition = {0};
        if (!parse_function_definition(parser, &function_definition))
        {
            println("Failed to parse function definition");
            return false;
        }

        grow_array_if_needed(parser->context->ast_arena, ast->function_definitions, Ast_Function_Definition);
        ast->function_definitions[ast->function_definitions_count++] = function_definition;

        // NOTE(vlad): Prefetching the next token to check if its type is EOF.
        if (!parser_fetch_token(parser))
        {
            return false;
        }
    }
    while (parser->current_token.type != TOKEN_EOF);

    return true;
}

internal void
destroy_parser(Parser* parser)
{
    // XXX(vlad): Clear errors here? Probably not.
    UNUSED(parser);
}
