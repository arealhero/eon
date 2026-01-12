#include "eon_parser.h"

maybe_unused internal String_View
convert_expression_to_prn(Arena* const restrict arena,
                          const Expression* const restrict expression)
{
    switch (expression->type)
    {
        case EXPRESSION_UNDEFINED:
        {
            FAIL();
        } break;

        case EXPRESSION_BINARY:
        {
            Binary_Expression* binary = expression->binary;

            const String_View lhs = convert_expression_to_prn(arena, binary->lhs);
            const String_View rhs = convert_expression_to_prn(arena, binary->rhs);
            return string_view(format_string(arena, "{} {} {}", lhs, rhs, binary->operator.lexeme));
        } break;

        case EXPRESSION_UNARY:
        {
            Unary_Expression* unary = expression->unary;

            const String_View unary_expression = convert_expression_to_prn(arena, unary->expression);
            return string_view(format_string(arena, "{} {}", unary_expression, unary->operator.lexeme));
        } break;

        case EXPRESSION_LITERAL:
        {
            Literal_Expression* literal = expression->literal;
            return string_view(format_string(arena, "{}", literal->token.lexeme));
        } break;

        case EXPRESSION_GROUPING:
        {
            Grouping_Expression* grouping = expression->grouping;
            return convert_expression_to_prn(arena, grouping->expression);
        } break;
    }
}

maybe_unused internal void
expression_destroy(Expression* expression)
{
    switch (expression->type)
    {
        case EXPRESSION_UNDEFINED:
        {
            FAIL();
        } break;

        case EXPRESSION_BINARY:
        {
            Binary_Expression* binary = expression->binary;

            expression_destroy(binary->lhs);
            expression_destroy(binary->rhs);

            free(binary);
        } break;

        case EXPRESSION_UNARY:
        {
            Unary_Expression* unary = expression->unary;

            expression_destroy(unary->expression);

            free(unary);
        } break;

        case EXPRESSION_LITERAL:
        {
            Literal_Expression* literal = expression->literal;

            free(literal);
        } break;

        case EXPRESSION_GROUPING:
        {
            Grouping_Expression* grouping = expression->grouping;

            expression_destroy(grouping->expression);

            free(grouping);
        } break;
    }

    free(expression);
}

internal void
parser_advance(Parser* parser)
{
    parser->previous_token = parser->current_token;

    if (!lexer_get_next_token(parser->lexer, &parser->current_token))
    {
        parser->current_token = (Token){0};
        parser->current_token.type = TOKEN_EOF;
    }
}

internal Bool
parser_match_and_optionally_advance(Parser* parser, const Token_Type type)
{
    if (parser->current_token.type == type)
    {
        parser_advance(parser);
        return true;
    }

    return false;
}

internal void parser_parse_expression(Parser* parser, Expression* expression);

internal void
parser_parse_primary(Parser* parser, Expression* expression)
{
    if (parser_match_and_optionally_advance(parser, TOKEN_TRUE)
        || parser_match_and_optionally_advance(parser, TOKEN_FALSE)
        || parser_match_and_optionally_advance(parser, TOKEN_IDENTIFIER)
        || parser_match_and_optionally_advance(parser, TOKEN_NUMBER))
    {
        expression->type = EXPRESSION_LITERAL;
        expression->literal = calloc(1, sizeof(Literal_Expression));
        expression->literal->token = parser->previous_token;
    }
    else if (parser_match_and_optionally_advance(parser, TOKEN_LEFT_PAREN))
    {
        Expression* expression_in_parens = calloc(1, sizeof(Expression));
        parser_parse_expression(parser, expression_in_parens);

        if (!parser_match_and_optionally_advance(parser, TOKEN_RIGHT_PAREN))
        {
            ASSERT(0 && "Expected ')' after expression");
        }

        expression->type = EXPRESSION_GROUPING;
        expression->grouping = calloc(1, sizeof(Grouping_Expression));
        expression->grouping->expression = expression_in_parens;
    }
    else
    {
        ASSERT(0 && "Unsupported token found");
    }
}

internal void
parser_parse_unary(Parser* parser, Expression* expression)
{
    if (parser_match_and_optionally_advance(parser, TOKEN_NOT)
        || parser_match_and_optionally_advance(parser, TOKEN_PLUS)
        || parser_match_and_optionally_advance(parser, TOKEN_MINUS))
    {
        Token operator = parser->previous_token;

        Expression* subexpression = calloc(1, sizeof(Expression));
        parser_parse_unary(parser, subexpression);

        expression->type = EXPRESSION_UNARY;
        expression->unary = calloc(1, sizeof(Unary_Expression));
        expression->unary->operator = operator;
        expression->unary->expression = subexpression;
    }
    else
    {
        parser_parse_primary(parser, expression);
    }
}

internal void
parser_parse_factor(Parser* parser, Expression* expression)
{
    parser_parse_unary(parser, expression);

    while (parser_match_and_optionally_advance(parser, TOKEN_SLASH)
           || parser_match_and_optionally_advance(parser, TOKEN_STAR))
    {
        Token operator = parser->previous_token;

        Expression* rhs = calloc(1, sizeof(Expression));
        parser_parse_unary(parser, rhs);

        Expression* lhs = calloc(1, sizeof(Expression));
        copy_memory(as_bytes(lhs), as_bytes(expression), sizeof(Expression));

        expression->type = EXPRESSION_BINARY;
        expression->binary = calloc(1, sizeof(Binary_Expression));
        expression->binary->lhs = lhs;
        expression->binary->operator = operator;
        expression->binary->rhs = rhs;
    }
}

internal void
parser_parse_term(Parser* parser, Expression* expression)
{
    parser_parse_factor(parser, expression);

    while (parser_match_and_optionally_advance(parser, TOKEN_MINUS)
           || parser_match_and_optionally_advance(parser, TOKEN_PLUS))
    {
        Token operator = parser->previous_token;

        Expression* rhs = calloc(1, sizeof(Expression));
        parser_parse_factor(parser, rhs);

        Expression* lhs = calloc(1, sizeof(Expression));
        copy_memory(as_bytes(lhs), as_bytes(expression), sizeof(Expression));

        expression->type = EXPRESSION_BINARY;
        expression->binary = calloc(1, sizeof(Binary_Expression));
        expression->binary->lhs = lhs;
        expression->binary->operator = operator;
        expression->binary->rhs = rhs;
    }
}

internal void
parser_parse_comparison(Parser* parser, Expression* expression)
{
    parser_parse_term(parser, expression);

    while (parser_match_and_optionally_advance(parser, TOKEN_LESS)
           || parser_match_and_optionally_advance(parser, TOKEN_LESS_OR_EQUAL)
           || parser_match_and_optionally_advance(parser, TOKEN_GREATER)
           || parser_match_and_optionally_advance(parser, TOKEN_GREATER_OR_EQUAL))
    {
        Token operator = parser->previous_token;

        Expression* rhs = calloc(1, sizeof(Expression));
        parser_parse_term(parser, rhs);

        Expression* lhs = calloc(1, sizeof(Expression));
        copy_memory(as_bytes(lhs), as_bytes(expression), sizeof(Expression));

        expression->type = EXPRESSION_BINARY;
        expression->binary = calloc(1, sizeof(Binary_Expression));
        expression->binary->lhs = lhs;
        expression->binary->operator = operator;
        expression->binary->rhs = rhs;
    }
}

internal void
parser_parse_equality(Parser* parser, Expression* expression)
{
    parser_parse_comparison(parser, expression);

    while (parser_match_and_optionally_advance(parser, TOKEN_EQUAL)
           || parser_match_and_optionally_advance(parser, TOKEN_NOT_EQUAL))
    {
        Token operator = parser->previous_token;

        Expression* rhs = calloc(1, sizeof(Expression));
        parser_parse_comparison(parser, rhs);

        Expression* lhs = calloc(1, sizeof(Expression));
        copy_memory(as_bytes(lhs), as_bytes(expression), sizeof(Expression));

        expression->type = EXPRESSION_BINARY;
        expression->binary = calloc(1, sizeof(Binary_Expression));
        expression->binary->lhs = lhs;
        expression->binary->operator = operator;
        expression->binary->rhs = rhs;
    }
}

internal void
parser_parse_expression(Parser* parser, Expression* expression)
{
    parser_parse_equality(parser, expression);
}

maybe_unused internal void
parser_create(Parser* parser, Lexer* lexer)
{
    parser->lexer = lexer;
    parser->previous_token = (Token){0};
    parser->current_token = (Token){0};
}

maybe_unused internal Expression*
parser_parse(Parser* parser)
{
    parser_advance(parser);

    Expression* expression = calloc(1, sizeof(Expression));
    parser_parse_expression(parser, expression);
    return expression;
}

maybe_unused internal void
parser_destroy(Parser* parser)
{
    UNUSED(parser);
}
