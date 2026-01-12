#include "eon_log.h"

#include <eon/io.h>

internal void
log_print_code_line_with_highlighting(const String_View prefix,
                                      const String_View code_line,
                                      const Size highlight_start,
                                      const Size highlight_length)
{
    println("{}{}", prefix, code_line);

    const Index offset = prefix.length + highlight_start;
    for (Index i = 0;
         i < offset;
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
    print("\n");
}
