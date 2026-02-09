#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

#include <eon/platform/filesystem.h>
#include <eon/platform/time.h>

// FIXME(vlad): Abstract this to 'eon/plaftorm' and support Windows.
#include <unistd.h>
#include <sys/wait.h>

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

            Index end_index = result.content.length - 1;
            if (end_index >= 0 && result.content.data[end_index] == '\n')
            {
                result.content.length -= 1;
            }

            if (!parse_integer(result.content, &expected_return_code))
            {
                println("Error: failed to parse expected return code from file {}",
                        expected_return_code_filename);
                return EXIT_FAILURE;
            }
        }
    }

    const Timestamp test_start_timestamp = platform_get_current_monotonic_timestamp();

    Bool test_failed = false;

    const pid_t eon_process_id = fork();
    if (eon_process_id == 0)
    {
        // NOTE(vlad): Child process.
        char* c_eon_executable = to_c_string(scratch_arena, eon_executable);
        char* c_main_filename = to_c_string(scratch_arena, main_filename);

        char* child_argv[] = { c_eon_executable, c_main_filename, NULL };
        char* child_envp[] = { NULL };

        if (execve(c_eon_executable, child_argv, child_envp) == -1)
        {
            println("execve failed");
            return EXIT_FAILURE;
        }
    }
    else
    {
        // NOTE(vlad): Parent process.
        int status;
        waitpid(eon_process_id, &status, 0);

        const int return_code = WEXITSTATUS(status);

        if (return_code != expected_return_code)
        {
            println("Error: invalid return code: expected {}, got {}",
                    expected_return_code,
                    return_code);
            test_failed = true;
        }
    }

    const Timestamp test_end_timestamp = platform_get_current_monotonic_timestamp();
    println("Test completed in {} mcs", test_end_timestamp - test_start_timestamp);

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
