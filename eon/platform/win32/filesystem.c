#if !EON_PLATFORM_FILESYSTEM_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/filesystem.h>" instead.
#endif

#include "windows_interface.h"

#include <eon/assert.h>

struct Win32_File_State
{
    HANDLE handle;
};
typedef struct Win32_File_State Win32_File_State;

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

internal Read_Binary_File_Result
platform_read_entire_binary_file(Arena* arena, const String_View filename)
{
    Read_Binary_File_Result result = {0};

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
                result.content.data = as_bytes(content);
                result.content.data_size_in_bytes = (Size)bytes_read;
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

internal Platform_File_State
platform_open_binary_file_for_writing(Arena* scratch_arena,
                                      const String_View filename)
{
    const char* zero_terminated_filename = to_c_string(scratch_arena, filename);

    const HANDLE handle = CreateFileA(zero_terminated_filename,
                                      GENERIC_WRITE,
                                      0,
                                      NULL,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        FAIL("Failed to open binary file for writing");
    }

    Win32_File_State state = {0};
    state.handle = handle;

    return *(Platform_File_State*)(&state);
}

internal void
platform_write_binary_blob_to_file(const Platform_File_State platform_file_state,
                                   const Bytes_View blob)
{
    const Win32_File_State state = *(Win32_File_State*)(&platform_file_state);

    DWORD number_of_bytes_written = 0;
    WriteFile(state.handle,
              blob.data,
              (DWORD)(blob.data_size_in_bytes),
              &number_of_bytes_written,
              NULL);

    ASSERT(number_of_bytes_written == blob.data_size_in_bytes);
}

internal void
platform_close_file(const Platform_File_State platform_file_state)
{
    const Win32_File_State state = *(Win32_File_State*)(&platform_file_state);

    // TODO(vlad): Assert that CloseHandle executed successfully.
    CloseHandle(state.handle);
}
