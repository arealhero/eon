#include "eon_diagnostics.h"

#include "eon_compilation_context.h"

internal inline String_View
message_level_to_string(const Message_Level level)
{
    switch (level)
    {
        case MESSAGE_LEVEL_ERROR: return string_view("error");
        case MESSAGE_LEVEL_NOTE: return string_view("note");
        case MAX_MESSAGE_LEVEL: FAIL("MAX_MESSAGE_LEVEL should not be converted to string.");
    }

    UNREACHABLE();
}

internal inline String_View
source_location_to_string(struct Compilation_Context* context,
                          const Source_Location* location)
{
    return (String_View) {
        .data = context->source_file.code.data + location->offset_in_bytes,
        .length = location->length_in_bytes,
    };
}

internal inline void
extend_location(Source_Location* location, const Source_Location* to_location)
{
    location->length_in_bytes = to_location->offset_in_bytes
        + to_location->length_in_bytes
        - location->offset_in_bytes;
}

internal String
format_diagnostic_message(Arena* arena,
                          Compilation_Context* context,
                          const Diagnostic_Message* message)
{
    Index current_line_start_index = message->location.offset_in_bytes;

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

    Index current_line_end_index = message->location.offset_in_bytes;

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

    const Index location_length = MAX(message->location.length_in_bytes, 1);

    const String line_number_as_a_string = format_string(arena, "{}", message->location.line + 1);
    const Size highlight_line_length = line_number_as_a_string.length + 3
        + message->location.column + location_length;

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
         i < message->location.column + 1;
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
                           "{}:{}:{}: {}: {}\n"
                           "  {} | {}\n"
                           "  {}",
                           context->source_file.filename,
                           line_number_as_a_string,
                           message->location.column + 1,
                           message_level_to_string(message->level),
                           message->text,
                           line_number_as_a_string,
                           current_line,
                           highlight_line);

    return result;
}
