#pragma once

#include <eon/string.h>
#include <eon/containers.h>

struct Diff
{
    Arena* line_indices_arena;

    String_View lhs;
    String_View rhs;

    array(Index, lines);
};
typedef struct Diff Diff;

internal String_View get_next_line(const String_View string, const Index start_index);

internal Diff calculate_line_diff(Arena* arena,
                                  const String_View lhs,
                                  const String_View rhs);

internal String_View line_diff_to_string(Arena* arena, const Diff* diff);
