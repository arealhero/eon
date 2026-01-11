#include "eon_parser.h"

internal inline char
parser_get_lookahead(Parser* parser)
{
    return parser->expression[parser->lookahead_index];
}

internal void
parser_show_error(Parser* parser)
{
    printf("\n%s\n", parser->expression);
    for (ssize i = 0;
         i < parser->lookahead_index;
         ++i)
    {
        printf(" ");
    }
    printf("^\n");
}

bool32
parser_parse_match(Parser* parser, const char symbol)
{
    if (parser->lookahead_index >= parser->expression_length)
    {
        parser_show_error(parser);
        printf("Error: expected '%c', found end of expression", symbol);
        return false;
    }

    if (parser_get_lookahead(parser) != symbol)
    {
        parser_show_error(parser);
        printf("Lookahead %c does not match symbol %c\n",
               parser_get_lookahead(parser),
               symbol);

        return false;
    }

    parser->lookahead_index += 1;
    return true;
}

bool32
parser_parse_term(Parser* parser)
{
    const char lookahead = parser_get_lookahead(parser);
    if ('0' <= lookahead && lookahead <= '9')
    {
        printf("%c", lookahead);
        return parser_parse_match(parser, lookahead);
    }

    parser_show_error(parser);
    printf("Error: expected digit, found ");
    if (lookahead == '\0')
    {
        puts("EOF");
    }
    else
    {
        printf("'%c'\n", lookahead);
    }

    return false;
}

bool32
parser_parse_expr(Parser* parser)
{
    if (!parser_parse_term(parser)) { return false; }
    while (true)
    {
        const char lookahead = parser_get_lookahead(parser);
        if (lookahead == '+')
        {
            if (!parser_parse_match(parser, '+')) { return false; }
            if (!parser_parse_term(parser)) { return false; }
            printf("+");
        }
        else if (lookahead == '-')
        {
            if (!parser_parse_match(parser, '-')) { return false; }
            if (!parser_parse_term(parser)) { return false; }
            printf("-");
        }
        else
        {
            if (lookahead != '\0')
            {
                parser_show_error(parser);
                printf("\nError: expected EOF, found '%c'\n", lookahead);
                return false;
            }

            return true;
        }
    }

    return true;
}

internal void
parser_create(Parser* parser, const char* expression)
{
    parser->expression = expression;
    parser->expression_length = (ssize)strlen(expression);
    parser->lookahead_index = 0;
}

internal bool32
parser_parse_expression(Parser* parser)
{
    return parser_parse_expr(parser);
}

internal void
parser_destroy(Parser* parser)
{
    (void) parser;
}
