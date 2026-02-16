#include <eon/common.h>
#include <eon/io.h>
#include <eon/memory.h>
#include <eon/string.h>

#define LOG_TIMINGS 0

#include <eon/platform/filesystem.h>

#if LOG_TIMINGS
#    include <eon/platform/time.h>
#endif

#include "eon_errors.h"
#include "eon_interpreter.h"
#include "eon_lexer.h"
#include "eon_parser.h"

int
main(const int argc, const char* argv[])
{
    init_io_state(GiB(1));

    if (argc != 2)
    {
        println("Usage: {} <file>", argv[0]);
        return EXIT_FAILURE;
    }

#if LOG_TIMINGS
    const Timestamp start_timestamp = platform_get_current_monotonic_timestamp();
#endif

    Arena* main_arena = arena_create("main", GiB(1), MiB(1));

    const String_View filename = string_view(argv[1]);

#if LOG_TIMINGS
    const Timestamp read_start_timestamp = platform_get_current_monotonic_timestamp();
#endif

    Read_File_Result read_result = platform_read_entire_text_file(main_arena, filename);
    if (read_result.status == READ_FILE_FAILURE)
    {
        println("Failed to read file '{}'", filename);
        return EXIT_FAILURE;
    }

#if LOG_TIMINGS
    const Timestamp read_end_timestamp = platform_get_current_monotonic_timestamp();
    println("File {} read in {} mcs", filename, read_end_timestamp - read_start_timestamp);
#endif

    const String_View code = string_view(read_result.content);

    Lexer lexer = {0};
    Parser parser = {0};

    Arena* errors_arena = arena_create("errors", GiB(1), MiB(1));

    Errors errors = {0};
    errors_create(&errors, errors_arena);

    lexer_create(&lexer, filename, code);
    // TODO(vlad): Make 'parser' the first argument of this function.
    parser_create(&parser, main_arena, &lexer, &errors);

    Arena* scratch_arena = arena_create("scratch", GiB(1), MiB(1));

#if LOG_TIMINGS
    const Timestamp parse_start_timestamp = platform_get_current_monotonic_timestamp();
#endif

    Ast ast = {0};
    if (!parser_parse(main_arena, &parser, &ast))
    {
        for (Index i = 0;
             i < errors.errors_count;
             ++i)
        {
            print_error(scratch_arena, &errors.errors[i]);
        }

        return EXIT_FAILURE;
    }

#if LOG_TIMINGS
    const Timestamp parse_end_timestamp = platform_get_current_monotonic_timestamp();
    println("File {} parsed in {} mcs",
            filename,
            parse_end_timestamp - parse_start_timestamp);
#endif

    Arena* scopes_arena = arena_create("interpreter-lexical-scopes", GiB(1), MiB(1));

    // TODO(vlad): Add type system.
    Interpreter interpreter = {0};
    interpreter_create(&interpreter, scopes_arena);

    Arena* runtime_arena = arena_create("interpreter-runtime", GiB(1), MiB(1));
    Arena* result_arena = arena_create("interpreter-result", GiB(1), MiB(1));
    Call_Info call_info = {0};

#if LOG_TIMINGS
    const Timestamp interpret_start_timestamp = platform_get_current_monotonic_timestamp();
#endif

    const Run_Result result = interpreter_execute_function(runtime_arena,
                                                           result_arena,
                                                           &interpreter,
                                                           &ast,
                                                           string_view("main"),
                                                           &call_info);
#if LOG_TIMINGS
    const Timestamp interpret_end_timestamp = platform_get_current_monotonic_timestamp();
    println("File {} interpreted in {} mcs",
            filename,
            interpret_end_timestamp - interpret_start_timestamp);
#endif

    if (result.status == INTERPRETER_RUN_COMPILE_ERROR)
    {
        println("Compile error encountered: {}", result.error);
        return EXIT_FAILURE;
    }
    else if (result.status == INTERPRETER_RUN_RUNTIME_ERROR)
    {
        println("Runtime error encountered: {}", result.error);
        return EXIT_FAILURE;
    }

#if LOG_TIMINGS
    const Timestamp end_timestamp = platform_get_current_monotonic_timestamp();
    println("Program execution took {} msc", end_timestamp - start_timestamp);
#endif

    interpreter_destroy(&interpreter);
    parser_destroy(&parser);
    errors_destroy(&errors);
    lexer_destroy(&lexer);

    arena_destroy(result_arena);
    arena_destroy(runtime_arena);
    arena_destroy(scopes_arena);
    arena_destroy(scratch_arena);
    arena_destroy(errors_arena);
    arena_destroy(main_arena);

    return result.result.s32_value;
}

#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>

#include "eon_errors.c"
#include "eon_interpreter.c"
#include "eon_lexer.c"
#include "eon_parser.c"
