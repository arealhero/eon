#include <eon/argument_parser.h>

#include <eon/platform/entry_point.h>
#include <eon/platform/io.h>
#include <eon/platform/subprocess.h>
#include <eon/platform/time.h>

#include <tests/golden-tests-comparison.h>

struct Configuration
{
    String_View path_to_compiler_executable;
    String_View test_directory;
    Bool show_help;
};
typedef struct Configuration Configuration;

internal void print_usage_message(Arena* arena, const Argument_Parser* parser);

internal int
eon_main(const String_View* arguments, const Size arguments_count)
{
    Configuration configuration = {0};

    Arena* test_arena = create_arena("test", MiB(1), MiB(1));

    // NOTE(vlad): Parsing arguments.
    {
        const Argument_Option options[] = {
            {
                .name = string_view("--compiler"),
                .kind = OPTION_WITH_STRING_ARGUMENT,
                .argument_name = string_view("path"),
                .description = string_view("Path to the 'eon' compiler executable"),
                .string = &configuration.path_to_compiler_executable,
            },
            {
                .name = string_view("--help"),
                .kind = OPTION_WITHOUT_ARGUMENT,
                .description = string_view("Show help message"),
                .flag = &configuration.show_help,
                .default_value.flag = false,
            },
        };

        Argument_Parser parser = create_argument_parser(test_arena,
                                                        options,
                                                        NUMBER_OF_STATIC_ARRAY_ELEMENTS(options));

        parse_command_line_arguments(&parser, arguments, arguments_count);

        if (configuration.show_help)
        {
            print_usage_message(test_arena, &parser);
            return EXIT_SUCCESS;
        }

        if (parser.free_arguments_count == 0)
        {
            println("Error: test directory was not provided.");
            print_usage_message(test_arena, &parser);
            return EXIT_FAILURE;
        }
        else if (parser.free_arguments_count > 1)
        {
            print("Error: unknown arguments encountered:");
            for (Index argument_index = 1;
                 argument_index < parser.free_arguments_count;
                 ++argument_index)
            {
                print(" {}", *parser.free_arguments[argument_index]);
            }
            println("");

            print_usage_message(test_arena, &parser);
            return EXIT_FAILURE;
        }

        configuration.test_directory = *parser.free_arguments[0];
    }

    const Timestamp test_start_timestamp = platform_get_current_monotonic_timestamp();
    Bool test_failed = false;

#if OS_WINDOWS
    const char DIRECTORY_SEPARATOR = '\\';
    const String_View OBJECT_FILE_EXTENSION = string_view(".obj");
    const String_View EXECUTABLE_FILE_EXTENSION = string_view(".exe");
#else
    const char DIRECTORY_SEPARATOR = '/';
    const String_View OBJECT_FILE_EXTENSION = string_view(".o");
    const String_View EXECUTABLE_FILE_EXTENSION = string_view("");
#endif

    const String_View main_filename = string_view(format_string(test_arena,
                                                                "{}{}main.eon",
                                                                configuration.test_directory,
                                                                DIRECTORY_SEPARATOR));

    const String_View build_directory = string_view(format_string(test_arena,
                                                                  "build{}{}",
                                                                  DIRECTORY_SEPARATOR,
                                                                  configuration.test_directory));

    const String_View object_filename = string_view(format_string(test_arena,
                                                                  "{}{}main{}",
                                                                  build_directory,
                                                                  DIRECTORY_SEPARATOR,
                                                                  OBJECT_FILE_EXTENSION));

    const String_View executable_filename = string_view(format_string(test_arena,
                                                                      "{}{}main{}",
                                                                      build_directory,
                                                                      DIRECTORY_SEPARATOR,
                                                                      EXECUTABLE_FILE_EXTENSION));

    // NOTE(vlad): Compiling test program.
    {
        const String_View compiler_arguments[] = {
            main_filename,
            string_view("--output"),
            object_filename,
        };

        const Spawn_Process_Result compiler_process_result = platform_spawn_subprocess_and_wait_until_done(test_arena,
                                                                                                           configuration.path_to_compiler_executable,
                                                                                                           compiler_arguments,
                                                                                                           NUMBER_OF_STATIC_ARRAY_ELEMENTS(compiler_arguments));

        if (compiler_process_result.status != SPAWN_PROCESS_OK)
        {
            println("Error: failed to create a subprocess.");
            test_failed = true;
            goto cleanup;
        }

        if (compiler_process_result.exit_code != EXIT_SUCCESS)
        {
            println("Error: compilation failed.");

            if (!strings_are_equal(compiler_process_result.stdout, ""))
            {
                println("{}", compiler_process_result.stdout);
            }

            if (!strings_are_equal(compiler_process_result.stderr, ""))
            {
                println("{}", compiler_process_result.stderr);
            }

            test_failed = true;
            goto cleanup;
        }
    }

    // NOTE(vlad): Linking test program.
    {
        // TODO(vlad): Remove hardcoded "linker" here.
        const String_View linker_executable = string_view("clang");
        const String_View linker_arguments[] = {
            object_filename,
            string_view("-o"),
            executable_filename,
            string_view("-Wl,/ENTRY:main"),
            string_view("-Wl,/SUBSYSTEM:CONSOLE"),
        };

        const Spawn_Process_Result linker_process_result = platform_spawn_subprocess_and_wait_until_done(test_arena,
                                                                                                         linker_executable,
                                                                                                         linker_arguments,
                                                                                                         NUMBER_OF_STATIC_ARRAY_ELEMENTS(linker_arguments));

        if (linker_process_result.status != SPAWN_PROCESS_OK)
        {
            println("Error: failed to create a subprocess.");

            test_failed = true;
            goto cleanup;
        }

        if (linker_process_result.exit_code != EXIT_SUCCESS)
        {
            println("Error: link failed.");

            if (!strings_are_equal(linker_process_result.stdout, ""))
            {
                println("{}", linker_process_result.stdout);
            }

            if (!strings_are_equal(linker_process_result.stderr, ""))
            {
                println("{}", linker_process_result.stderr);
            }

            test_failed = true;
            goto cleanup;
        }
    }

    // NOTE(vlad): Running test program and testing its output.

    const Spawn_Process_Result test_process_result = platform_spawn_subprocess_and_wait_until_done(test_arena,
                                                                                                   executable_filename,
                                                                                                   NULL,
                                                                                                   0);

    if (test_process_result.status != SPAWN_PROCESS_OK)
    {
        println("Error: failed to create a subprocess.");

        test_failed = true;
        goto cleanup;
    }

    Bool canonize_output = false; // TODO(vlad): Make this a command line argument.

    {
        const String_View exit_code = string_view(format_string(test_arena, "{}", test_process_result.exit_code));
        const String_View expected_exit_code_filename = string_view(format_string(test_arena,
                                                                                  "{}{}expected-exit-code.txt",
                                                                                  configuration.test_directory,
                                                                                  DIRECTORY_SEPARATOR));

        const Bool success = compare_outputs_and_optionally_canonize(test_arena,
                                                                     string_view("Exit code"),
                                                                     expected_exit_code_filename,
                                                                     exit_code,
                                                                     canonize_output);
        test_failed = !success;
    }

    {
        const String_View expected_stdout_filename = string_view(format_string(test_arena,
                                                                               "{}{}expected-stdout.txt",
                                                                               configuration.test_directory,
                                                                               DIRECTORY_SEPARATOR));

        const Bool success = compare_outputs_and_optionally_canonize(test_arena,
                                                                     string_view("stdout"),
                                                                     expected_stdout_filename,
                                                                     test_process_result.stdout,
                                                                     canonize_output);
        test_failed = !success;
    }

    {
        const String_View expected_stderr_filename = string_view(format_string(test_arena,
                                                                               "{}{}expected-stderr.txt",
                                                                               configuration.test_directory,
                                                                               DIRECTORY_SEPARATOR));

        const Bool success = compare_outputs_and_optionally_canonize(test_arena,
                                                                     string_view("stderr"),
                                                                     expected_stderr_filename,
                                                                     test_process_result.stderr,
                                                                     canonize_output);
        test_failed = !success;
    }

cleanup:
    const Timestamp test_end_timestamp = platform_get_current_monotonic_timestamp();
    println("Test took {} mcs to complete", test_end_timestamp - test_start_timestamp);

    return (test_failed ? EXIT_FAILURE : EXIT_SUCCESS);
}

internal void
print_usage_message(Arena* arena, const Argument_Parser* parser)
{
    const String_View VALUE_PLACEHOLDER = string_view("<value>");
    const String_View DESCRIPTION_PLACEHOLDER = string_view("No description");

    ASSERT(!strings_are_equal(parser->executable_name, ""));

    String_Builder builder = {0};
    create_string_builder(&builder, arena);

    append_string(&builder, "Usage: ");
    append_string(&builder, parser->executable_name);
    append_string(&builder, " [options] <test-directory>\n\n");
    append_string(&builder, "Options:\n");

    Size max_option_name_length = 0;

    // NOTE(vlad): Calculating max option name length to correctly pad options and their descriptions.
    {
        for (Index option_index = 0;
             option_index < parser->options_count;
             ++option_index)
        {
            const Argument_Option* option = &parser->options[option_index];

            Size current_option_name_length = option->name.length;

            switch (option->kind)
            {
                case OPTION_WITHOUT_ARGUMENT:
                {
                } break;

                case OPTION_WITH_STRING_ARGUMENT:
                {
                    // NOTE(vlad): Space between the option and its argument.
                    current_option_name_length += 1;

                    if (strings_are_equal(option->argument_name, ""))
                    {
                        current_option_name_length += VALUE_PLACEHOLDER.length;
                    }
                    else
                    {
                        current_option_name_length += option->argument_name.length + 2; // NOTE(vlad): Including '<' and '>'.
                    }
                } break;
            }

            if (max_option_name_length < current_option_name_length)
            {
                max_option_name_length = current_option_name_length;
            }
        }
    }

    for (Index option_index = 0;
         option_index < parser->options_count;
         ++option_index)
    {
        const Argument_Option* option = &parser->options[option_index];

        append_string(&builder, "  ");

        const Size message_length_before_this_option = builder.data_count;

        append_string(&builder, option->name);

        switch (option->kind)
        {
            case OPTION_WITHOUT_ARGUMENT:
            {
            } break;

            case OPTION_WITH_STRING_ARGUMENT:
            {
                append_string(&builder, " ");

                if (strings_are_equal(option->argument_name, ""))
                {
                    append_string(&builder, VALUE_PLACEHOLDER);
                }
                else
                {
                    append_string(&builder, "<");
                    append_string(&builder, option->argument_name);
                    append_string(&builder, ">");
                }
            } break;
        }

        const Size message_length_after_this_option = builder.data_count;
        const Size this_option_length = message_length_after_this_option - message_length_before_this_option;

        const Size right_padding_length = max_option_name_length - this_option_length;
        for (Size i = 0;
             i < right_padding_length;
             ++i)
        {
            append_string(&builder, " ");
        }

        append_string(&builder, " ");

        if (strings_are_equal(option->description, ""))
        {
            append_string(&builder, DESCRIPTION_PLACEHOLDER);
        }
        else
        {
            append_string(&builder, option->description);
        }

        append_string(&builder, "\n");
    }

    println("{}", string_builder_to_string(&builder));
}

#include <eon/argument_parser.c>
#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>

#include <eon/diff.c>

#include <tests/golden-tests-comparison.c>
