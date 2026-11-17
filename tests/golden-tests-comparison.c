#include "golden-tests-comparison.h"

#include <eon/diff.h>

#include <eon/platform/filesystem.h>

internal Bool
compare_outputs_and_optionally_canonize(Arena* scratch_arena,
                                        const String_View test_name,
                                        const String_View filename,
                                        const String_View output,
                                        const Bool canonize_output)
{
    const Read_File_Result result = platform_read_entire_text_file(scratch_arena, filename);

    String_View canon = {0};
    if (result.status == READ_FILE_SUCCESS)
    {
        canon = string_view(result.content);
    }

    // NOTE(vlad): Alas we cannot just use 'strings_are_equal' on Windows because it uses CRLF by default and that
    //             messes up the comparison. We could in theory emit a '\r\n' instead of '\n' during the output
    //             construction, but that will be fragile as well: git on Windows supports both CRLF and LF checkouts so
    //             we do not know what line ending style is used right now.
    //
    //             Also note that we will use 'strings_are_equal' on other platforms because we will not tolerate
    //             CRLF-style canon files in our repository.

#if OS_WINDOWS
    Index current_canon_index = 0;
    Index current_output_index = 0;

    const Bool canon_is_empty = canon.length == 0;
    const Bool output_is_empty = output.length == 0;

    Bool diff_detected = canon_is_empty ^ output_is_empty;

    while (current_canon_index < canon.length && current_output_index < output.length)
    {
        String_View canon_line = get_next_line(canon, current_canon_index);
        String_View output_line = get_next_line(output, current_output_index);

        current_canon_index += canon_line.length + 1;
        current_output_index += output_line.length + 1;

        if (canon_line.length != 0 && canon_line.data[canon_line.length - 1] == '\r')
        {
            // NOTE(vlad): CRLF detected.
            canon_line.length -= 1;
        }

        if (!strings_are_equal(canon_line, output_line))
        {
            diff_detected = true;
            break;
        }
    }

    if (!diff_detected)
    {
        if (canonize_output)
        {
            println("{}: Outputs are the same", test_name);
        }

        return true;
    }
#else
    if (strings_are_equal(output, canon))
    {
        if (canonize_output)
        {
            println("{}: Outputs are the same", test_name);
        }

        return true;
    }
#endif

    if (canonize_output)
    {
        platform_write_string_to_file(scratch_arena, filename, output);
        println("{}: Output canonized", test_name);
        return true;
    }

    if (canon.length == 0)
    {
        println("{}: Canon is empty. Current output:\n\n{}", test_name, output);
        return false;
    }
    else
    {
        const Diff diff = calculate_line_diff(scratch_arena, canon, output);
        const String_View diff_string = line_diff_to_string(scratch_arena, &diff);

        println("{}: Test failed with this diff:\n\n{}", test_name, diff_string);
        return false;
    }
}
