#include <stdio.h>
#include <string.h> // FIXME(vlad): Remove this header.

#include "eon_common.h"
#include "eon_lexer.h"
#include "eon_parser.h"

int
main(const int args_count, const char* args[])
{
    if (args_count != 2)
    {
        puts("usage: eon <expression>");
        return 1;
    }

    const char* expression = args[1];

    Parser parser = {0};
    parser_create(&parser, expression);

    if (parser_parse_expression(&parser))
    {
        puts("");
    }

    parser_destroy(&parser);

    return 0;
}

#include "eon_lexer.c"
#include "eon_parser.c"
