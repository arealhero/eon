#include "grammar_log.h"

#include <eon/string.h>

internal void
show_grammar_error(Arena* scratch,
                   String_View grammar,
                   const ssize line,
                   const ssize column,
                   const ssize highlight_length)
{
    ssize line_start_index = 0;
    ssize line_length = 0;

    ssize current_line = 0;
    ssize current_index = 0;
    while (current_line != line)
    {
        if (current_index >= grammar.length)
        {
            printf("ERROR: Invalid line number provided:\n"
                   "       Total lines: %ld\n"
                   "       Requested line: %ld\n",
                   current_line,
                   line);
            FAIL();
        }

        ASSERT(current_index < grammar.length);
        const char current_char = grammar.data[current_index];
        if (current_char == '\n')
        {
            current_line += 1;
        }

        current_index += 1;
        line_start_index += 1;
    }

    // NOTE(vlad): Finding the end of current line.
    for (;
         current_index != grammar.length
             && grammar.data[current_index] != '\n';
         ++current_index)
    {
        line_length += 1;
    }

    String_View requested_line = {0};
    requested_line.data = grammar.data + line_start_index;
    requested_line.length = line_length;

    printf("\n");
    String error_line_prefix = format_string(scratch, "   {}", line + 1);
    String error_line = format_string(scratch, "{} | {}\n", error_line_prefix, requested_line);
    printf("%.*s", FORMAT_STRING(error_line));

    // TODO(vlad): Construct string inplace and print it in one function call.
    //             We already know this string's length, so we can preallocate the memory for it.
    for (ssize i = 0;
         i < error_line_prefix.length;
         ++i)
    {
        printf(" ");
    }
    printf(" | ");
    for (ssize i = 0;
         i < column;
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
    printf("\n\n");
}
