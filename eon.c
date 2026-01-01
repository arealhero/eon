#include <stdio.h>
#include <string.h> // FIXME(vlad): Remove this header.

#include "eon_common.h"
#include "eon_lexer.h"

bool32 parse_expression(const char* expression,
                        const ssize expression_length);

int
main(const int args_count, const char* args[])
{
    if (args_count != 2)
    {
        puts("usage: eon <expression>");
        return 1;
    }

    const char* expression = args[1];
    const ssize expression_length = strlen(expression);

    Lexer lexer = {0};
    lexer_create(&lexer, expression);

    if (parse_expression(expression, expression_length))
    {
        puts("");
    }

    lexer_destroy(&lexer);

    return 0;
}

struct Parser
{
    const char* expression;
    ssize expression_length;

    ssize lookahead_index;
};
typedef struct Parser Parser;

internal bool32 parse_term(Parser* parser);
internal bool32 parse_expr(Parser* parser);
internal bool32 parse_match(Parser* parser, const char symbol);
internal inline char parser_get_lookahead(Parser* parser);

internal void parser_show_error(Parser* parser);

bool32
parse_expression(const char* expression,
                 const ssize expression_length)
{
    Parser parser = {0};
    parser.expression = expression;
    parser.expression_length = expression_length;

    printf("Parsing expression '%s'\n", expression);

    return parse_expr(&parser);
}

bool32
parse_term(Parser* parser)
{
    const char lookahead = parser_get_lookahead(parser);
    if ('0' <= lookahead && lookahead <= '9')
    {
        printf("%c", lookahead);
        return parse_match(parser, lookahead);
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
parse_expr(Parser* parser)
{
    if (!parse_term(parser)) { return false; }
    while (true)
    {
        const char lookahead = parser_get_lookahead(parser);
        if (lookahead == '+')
        {
            if (!parse_match(parser, '+')) { return false; }
            if (!parse_term(parser)) { return false; }
            printf("+");
        }
        else if (lookahead == '-')
        {
            if (!parse_match(parser, '-')) { return false; }
            if (!parse_term(parser)) { return false; }
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

bool32
parse_match(Parser* parser, const char symbol)
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

#include "eon_lexer.c"
