#if !EON_PLATFORM_FILESYSTEM_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/filesystem.h>" instead.
#endif

#include "win32_hacks.h"
#include <windows.h>

internal Read_File_Result
platform_read_entire_text_file(Arena* arena, const String_View filename)
{
    Read_File_Result result = {0};

    const char* zero_terminated_filename = to_c_string(arena, filename);

    const HANDLE file_handle = CreateFileA(zero_terminated_filename,
                                           GENERIC_READ,
                                           FILE_SHARE_READ,
                                           NULL,
                                           OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);

    if (file_handle == INVALID_HANDLE_VALUE)
    {
        result.status = READ_FILE_FAILURE;
        return result;
    }

    LARGE_INTEGER file_size = {0};
    if (GetFileSizeEx(file_handle, &file_size))
    {
        const Size content_length = (Size)file_size.QuadPart;

        if (content_length == 0)
        {
            result.status = READ_FILE_SUCCESS;
        }
        else
        {
            char* content = allocate_uninitialized_array(arena, content_length, char);

            DWORD bytes_read = 0;
            if (ReadFile(file_handle, content, (DWORD)content_length, &bytes_read, NULL))
            {
                result.status = READ_FILE_SUCCESS;
                result.content.data = content;
                result.content.length = (Size)bytes_read;
            }
            else
            {
                result.status = READ_FILE_FAILURE;
            }
        }
    }
    else
    {
        result.status = READ_FILE_FAILURE;
    }

    CloseHandle(file_handle);

    return result;
}

internal void
platform_write_string_to_file(Arena* scratch_arena,
                              const String_View filename,
                              const String_View content)
{
    const char* zero_terminated_filename = to_c_string(scratch_arena, filename);

    HANDLE file_handle = CreateFileA(zero_terminated_filename,
                                     GENERIC_WRITE,
                                     0,
                                     NULL,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);

    if (file_handle == INVALID_HANDLE_VALUE)
    {
        FAIL("Failed to open file");
        return;
    }

    DWORD bytes_written = 0;
    if (!WriteFile(file_handle, content.data, (DWORD)content.length, &bytes_written, NULL))
    {
        FAIL("Failed to write to a file");
    }

    ASSERT(bytes_written == (DWORD)content.length);
    CloseHandle(file_handle);
}

#include "win32_restore_hacks.h" // IWYU pragma: export
