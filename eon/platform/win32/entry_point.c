#if !EON_PLATFORM_ENTRY_POINT_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/entry_point.h>" instead.
#endif

#include "windows_interface.h"

#include <eon/keywords.h>
#include <eon/assert.h>
#include <eon/string_declarations.h>

// NOTE(vlad): Compiler can choose to emit a call to 'memset' when we initialize a struct
//             to zero like this: 'Struct var = {0};' (yes, even when '-fno-builtin' flag was provided).
//             Since we do not use libc, we need to provide the 'memset' ourselves.
//
//             Also we cannot mark it as internal *sigh*.
void*
memset(void* ptr,
       int value,
       const USize number_of_bytes)
{
    Byte* memory = as_bytes(ptr);

    for (USize i = 0;
         i < number_of_bytes;
         ++i)
    {
        memory[i] = (Byte)(value);
    }

    return ptr;
}

void*
memcpy(void* destination,
       const void* source,
       USize number_of_bytes)
{
    const Byte* from = (const Byte*)(source);
    Byte* to = as_bytes(destination);

    for (USize i = 0;
         i < number_of_bytes;
         ++i)
    {
        to[i] = from[i];
    }

    return destination;
}

// NOTE(vlad): Since we use floats, the linker expects this variable to be defined.
//             That in turn will force Windows to initialize the FPU.
int _fltused = 0;

void
platform_entry_point(void)
{
    init_io_state(GiB(1));

    String_View raw_arguments = {0};

    {
        raw_arguments.data = GetCommandLineA();

        for (const char* c = raw_arguments.data;
             *c != '\0';
             ++c)
        {
            raw_arguments.length += 1;
        }
    }

    Size number_of_spaces = 0;
    for (Index index = 0;
         index < raw_arguments.length;
         ++index)
    {
        if (raw_arguments.data[index] == ' ')
        {
            number_of_spaces += 1;
        }
    }

    const Size max_arguments_count = 1 + number_of_spaces;
    const Size number_of_bytes = max_arguments_count * size_of(String_View);

    String_View* arguments = VirtualAlloc(NULL,
                                          (SIZE_T)(number_of_bytes),
                                          MEM_RESERVE | MEM_COMMIT,
                                          PAGE_READWRITE);
    Size arguments_count = 0;

    {
        // NOTE(vlad): Parsing arguments.

        Bool should_skip_next_char = false;
        Bool inside_quotes = false;

        Index word_start_index = 0;
        for (Index index = 0;
             index < raw_arguments.length;
             ++index)
        {
            const char c = raw_arguments.data[index];

            if (should_skip_next_char)
            {
                should_skip_next_char = false;
            }
            else
            {
                switch (c)
                {
                    case '\\':
                    {
                        should_skip_next_char = true;
                    } break;

                    case '"':
                    {
                        inside_quotes = !inside_quotes;
                    } break;

                    case ' ':
                    {
                        if (!inside_quotes)
                        {
                            const Index word_end_index = index;

                            if (word_end_index == word_start_index)
                            {
                                // NOTE(vlad): Multiple spaces in a row, skipping.
                            }
                            else
                            {
                                String_View argument = {0};
                                argument.data = raw_arguments.data + word_start_index;
                                argument.length = word_end_index - word_start_index;
                                ASSERT(arguments_count < max_arguments_count);
                                arguments[arguments_count++] = argument;
                            }

                            word_start_index = word_end_index + 1;
                        }
                    } break;
                }
            }
        }

        if (word_start_index != raw_arguments.length)
        {
            String_View argument = {0};
            argument.data = raw_arguments.data + word_start_index;
            argument.length = raw_arguments.length - word_start_index;
            ASSERT(arguments_count < max_arguments_count);
            arguments[arguments_count++] = argument;
        }
    }

    const int return_code = eon_main(arguments, arguments_count);

    deinit_io_state();

    ExitProcess((DWORD)(return_code));
}
