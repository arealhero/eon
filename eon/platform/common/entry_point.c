#if !EON_PLATFORM_ENTRY_POINT_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/entry_point.h>" instead.
#endif

#include <eon/platform/memory.h>

#if OS_LINUX
#    include <eon/platform/linux/linux_interface.h>
#endif

#include <eon/build_info.h>
#include <eon/string.h>

int
main(const int argc, const char** argv)
{
#if OS_LINUX
    // NOTE(vlad): Initializing LINUX_PAGE_SIZE from the kernel's aux vector.
    {
        const char** envp = argv + argc + 1;
        while (*envp != NULL)
        {
            envp += 1;
        }

        unsigned long* auxv = (unsigned long*)(envp + 1);

        for (Index i = 0;
             auxv[i] != AT_NULL;
             i += 2)
        {
            if (auxv[i] == AT_PAGESZ)
            {
                LINUX_PAGE_SIZE = (Size)(auxv[i + 1]);
                break;
            }
        }

        SILENT_ASSERT(LINUX_PAGE_SIZE != 0, "Failed to get PAGE_SIZE");
    }
#endif

    init_io_state(MiB(30));

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
