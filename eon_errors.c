#include "eon_errors.h"

#include <eon/io.h>

internal void
errors_create(Errors* errors, Arena* errors_arena)
{
    *errors = (Errors) {0};
    errors->errors_arena = errors_arena;
}

internal void
add_error(Errors* errors, Error* error)
{
    if (errors->errors_count == errors->errors_capacity)
    {
        const Size new_capacity = MAX(1, 2 * errors->errors_capacity);
        errors->errors = reallocate(errors->errors_arena,
                                    errors->errors,
                                    Error,
                                    errors->errors_capacity,
                                    new_capacity);
        errors->errors_capacity = new_capacity;
    }

    errors->errors[errors->errors_count] = *error;
    errors->errors_count += 1;
}

maybe_unused internal void
print_error(Arena* scratch, const Error* error)
{
    println("{}:{}:{}: Error: {}",
            error->filename,
            error->line + 1,
            error->column + 1,
            error->message);

    Index line_start_index = 0;
    Size line_length = 0;

    Index current_line = 0;
    Index current_index = 0;
    while (current_line != error->line)
    {
        if (current_index >= error->code.length)
        {
            println("ERROR: Invalid line number provided:\n"
                    "       Total lines: {}\n"
                    "       Requested line: {}",
                    current_line,
                    error->line);
            FAIL("Invalid line number provided");
        }

        ASSERT(current_index < error->code.length);
        const char current_char = error->code.data[current_index];
        if (current_char == '\n')
        {
            current_line += 1;
        }

        current_index += 1;
        line_start_index += 1;
    }

    // NOTE(vlad): Finding the end of current line.
    for (;
         current_index != error->code.length
             && error->code.data[current_index] != '\n';
         ++current_index)
    {
        line_length += 1;
    }

    String_View requested_line = {0};
    requested_line.data = error->code.data + line_start_index;
    requested_line.length = line_length;

    print("\n");
    String error_line_prefix = format_string(scratch, "   {}", error->line + 1);
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
         i < error->column;
         ++i)
    {
        print(" ");
    }
    print("^");
    for (Index i = 0;
         i < error->highlight_length - 1;
         ++i)
    {
        print("~");
    }
    print("\n\n");
}

maybe_unused internal void
clear_errors(Errors* errors)
{
    // NOTE(vlad): Do not clear 'errors_arena' here: it can be used for something else.
    errors->errors = NULL;
    errors->errors_count = 0;
    errors->errors_capacity = 0;
}

internal void
errors_destroy(Errors* errors)
{
    UNUSED(errors);
}
