#include "grammar_log.h"

#include <eon/string.h>

internal void
show_grammar_error(Arena* scratch,
                   String_View grammar,
                   const s64 line,
                   const s64 column,
                   const Size highlight_length)
{
    Index line_start_index = 0;
    Size line_length = 0;

    s64 current_line = 0;
    s64 current_index = 0;
    while (current_line != line)
    {
        if (current_index >= grammar.length)
        {
            println("ERROR: Invalid line number provided:\n"
                    "       Total lines: {}\n"
                    "       Requested line: {}",
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

    print("\n");
    String error_line_prefix = format_string(scratch, "   {}", line + 1);
    println("{} | {}", error_line_prefix, requested_line);

    // TODO(vlad): Construct string inplace and print it in one function call.
    //             We already know this string's length, so we can preallocate the memory for it.
    for (Index i = 0;
         i < error_line_prefix.length;
         ++i)
    {
        print(" ");
    }
    print(" | ");
    for (Index i = 0;
         i < column;
         ++i)
    {
        print(" ");
    }
    print("^");
    for (Index i = 0;
         i < highlight_length - 1;
         ++i)
    {
        print("~");
    }
    print("\n\n");
}
