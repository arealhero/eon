#include "diff.h"

internal String_View
get_next_line(const String_View string, const Index start_index)
{
    String_View result = {0};

    if (string.length <= start_index || string.data[start_index] == '\n')
    {
        return result;
    }

    Index end_index = start_index + 1;
    for (;
         end_index < string.length;
         ++end_index)
    {
        if (string.data[end_index] == '\n')
        {
            break;
        }
    }

    result.data = string.data + start_index;
    result.length = end_index - start_index;
    return result;
}

internal String_View
find_line_by_number(const String_View string, const Index line_number)
{
    Index start_index = 0;
    Index current_line_number = 0;

    while (current_line_number != line_number && start_index < string.length)
    {
        if (string.data[start_index] == '\n')
        {
            current_line_number += 1;
        }

        start_index += 1;
    }

    if (current_line_number == line_number)
    {
        return get_next_line(string, start_index);
    }

    return (String_View){0};
}

internal Diff
calculate_line_diff(Arena* arena,
                    const String_View lhs,
                    const String_View rhs)
{
    Diff diff = {0};
    diff.line_indices_arena = arena;
    diff.lhs = lhs;
    diff.rhs = rhs;

    Index line_index = 0;
    Index current_lhs_index = 0;
    Index current_rhs_index = 0;

    while (current_lhs_index < lhs.length || current_rhs_index < rhs.length)
    {
        const String_View lhs_line = get_next_line(lhs, current_lhs_index);
        const String_View rhs_line = get_next_line(rhs, current_rhs_index);

        if (!strings_are_equal(lhs_line, rhs_line))
        {
            append_array(diff.line_indices_arena, diff.lines, Index, line_index);
        }

        current_lhs_index += lhs_line.length + 1;
        current_rhs_index += rhs_line.length + 1;

        line_index += 1;
    }

    return diff;
}

internal String_View
line_diff_to_string(Arena* arena, const Diff* diff)
{
    String_Builder builder = {0};
    create_string_builder(&builder, arena);

    for (Index line_index = 0;
         line_index < diff->lines_count;
         ++line_index)
    {
        const Index line = diff->lines[line_index];

        const String_View lhs_line = find_line_by_number(diff->lhs, line);
        const String_View rhs_line = find_line_by_number(diff->rhs, line);

        append_string(&builder, string_view(format_string(arena, "{}\n", line + 1)));
        append_string(&builder, string_view("< "));
        append_string(&builder, lhs_line);
        append_string(&builder, string_view("\n"));

        append_string(&builder, string_view("> "));
        append_string(&builder, rhs_line);
        append_string(&builder, string_view("\n"));
    }

    return string_builder_to_string(&builder);
}
