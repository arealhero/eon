#include <eon_cfg.h>
#include <eon_compilation_context.h>
#include <eon_lexer.h>
#include <eon_lexical_scopes.h>
#include <eon_machine_ir.h>
#include <eon_parser.h>
#include <eon_ssa.h>
#include <eon_tac.h>
#include <eon_types.h>

#include <eon/io.h>
#include <eon/object/elf.h>

#include <eon/platform/entry_point.h>
#include <eon/platform/filesystem.h>
#include <eon/platform/time.h>

#define ENABLE_TIMER 1

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
    println("Usage: eon <filename>");
}

struct Pipeline_Step
{
    String_View name;
    void (*run)(Compilation_Context* context);
};
typedef struct Pipeline_Step Pipeline_Step;

internal int
eon_main(const String_View* arguments, const Size arguments_count)
{
    if (arguments_count != 2)
    {
        print_usage();
        return EXIT_FAILURE;
    }

    const String_View filename = arguments[1];

    Arena* source_code_arena = create_arena("source-code", MiB(10), MiB(1));

    Arena_Provider arena_provider = {0};
    Compilation_Context context = {0};

    const Timestamp build_start_timestamp = platform_get_current_monotonic_timestamp();

    {
        START_TIMER(reading_file);
        const Read_File_Result result = platform_read_entire_text_file(source_code_arena, filename);
        END_TIMER(reading_file, "File read");

        if (result.status != READ_FILE_SUCCESS)
        {
            println("Error: failed to read file {}", filename);
            destroy_arena(source_code_arena);
            return EXIT_FAILURE;
        }

        Source_File source_file = {0};
        source_file.filename = filename;
        source_file.code = string_view(result.content);

        START_TIMER(context_created);
        create_compilation_context(&context, &arena_provider, &source_file);
        END_TIMER(context_created, "Compilation context created");
    }

    Bool build_failed = false;

    Pipeline_Step steps[] = {
        { string_view("AST parsed"), parse_ast },
        { string_view("AST validated"), validate_ast },
        { string_view("Lexical scopes created"), create_lexical_scopes },
        { string_view("Types resolved"), resolve_and_validate_types },
        { string_view("TAC constructed"), lower_ast_to_tac },
        { string_view("CFG constructed"), construct_cfg_from_tac },
        { string_view("SSA constructed"), construct_ssa_from_cfg },
        { string_view("Unused SSA assignments found"), find_unused_ssa_assignments },
        { string_view("Constant folding performed"), perform_constant_folding },
        { string_view("Unreachable jumps removed"), remove_unreachable_jumps },
        { string_view("Unreachable CFG blocks removed"), remove_unreachable_cfg_blocks },
        { string_view("MIR constructed"), lower_ssa_to_mir },
        { string_view("ISA constraints gathered"), add_isa_constraints_to_mir },
        { string_view("Registers allocated"), allocate_registers },
        { string_view("Layout of MIR blocks computed"), compute_layout_of_mir_blocks },
        { string_view("Binary code emitted"), emit_binary_code },
        { string_view("Object file built"), build_elf64_object_file }, // TODO(vlad): Remove hardcoded elf64-specific function.
    };

    for (Index step_index = 0;
         step_index < NUMBER_OF_STATIC_ARRAY_ELEMENTS(steps);
         ++step_index)
    {
        const Pipeline_Step* step = &steps[step_index];

        START_TIMER(step_timer);
        step->run(&context);
        END_TIMER(step_timer, step->name);

        if (has_diagnostic_messages(&context))
        {
            build_failed = true;
            break;
        }
    }

    if (!has_diagnostic_messages(&context))
    {
        START_TIMER(object_file_writing);
        String_View object_file_prefix = filename;

        if (filename.length > 4)
        {
            String_View possible_extension = get_substring(object_file_prefix,
                                                           object_file_prefix.length - 4,
                                                           4);

            if (strings_are_equal(possible_extension, ".eon"))
            {
                chop_string_suffix(&object_file_prefix, 4);
            }
        }

        const String_View object_filename = string_view(format_string(context.scratch_arena,
                                                                      "{}.o",
                                                                      object_file_prefix));

        // TODO(vlad): Do not pass arena here. We can construct the zero-terminated filename ourselves.
        const Platform_File_State file_state = platform_open_binary_file_for_writing(context.scratch_arena,
                                                                                     object_filename);

        Bytes_View object_file_view = {0};
        object_file_view.data = context.object_file_bytes;
        object_file_view.data_size_in_bytes = context.object_file_bytes_count;

        platform_write_binary_blob_to_file(file_state, object_file_view);

        platform_close_file(file_state);

        END_TIMER(object_file_writing, "Object file wrote");
    }

    const String_View diagnostic_messages = dump_diagnostic_messages(context.scratch_arena, &context, MAX_MESSAGE_LEVEL);
    println("{}", diagnostic_messages);

    const Timestamp build_end_timestamp = platform_get_current_monotonic_timestamp();
    println("Build took {} mcs to complete", build_end_timestamp - build_start_timestamp);

    destroy_compilation_context(&context);

    return (build_failed ? EXIT_FAILURE : EXIT_SUCCESS);
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

#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>

#include <eon/object/elf.c>
