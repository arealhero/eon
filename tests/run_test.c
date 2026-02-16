#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include <eon/platform/filesystem.h>
#include <eon/platform/time.h>

// FIXME(vlad): Abstract this to 'eon/plaftorm' and support Windows.
#include <unistd.h>
#include <sys/wait.h>

internal String
read_from_fd_until_done(Arena* arena, const int fd)
{
    String result = {0};

    enum { buffer_size = 512 };
    char buffer[buffer_size] = {0};

    Size bytes_read = 0;
    while ((bytes_read = read(fd, buffer, buffer_size)) > 0)
    {
        String_View next_chunk = {0};
        next_chunk.data = buffer;
        next_chunk.length = bytes_read;

        result.data = reallocate(arena,
                                 result.data,
                                 char,
                                 result.length,
                                 result.length + bytes_read);
        copy_memory(as_bytes(result.data + result.length),
                    as_bytes(buffer),
                    bytes_read);

        result.length += bytes_read;
    }

    return result;
}

internal void
remove_trailing_newline_if_needed(String* string)
{
    if (string->length == 0)
    {
        return;
    }

    if (string->data[string->length - 1] == '\n')
    {
        string->length -= 1;
    }
}

int
main(const int argc, const char* argv[])
{
    init_io_state(GiB(1));

    if (argc != 3)
    {
        println("Usage: run_test <eon executable> <directory>");
        return EXIT_FAILURE;
    }

    const String_View eon_executable = string_view(argv[1]);
    const String_View test_directory = string_view(argv[2]);

    Arena* scratch_arena = arena_create("scratch", GiB(1), MiB(1));

    {
        const File_Info eon_executable_info = platform_get_file_info(scratch_arena, eon_executable);

        if (!eon_executable_info.exists)
        {
            println("Error: file {} does not exist", eon_executable);
            return EXIT_FAILURE;
        }

        if (!eon_executable_info.executable)
        {
            println("Error: file {} cannot be executed", eon_executable);
            return EXIT_FAILURE;
        }
    }

    const String_View main_filename = string_view(format_string(scratch_arena, "{}/main.eon", test_directory));

    {
        const File_Info main_filename_info = platform_get_file_info(scratch_arena, main_filename);

        if (!main_filename_info.exists)
        {
            println("Error: file {} does not exist", main_filename);
            return EXIT_FAILURE;
        }

        if (!main_filename_info.readable)
        {
            println("Error: file {} cannot be read", main_filename);
            return EXIT_FAILURE;
        }
    }

    int expected_return_code = 0;
    {
        const String_View expected_return_code_filename = string_view(format_string(scratch_arena,
                                                                                    "{}/expected_return_code",
                                                                                    test_directory));
        const File_Info info = platform_get_file_info(scratch_arena, expected_return_code_filename);

        if (info.exists)
        {
            if (!info.readable)
            {
                println("Error: file {} cannot be read", expected_return_code_filename);
                return EXIT_FAILURE;
            }

            Read_File_Result result = platform_read_entire_text_file(scratch_arena,
                                                                     expected_return_code_filename);

            if (result.status != READ_FILE_SUCCESS)
            {
                println("Error: failed to read file {}", expected_return_code_filename);
                return EXIT_FAILURE;
            }

            remove_trailing_newline_if_needed(&result.content);

            if (!parse_integer(result.content, &expected_return_code))
            {
                println("Error: failed to parse expected return code from file {}",
                        expected_return_code_filename);
                return EXIT_FAILURE;
            }
        }
    }

    String expected_stdout = {0};
    {
        const String_View expected_stdout_filename = string_view(format_string(scratch_arena,
                                                                               "{}/expected_stdout",
                                                                               test_directory));
        const File_Info info = platform_get_file_info(scratch_arena, expected_stdout_filename);

        if (info.exists)
        {
            if (!info.readable)
            {
                println("Error: file {} cannot be read", expected_stdout_filename);
                return EXIT_FAILURE;
            }

            Read_File_Result result = platform_read_entire_text_file(scratch_arena,
                                                                     expected_stdout_filename);

            if (result.status != READ_FILE_SUCCESS)
            {
                println("Error: failed to read file {}", expected_stdout_filename);
                return EXIT_FAILURE;
            }

            expected_stdout = result.content;
        }
    }

    String expected_stderr = {0};
    {
        const String_View expected_stderr_filename = string_view(format_string(scratch_arena,
                                                                               "{}/expected_stderr",
                                                                               test_directory));
        const File_Info info = platform_get_file_info(scratch_arena, expected_stderr_filename);

        if (info.exists)
        {
            if (!info.readable)
            {
                println("Error: file {} cannot be read", expected_stderr_filename);
                return EXIT_FAILURE;
            }

            Read_File_Result result = platform_read_entire_text_file(scratch_arena,
                                                                     expected_stderr_filename);

            if (result.status != READ_FILE_SUCCESS)
            {
                println("Error: failed to read file {}", expected_stderr_filename);
                return EXIT_FAILURE;
            }

            expected_stderr = result.content;
        }
    }

    int stdout_pipe[2];
    if (pipe(stdout_pipe) == -1)
    {
        println("Failed to create stdout pipe");
        return EXIT_FAILURE;
    }

    int stderr_pipe[2];
    if (pipe(stderr_pipe) == -1)
    {
        println("Failed to create stderr pipe");
        return EXIT_FAILURE;
    }

    const Timestamp test_start_timestamp = platform_get_current_monotonic_timestamp();

    Bool test_failed = false;

    const pid_t eon_process_id = fork();
    if (eon_process_id == 0)
    {
        // NOTE(vlad): Child process.

        if (dup2(stdout_pipe[1], STDOUT_FD) == -1)
        {
            println("Failed to pipe stdout to parent process");
            return EXIT_FAILURE;
        }

        if (dup2(stderr_pipe[1], STDERR_FD) == -1)
        {
            println("Failed to pipe stderr to parent process");
            return EXIT_FAILURE;
        }

        // NOTE(vlad): We won't read from stdout and stderr.
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // NOTE(vlad): Duplicated by STDOUT_FD and STDERR_FD, thus no longer needed.
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        char* c_eon_executable = to_c_string(scratch_arena, eon_executable);
        char* c_main_filename = to_c_string(scratch_arena, main_filename);

        char* child_argv[] = { c_eon_executable, c_main_filename, NULL };
        char* child_envp[] = { NULL };

        if (execve(c_eon_executable, child_argv, child_envp) == -1)
        {
            println("execve failed");
            return EXIT_FAILURE;
        }

        UNREACHABLE();
    }

    // NOTE(vlad): Parent process.

    // NOTE(vlad): We won't write to stdout and stderr of the child.
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    Arena* child_output_arena = arena_create("child_output", GiB(1), MiB(1));

    const String child_stdout = read_from_fd_until_done(child_output_arena,
                                                        stdout_pipe[0]);
    const String child_stderr = read_from_fd_until_done(child_output_arena,
                                                        stderr_pipe[0]);

    // NOTE(vlad): Parent process.
    int status;
    waitpid(eon_process_id, &status, 0);

    if (!strings_are_equal(string_view(child_stdout),
                           string_view(expected_stdout)))
    {
        println("Error: invalid stdout\n"
                "Expected:\n"
                "---\n"
                "{}\n"
                "---\n"
                "Got:\n"
                "---\n"
                "{}\n"
                "---",
                expected_stdout,
                child_stdout);
        test_failed = true;
    }

    if (!strings_are_equal(string_view(child_stderr),
                           string_view(expected_stderr)))
    {
        println("Error: invalid stderr\n"
                "Expected:\n"
                "{}\n"
                "Got:\n"
                "{}",
                expected_stderr,
                child_stderr);
        test_failed = true;
    }

    const int return_code = WEXITSTATUS(status);

    if (return_code != expected_return_code)
    {
        println("Error: invalid return code: expected {}, got {}",
                expected_return_code,
                return_code);
        test_failed = true;
    }

    const Timestamp test_end_timestamp = platform_get_current_monotonic_timestamp();
    println("Test took in {} mcs", test_end_timestamp - test_start_timestamp);

    arena_destroy(child_output_arena);
    arena_destroy(scratch_arena);

    if (test_failed)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>
