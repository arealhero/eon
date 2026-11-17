#if !EON_PLATFORM_SUBPROCESS_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/subprocess.h>" instead.
#endif

#include <eon/assert.h>

#include "windows_interface.h"

// NOTE(vlad): We need two threads to avoid deadlocks when child rapidly writes to stdout and to stderr.
//             For example, we can hang on the ReadFile(stdout) while the child is hanging on the WriteFile(stderr)
//             due to the stdout buffer being empty and the stderr buffer being full.

struct Win32_Pipe_Reader_State
{
    Arena* arena;
    CRITICAL_SECTION* arena_critical_section;
    HANDLE read_handle;
    Buffer buffer;
};
typedef struct Win32_Pipe_Reader_State Win32_Pipe_Reader_State;

internal DWORD WINAPI
win32_pipe_reader_thread(void* argument)
{
    enum { CHUNK_SIZE = KiB(4) };

    Win32_Pipe_Reader_State* state = argument;

    Byte chunk[CHUNK_SIZE];
    DWORD bytes_read = 0;
    while (ReadFile(state->read_handle, chunk, CHUNK_SIZE, &bytes_read, NULL) != 0 && bytes_read > 0)
    {
        EnterCriticalSection(state->arena_critical_section);

        Bytes_View bytes = {0};
        bytes.data = chunk;
        bytes.data_size_in_bytes = bytes_read;

        append_bytes_to_buffer(state->arena, &state->buffer, bytes);

        LeaveCriticalSection(state->arena_critical_section);
    }

    return 0;
}

internal Spawn_Process_Result
platform_spawn_subprocess_and_wait_until_done(Arena* arena,
                                              const String_View executable,
                                              const String_View* arguments,
                                              const Size arguments_count)
{
    SECURITY_ATTRIBUTES security_attributes = {0};
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.bInheritHandle = TRUE;

    HANDLE stdout_read_handle = INVALID_HANDLE_VALUE;
    HANDLE stdout_write_handle = INVALID_HANDLE_VALUE;

    if (!CreatePipe(&stdout_read_handle, &stdout_write_handle, &security_attributes, 0))
    {
        Spawn_Process_Result result = {0};
        result.status = SPAWN_PROCESS_FAILED;
        return result;
    }

    HANDLE stderr_read_handle = INVALID_HANDLE_VALUE;
    HANDLE stderr_write_handle = INVALID_HANDLE_VALUE;

    if (!CreatePipe(&stderr_read_handle, &stderr_write_handle, &security_attributes, 0))
    {
        Spawn_Process_Result result = {0};
        result.status = SPAWN_PROCESS_FAILED;
        return result;
    }

    ASSERT(stdout_read_handle != INVALID_HANDLE_VALUE);
    ASSERT(stdout_write_handle != INVALID_HANDLE_VALUE);

    ASSERT(stderr_read_handle != INVALID_HANDLE_VALUE);
    ASSERT(stderr_write_handle != INVALID_HANDLE_VALUE);

    // NOTE(vlad): Parent process must not inherit the read end of pipes.
    SetHandleInformation(stdout_read_handle, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stderr_read_handle, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA startup_info = {0};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup_info.hStdOutput = stdout_write_handle;
    startup_info.hStdError = stderr_write_handle;

    PROCESS_INFORMATION process_information = {0};
    // NOTE(vlad): 'CreateProcess' may mutate the command line, thus we need to copy it here.
    {
        String_Builder builder = {0};
        create_string_builder(&builder, arena);
        append_string(&builder, executable);

        for (Index argument_index = 0;
             argument_index < arguments_count;
             ++argument_index)
        {
            append_string(&builder, " ");

            // FIXME(vlad): Add quotes if argument contains whitespaces.
            append_string(&builder, arguments[argument_index]);
        }

        // FIXME(vlad): Append '\0' to the builder instead of copying the string here.
        const String_View command = string_builder_to_string(&builder);
        const C_String zero_terminated_command = to_c_string(arena, command);

        const BOOL ok = CreateProcessA(NULL,
                                       zero_terminated_command,
                                       NULL,
                                       NULL,
                                       TRUE, // NOTE(vlad): Inherit handles.
                                       0,
                                       NULL,
                                       NULL,
                                       &startup_info,
                                       &process_information);

        // NOTE(vlad): Parent process must not write to the write end of pipes.
        CloseHandle(stdout_write_handle);
        CloseHandle(stderr_write_handle);

        if (!ok)
        {
            CloseHandle(stdout_read_handle);
            CloseHandle(stderr_read_handle);

            Spawn_Process_Result result = {0};
            result.status = SPAWN_PROCESS_FAILED;
            return result;
        }
    }

    CRITICAL_SECTION arena_critical_section = {0};
    InitializeCriticalSection(&arena_critical_section);

    Win32_Pipe_Reader_State stdout_reader_state = {0};
    stdout_reader_state.arena = arena;
    stdout_reader_state.arena_critical_section = &arena_critical_section;
    stdout_reader_state.read_handle = stdout_read_handle;

    Win32_Pipe_Reader_State stderr_reader_state = {0};
    stderr_reader_state.arena = arena;
    stderr_reader_state.arena_critical_section = &arena_critical_section;
    stderr_reader_state.read_handle = stderr_read_handle;

    HANDLE stdout_reader_thread_handle = CreateThread(NULL,
                                                      0,
                                                      win32_pipe_reader_thread,
                                                      &stdout_reader_state,
                                                      0,
                                                      NULL);
    HANDLE stderr_reader_thread_handle = CreateThread(NULL,
                                                      0,
                                                      win32_pipe_reader_thread,
                                                      &stderr_reader_state,
                                                      0,
                                                      NULL);

    WaitForSingleObject(process_information.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(process_information.hProcess, &exit_code);

    WaitForSingleObject(stdout_reader_thread_handle, INFINITE);
    WaitForSingleObject(stderr_reader_thread_handle, INFINITE);

    DeleteCriticalSection(&arena_critical_section);

    CloseHandle(process_information.hProcess);
    CloseHandle(process_information.hThread);

    CloseHandle(stdout_reader_thread_handle);
    CloseHandle(stderr_reader_thread_handle);

    CloseHandle(stdout_read_handle);
    CloseHandle(stderr_read_handle);

    String_View stdout = {0};
    stdout.data = (const char*)(stdout_reader_state.buffer.bytes);
    stdout.length = stdout_reader_state.buffer.bytes_count;

    String_View stderr = {0};
    stderr.data = (const char*)(stderr_reader_state.buffer.bytes);
    stderr.length = stderr_reader_state.buffer.bytes_count;

    Spawn_Process_Result result = {0};
    result.status = SPAWN_PROCESS_OK;
    result.exit_code = exit_code;
    result.stdout = stdout;
    result.stderr = stderr;

    return result;
}
