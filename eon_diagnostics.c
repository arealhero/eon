#include "eon_diagnostics.h"

#include "eon_compilation_context.h"

maybe_unused internal String
format_error_message(Arena* arena, Compilation_Context* context, const Error* error)
{
    Index current_line_start_index = error->location.offset_in_bytes;

    while (current_line_start_index > 0)
    {
        const char this_char = context->source_file.code.data[current_line_start_index];
        if (this_char == '\n')
        {
            current_line_start_index += 1;
            break;
        }

        current_line_start_index -= 1;
    }

    Index current_line_end_index = error->location.offset_in_bytes;

    while (current_line_end_index < context->source_file.code.length)
    {
        const char this_char = context->source_file.code.data[current_line_end_index];
        if (this_char == '\n')
        {
            break;
        }

        current_line_end_index += 1;
    }

    const String_View current_line = (String_View) {
        .data = context->source_file.code.data + current_line_start_index,
        .length = current_line_end_index - current_line_start_index,
    };

    String result = {0};

    const Index location_length = MAX(error->location.length_in_bytes, 1);

    const String line_number_as_a_string = format_string(arena, "{}", error->location.line + 1);
    const Size highlight_line_length = line_number_as_a_string.length + 3
        + error->location.column + location_length;

    // TODO(vlad): Support multiline highlights?

    String highlight_line = {0};
    highlight_line.data = allocate_uninitialized_array(arena, highlight_line_length, char);
    highlight_line.length = highlight_line_length;

    Index next_char_index = 0;
    for (Index i = 0;
         i < line_number_as_a_string.length + 1;
         ++i)
    {
        highlight_line.data[next_char_index++] = ' ';
    }

    highlight_line.data[next_char_index++] = '|';

    for (Index i = 0;
         i < error->location.column + 1;
         ++i)
    {
        highlight_line.data[next_char_index++] = ' ';
    }

    highlight_line.data[next_char_index++] = '^';

    for (Index i = 0;
         i < location_length - 1;
         ++i)
    {
        highlight_line.data[next_char_index++] = '~';
    }

    result = format_string(arena,
                           "{}:{}:{}: error: {}\n"
                           "  {} | {}\n"
                           "  {}",
                           context->source_file.filename,
                           line_number_as_a_string,
                           error->location.column + 1,
                           error->message,
                           line_number_as_a_string,
                           current_line,
                           highlight_line);

    return result;
}
