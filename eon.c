#include <stdio.h> // NOTE(vlad): This include is needed because 'readline' does not include it
                   //             itself for some stupid reason.
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <eon/common.h>
#include <eon/io.h>
#include <eon/memory.h>
#include <eon/string.h>

#include "eon_lexer.h"
#include "eon_log.h"
#include "eon_parser.h"

int
main(void)
{
    init_io_state(GiB(1));

    Lexer lexer = {0};
    char* raw_input = NULL;
    while ((raw_input = readline("> ")) != NULL)
    {
        String_View input = string_view(raw_input);
        println("Got input: '{}'", input);
        lexer_create(&lexer, input);

        println("Got tokens:");
        Token token = {0};
        while (lexer_get_next_token(&lexer, &token))
        {
            String_View prefix = string_view("Found token: ");
            log_print_code_line_with_highlighting(string_view(prefix),
                                                  input,
                                                  token.column,
                                                  token.lexeme.length);

            println(" type = '{}', lexeme = '{}'\n",
                    token_type_to_string(token.type),
                    token.lexeme);
        }

        lexer_destroy(&lexer);

        // TODO(vlad): Use arenas instead.
        free((void*)input.data);
    }

    return 0;
}

#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>

#include "eon_lexer.c"
#include "eon_log.c"
#include "eon_parser.c"
