#include "eon_log.h"

#include <eon/io.h>

internal void
log_print_code_line_with_highlighting(const String_View prefix,
                                      const String_View code_line,
                                      const ssize highlight_start,
                                      const ssize highlight_length)
{
    println("{}{}", prefix, code_line);

    const ssize offset = prefix.length + highlight_start;
    for (ssize i = 0;
         i < offset;
         ++i)
    {
        print(" ");
    }

    print("^");
    for (ssize i = 0;
         i < highlight_length - 1;
         ++i)
    {
        print("~");
    }
    print("\n");
}
