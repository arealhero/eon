#include "eon_log.h"

internal void
log_print_code_line_with_highlighting(const String_View prefix,
                                      const String_View code_line,
                                      const ssize highlight_start,
                                      const ssize highlight_length)
{
    printf("%.*s%.*s\n", FORMAT_STRING(prefix), FORMAT_STRING(code_line));

    const ssize offset = prefix.length + highlight_start;
    for (ssize i = 0;
         i < offset;
         ++i)
    {
        printf(" ");
    }

    printf("^");
    for (ssize i = 0;
         i < highlight_length - 1;
         ++i)
    {
        printf("~");
    }
    printf("\n");
}
