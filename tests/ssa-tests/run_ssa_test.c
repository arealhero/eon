// FIXME(vlad): Rename this file to something like 'run_pipeline_test' or 'run_ir_pipeline_test'.

#include <eon/common.h>
#include <eon/diff.h>
#include <eon/memory.h>
#include <eon/string.h>

#include <eon/platform/filesystem.h>
#include <eon/platform/time.h>
#include <eon/platform/entry_point.h>

#include <tests/converters.h>

#include <eon_cfg.h>
#include <eon_compilation_context.h>
#include <eon_lexer.h>
#include <eon_lexical_scopes.h>
#include <eon_machine_ir.h>
#include <eon_parser.h>
#include <eon_ssa.h>
#include <eon_tac.h>
#include <eon_types.h>

#define ENABLE_TIMER 0

#if ENABLE_TIMER
#    define START_TIMER(name)                                           \
    const Timestamp name##_start = platform_get_current_monotonic_timestamp()

#    define END_TIMER(name, message)                                    \
    do                                                                  \
    {                                                                   \
        const Timestamp name##_end = platform_get_current_monotonic_timestamp(); \
        println("{} in {} mcs", message, name##_end - name##_start);    \
    }                                                                   \
    while (0)
#else
#    define START_TIMER(name)
#    define END_TIMER(name, message)
#endif

struct Arena_Provider
{
    s32 dummy_field;
};
typedef struct Arena_Provider Arena_Provider;

internal inline void
print_usage(void)
{
    println("Usage: run_ssa_test <directory> [canonize]");
}

internal Bool compare_outputs_and_optionally_canonize(Arena* scratch_arena,
                                                      const String_View test_name,
                                                      const String_View filename,
                                                      const String_View output,
                                                      const Bool canonize_output);

internal int
eon_main(const String_View* arguments, const Size arguments_count)
{
    Bool canonize_output = false;
    String_View test_directory = {0};

    if (arguments_count == 2)
    {
        test_directory = arguments[1];
    }
    else if (arguments_count == 3)
    {
        test_directory = arguments[1];

        if (strings_are_equal(arguments[2], "canonize"))
        {
            canonize_output = true;
        }
        else
        {
            println("Unknown argument encountered: '{}'", arguments[2]);
            print_usage();
            return EXIT_FAILURE;
        }
    }
    else
    {
        print_usage();
        return EXIT_FAILURE;
    }

    Arena* source_code_arena = create_arena("source-code", GiB(1), MiB(1));
    Arena* ssa_string_arena = create_arena("ssa-string", GiB(1), MiB(1));

    Arena_Provider arena_provider = {0};
    Compilation_Context context = {0};

    const Timestamp test_start_timestamp = platform_get_current_monotonic_timestamp();

    {
        const String_View main_filename = string_view(format_string(source_code_arena, "{}/main.eon", test_directory));

        START_TIMER(reading_file);
        const Read_File_Result result = platform_read_entire_text_file(source_code_arena, main_filename);
        END_TIMER(reading_file, "File read");

        if (result.status != READ_FILE_SUCCESS)
        {
            println("Error: failed to read file {}", main_filename);
            destroy_arena(ssa_string_arena);
            destroy_arena(source_code_arena);
            return EXIT_FAILURE;
        }

        Source_File source_file = {0};
        source_file.filename = main_filename;
        source_file.code = string_view(result.content);

        START_TIMER(context_created);
        create_compilation_context(&context, &arena_provider, &source_file);
        END_TIMER(context_created, "Compilation context created");
    }

    // FIXME(vlad): We should test all available calling conventions and architectures here.
    context.calling_convention = CALLING_CONVENTION_MICROSOFT_X64;
    context.target_architecture = TARGET_ARCH_X86_64;

    Bool test_failed = false;

    Lexer lexer = {0};
    Parser parser = {0};

    create_lexer(&lexer, &context);
    create_parser(&parser, &lexer, &context);

    START_TIMER(ast_parsing);
    if (!parse_ast(&parser))
    {
        println("Error: failed to parse {}", context.source_file.filename);
        test_failed = true;
        goto cleanup;
    }
    END_TIMER(ast_parsing, "AST parsed");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    START_TIMER(ast_validation);
    validate_ast(&context);
    END_TIMER(ast_validation, "AST validated");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    START_TIMER(lexical_scopes_creating);
    create_lexical_scopes(&context);
    END_TIMER(lexical_scopes_creating, "Lexical scopes created");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    START_TIMER(types_resolved);
    resolve_and_validate_types(&context);
    END_TIMER(types_resolved, "Types resolved");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    START_TIMER(tac_built);
    lower_ast_to_tac(&context);
    END_TIMER(tac_built, "AST lowered to TAC");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    START_TIMER(cfg_built);
    construct_cfg_from_tac(&context);
    END_TIMER(cfg_built, "CFG built");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    START_TIMER(ssa_construction);
    construct_ssa_from_cfg(&context);
    END_TIMER(ssa_construction, "SSA constructed");

    {
        START_TIMER(comparing_plain_ssa);
        const String_View plain_ssa_string = convert_ssa_to_string(ssa_string_arena, &context);
        const String_View plain_ssa_filename = string_view(format_string(source_code_arena, "{}/plain.ssa", test_directory));

        const Bool success = compare_outputs_and_optionally_canonize(context.scratch_arena,
                                                                     string_view("Plain SSA"),
                                                                     plain_ssa_filename,
                                                                     plain_ssa_string,
                                                                     canonize_output);
        test_failed = !success;

        END_TIMER(comparing_plain_ssa, "Plain SSA processed");
    }

    START_TIMER(unused_ssa_assignments);
    find_unused_ssa_assignments(&context);
    END_TIMER(unused_ssa_assignments, "Unused SSA assignments checked");

    START_TIMER(constant_folding);
    perform_constant_folding(&context);
    END_TIMER(constant_folding, "Constant folding performed");

    START_TIMER(unreachable_jumps_removal);
    remove_unreachable_jumps(&context);
    END_TIMER(unreachable_jumps_removal, "Unreachable jumps removed");

    START_TIMER(unreachable_cfg_blocks_removed_v2);
    remove_unreachable_cfg_blocks(&context);
    END_TIMER(unreachable_cfg_blocks_removed_v2, "Unreachable CFG blocks removed");

    {
        START_TIMER(comparing_ssa_after_constant_folding);
        const String_View ssa_string_after_constant_folding = convert_ssa_to_string(ssa_string_arena, &context);
        const String_View ssa_after_constant_folding_filename = string_view(format_string(source_code_arena, "{}/after-constant-folding.ssa", test_directory));

        const Bool success = compare_outputs_and_optionally_canonize(context.scratch_arena,
                                                                     string_view("SSA after constant folding"),
                                                                     ssa_after_constant_folding_filename,
                                                                     ssa_string_after_constant_folding,
                                                                     canonize_output);
        test_failed = !success;

        END_TIMER(comparing_ssa_after_constant_folding, "SSA after constant folding processed");
    }

    START_TIMER(ssa_to_mir_lowering);
    lower_ssa_to_mir(&context);
    END_TIMER(ssa_to_mir_lowering, "SSA lowered to MIR");

    {
        START_TIMER(comparing_mir);
        const String_View mir_string = convert_mir_to_string(ssa_string_arena, &context);
        const String_View mir_filename = string_view(format_string(source_code_arena, "{}/plain.mir", test_directory));

        const Bool success = compare_outputs_and_optionally_canonize(context.scratch_arena,
                                                                     string_view("MIR"),
                                                                     mir_filename,
                                                                     mir_string,
                                                                     canonize_output);
        test_failed = !success;

        END_TIMER(comparing_mir, "MIR processed");
    }

    START_TIMER(isa_constraints_gathering);
    add_isa_constraints_to_mir(&context);
    END_TIMER(isa_constraints_gathering, "ISA constraints added to MIR");

    {
        START_TIMER(comparing_mir_after_isa_constraints);
        const String_View mir_string = convert_mir_to_string(ssa_string_arena, &context);
        const String_View mir_filename = string_view(format_string(source_code_arena, "{}/after-isa-constraints.mir", test_directory));

        const Bool success = compare_outputs_and_optionally_canonize(context.scratch_arena,
                                                                     string_view("MIR"),
                                                                     mir_filename,
                                                                     mir_string,
                                                                     canonize_output);
        test_failed = !success;

        END_TIMER(comparing_mir_after_isa_constraints, "Processed MIR after ISA constraints gathering");
    }

    START_TIMER(register_allocation);
    allocate_registers(&context);
    END_TIMER(register_allocation, "Registers allocated");

    {
        START_TIMER(comparing_mir_after_register_allocation);
        const String_View mir_string = convert_mir_to_string(ssa_string_arena, &context);
        const String_View mir_filename = string_view(format_string(source_code_arena,
                                                                   "{}/after-register-allocation.mir",
                                                                   test_directory));

        const Bool success = compare_outputs_and_optionally_canonize(context.scratch_arena,
                                                                     string_view("MIR"),
                                                                     mir_filename,
                                                                     mir_string,
                                                                     canonize_output);
        test_failed = !success;

        END_TIMER(comparing_mir_after_register_allocation, "Processed MIR after register allocation");
    }

    START_TIMER(mir_blocks_layout);
    compute_layout_of_mir_blocks(&context);
    END_TIMER(mir_blocks_layout, "Layout of MIR blocks computed");

    {
        START_TIMER(comparing_mir_after_layout_is_computed);
        const String_View mir_string = convert_mir_to_string(ssa_string_arena, &context);
        const String_View mir_filename = string_view(format_string(source_code_arena,
                                                                   "{}/after-blocks-layout-is-computed.mir",
                                                                   test_directory));

        const Bool success = compare_outputs_and_optionally_canonize(context.scratch_arena,
                                                                     string_view("MIR"),
                                                                     mir_filename,
                                                                     mir_string,
                                                                     canonize_output);
        test_failed = !success;

        END_TIMER(comparing_mir_after_layout_is_computed, "Processed MIR after blocks layout is computed");
    }

cleanup:
    {
        START_TIMER(comparing_diagnostic_messages);

#if OS_WINDOWS
        const String_View plain_diagnostic_messages = dump_diagnostic_messages(context.scratch_arena, &context, MAX_MESSAGE_LEVEL);
        String diagnostic_messages_copy = copy_string(context.scratch_arena, plain_diagnostic_messages);

        Bool should_replace_slashes = true;
        for (Index i = 0;
             i < diagnostic_messages_copy.length;
             ++i)
        {
            switch (diagnostic_messages_copy.data[i])
            {
                case '\\':
                {
                    if (should_replace_slashes)
                    {
                        diagnostic_messages_copy.data[i] = '/';
                    }
                } break;

                case ' ':
                {
                    should_replace_slashes = false;
                } break;

                case '\n':
                {
                    should_replace_slashes = true;
                }
            }
        }
        const String_View diagnostic_messages = string_view(diagnostic_messages_copy);
#else
        const String_View diagnostic_messages = dump_diagnostic_messages(context.scratch_arena, &context, MAX_MESSAGE_LEVEL);
#endif

        const String_View diagnostics_filename = string_view(format_string(source_code_arena, "{}/diagnostics.out", test_directory));

        const Bool success = compare_outputs_and_optionally_canonize(context.scratch_arena,
                                                                     string_view("Diagnostics"),
                                                                     diagnostics_filename,
                                                                     diagnostic_messages,
                                                                     canonize_output);
        test_failed = !success;

        END_TIMER(comparing_diagnostic_messages, "Diagnostic messages compared");
    }

    const Timestamp test_end_timestamp = platform_get_current_monotonic_timestamp();
    println("Test took {} mcs to complete", test_end_timestamp - test_start_timestamp);

    destroy_compilation_context(&context);
    destroy_parser(&parser);
    destroy_lexer(&lexer);

    destroy_arena(ssa_string_arena);

    return test_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

internal Arena*
acquire_arena_from_provider(Arena_Provider* provider,
                            const String_View arena_name,
                            const Size number_of_bytes_to_reserve,
                            const Size number_of_bytes_to_commit)
{
    UNUSED(provider);
    return create_arena(arena_name, number_of_bytes_to_reserve, number_of_bytes_to_commit);
}

internal void
request_arena_reset(Arena_Provider* provider, Arena* arena)
{
    UNUSED(provider);
    arena_clear(arena);
}

internal void
release_arena_to_provider(Arena_Provider* provider, Arena* arena)
{
    UNUSED(provider);
    destroy_arena(arena);
}

internal Bool
compare_outputs_and_optionally_canonize(Arena* scratch_arena,
                                        const String_View test_name,
                                        const String_View filename,
                                        const String_View output,
                                        const Bool canonize_output)
{
    const Read_File_Result result = platform_read_entire_text_file(scratch_arena, filename);

    String_View canon = {0};
    if (result.status == READ_FILE_SUCCESS)
    {
        canon = string_view(result.content);
    }

    // NOTE(vlad): Alas we cannot just use 'strings_are_equal' on Windows because it uses CRLF by default and that
    //             messes up the comparison. We could in theory emit a '\r\n' instead of '\n' during the output
    //             construction, but that will be fragile as well: git on Windows supports both CRLF and LF checkouts so
    //             we do not know what line ending style is used right now.
    //
    //             Also note that we will use 'strings_are_equal' on other platforms because we will not tolerate
    //             CRLF-style canon files in our repository.

#if OS_WINDOWS
    Index current_canon_index = 0;
    Index current_output_index = 0;

    const Bool canon_is_empty = canon.length == 0;
    const Bool output_is_empty = output.length == 0;

    Bool diff_detected = canon_is_empty ^ output_is_empty;

    while (current_canon_index < canon.length && current_output_index < output.length)
    {
        String_View canon_line = get_next_line(canon, current_canon_index);
        String_View output_line = get_next_line(output, current_output_index);

        current_canon_index += canon_line.length + 1;
        current_output_index += output_line.length + 1;

        if (canon_line.length != 0 && canon_line.data[canon_line.length - 1] == '\r')
        {
            // NOTE(vlad): CRLF detected.
            canon_line.length -= 1;
        }

        if (!strings_are_equal(canon_line, output_line))
        {
            diff_detected = true;
            break;
        }
    }

    if (!diff_detected)
    {
        if (canonize_output)
        {
            println("{}: Outputs are the same", test_name);
        }

        return true;
    }
#else
    if (strings_are_equal(output, canon))
    {
        if (canonize_output)
        {
            println("{}: Outputs are the same", test_name);
        }

        return true;
    }
#endif

    if (canonize_output)
    {
        platform_write_string_to_file(scratch_arena, filename, output);
        println("{}: Output canonized", test_name);
        return true;
    }

    if (canon.length == 0)
    {
        println("{}: Canon is empty. Current output:\n\n{}", test_name, output);
        return false;
    }
    else
    {
        const Diff diff = calculate_line_diff(scratch_arena, canon, output);
        const String_View diff_string = line_diff_to_string(scratch_arena, &diff);

        println("{}: Test failed with this diff:\n\n{}", test_name, diff_string);
        return false;
    }
}

#include <eon/diff.c>
#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>

#include <tests/converters.c>

#include <eon_ast.c>
#include <eon_cfg.c>
#include <eon_compilation_context.c>
#include <eon_diagnostics.c>
#include <eon_lexer.c>
#include <eon_lexical_scopes.c>
#include <eon_machine_ir.c>
#include <eon_parser.c>
#include <eon_ssa.c>
#include <eon_tac.c>
#include <eon_types.c>
