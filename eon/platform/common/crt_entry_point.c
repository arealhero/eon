#if !EON_PLATFORM_ENTRY_POINT_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/entry_point.h>" instead.
#endif

#include <eon/platform/memory.h>

#include <eon/string.h>

int
main(const int argc, const char** argv)
{
    init_io_state(GiB(1));

    const Size number_of_bytes = argc * size_of(String_View);
    String_View* arguments = (String_View*)(platform_reserve_memory(number_of_bytes));
    platform_commit_memory(as_bytes(arguments), number_of_bytes);

    Size arguments_count = argc;

    for (Index argument_index = 0;
         argument_index < argc;
         ++argument_index)
    {
        arguments[argument_index] = string_view(argv[argument_index]);
    }

    const int return_code = eon_main(arguments, arguments_count);

    deinit_io_state();

    return return_code;
}
