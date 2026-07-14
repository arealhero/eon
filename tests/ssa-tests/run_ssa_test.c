#include <eon/common.h>
#include <eon/diff.h>
#include <eon/memory.h>
#include <eon/string.h>

#include <eon/platform/filesystem.h>
#include <eon/platform/time.h>

#include <eon_cfg.h>
#include <eon_compilation_context.h>
#include <eon_lexer.h>
#include <eon_lexical_scopes.h>
#include <eon_parser.h>
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

internal String_View convert_ssa_to_string(Arena* arena, Compilation_Context* context);

internal Bool compare_outputs_and_optionally_canonize(Arena* scratch_arena,
                                                      const String_View test_name,
                                                      const String_View filename,
                                                      const String_View output,
                                                      const Bool canonize_output);

int
main(const int argc, const char* argv[])
{
    init_io_state(GiB(1));

    Bool canonize_output = false;
    String_View test_directory = {0};

    if (argc == 2)
    {
        test_directory = string_view(argv[1]);
    }
    else if (argc == 3)
    {
        test_directory = string_view(argv[1]);

        if (strings_are_equal(string_view(argv[2]), string_view("canonize")))
        {
            canonize_output = true;
        }
        else
        {
            println("Unknown argument {} encountered", argv[2]);
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

    // TODO(vlad): 'find_unused_ssa_assignments()'.

    START_TIMER(constant_folding);
    perform_constant_folding(&context);
    END_TIMER(constant_folding, "Constant folding performed");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    START_TIMER(unreachable_jumps_removal);
    remove_unreachable_jumps(&context);
    END_TIMER(unreachable_jumps_removal, "Unreachable jumps removed");

    if (has_diagnostic_messages(&context))
    {
        test_failed = true;
        goto cleanup;
    }

    {
        START_TIMER(unreachable_cfg_blocks_removed_v2);
        remove_unreachable_cfg_blocks(&context);
        END_TIMER(unreachable_cfg_blocks_removed_v2, "Unreachable CFG blocks removed");
    }

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

cleanup:
    {
        if (has_diagnostic_messages(&context))
        {
            const String messages = dump_diagnostic_messages(context.scratch_arena, &context, MAX_MESSAGE_LEVEL);
            println("{}", messages);
        }

        const Timestamp test_end_timestamp = platform_get_current_monotonic_timestamp();
        println("Test took {} mcs to complete", test_end_timestamp - test_start_timestamp);

        destroy_compilation_context(&context);
        destroy_parser(&parser);
        destroy_lexer(&lexer);
    }

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

// FIXME(vlad): Use 'tac_function->first_tac_variable_index' instead.
struct Conversion_Context
{
    Index temporary_variables_offset;
};
typedef struct Conversion_Context Conversion_Context;

internal void
convert_tac_operand_to_string(Compilation_Context* context,
                              String_Builder* builder,
                              const Tac_Operand* operand,
                              Conversion_Context* conversion_context)
{
    Tac* tac = &context->tac;

    switch (operand->kind)
    {
        case TAC_OPERAND_NONE:
        {
        } break;

        case TAC_OPERAND_FUNCTION_LABEL:
        {
            const Tac_Function_Label_Id function_label_id = operand->function_label_id;
            const Tac_Function* function = get_tac_function_by_label(tac, function_label_id);

            append_string(builder, string_view(" "));
            append_string(builder, function->ast_function_definition->name.token.lexeme);
        } break;

        case TAC_OPERAND_VARIABLE:
        {
            const Tac_Variable_Id variable_id = operand->variable_id;
            ASSERT(variable_id.index != INVALID_TAC_INDEX);
            ASSERT(variable_id.ssa_version != SSA_VERSION_UNDEFINED);
            ASSERT(variable_id.ssa_version != SSA_VERSION_UNSET);

            const Tac_Variable* variable = &tac->variables[variable_id.index];

            String_View name = {0};

            if (variable->is_temporary)
            {
                if (conversion_context->temporary_variables_offset == 0)
                {
                    conversion_context->temporary_variables_offset = variable_id.index;
                }

                const Index temporary_variable_index = variable_id.index - conversion_context->temporary_variables_offset + 1;
                name = string_view(format_string(context->scratch_arena, "<temp_{}>", temporary_variable_index));
            }
            else
            {
                const Symbol* symbol = get_symbol_by_id(context, variable->symbol_id);
                name = symbol->name;
            }

            append_string(builder, string_view(format_string(context->scratch_arena, " VARIABLE {}@{}", name, variable_id.ssa_version)));
        } break;

        case TAC_OPERAND_LABEL:
        {
            ASSERT(operand->label_id.index != INVALID_TAC_INDEX);
            append_string(builder, string_view(format_string(context->scratch_arena, " LABEL_{}", operand->label_id.index)));
        } break;

        case TAC_OPERAND_CONSTANT:
        {
            const Tac_Constant_Id constant_id = operand->constant_id;
            ASSERT(constant_id.index != INVALID_TAC_INDEX);

            const Tac_Constant* constant = &tac->constants[constant_id.index];

            append_string(builder, string_view(" CONSTANT "));

            String_View constant_string = {0};

            switch (constant->kind)
            {
                case TAC_CONSTANT_UNDEFINED:
                {
                    UNREACHABLE();
                } break;

                case TAC_CONSTANT_BOOLEAN:
                {
                    if (constant->boolean_value)
                    {
                        constant_string = string_view("bool TRUE");
                    }
                    else
                    {
                        constant_string = string_view("bool FALSE");
                    }
                } break;

#define DECLARE_INTEGER_CASE(kind, Type)                                \
                case kind:                                              \
                {                                                       \
                    constant_string = string_view(format_string(context->scratch_arena, #Type " {}", constant->integer_value)); \
                } break

                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT8,  s8);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT16, s16);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT32, s32);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_INT64, s64);

                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT8,  u8);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT16, u16);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT32, u32);
                DECLARE_INTEGER_CASE(TAC_CONSTANT_UINT64, u64);

                case TAC_CONSTANT_FLOAT32:
                {
                    constant_string = string_view(format_string(context->scratch_arena, "f32 {}", constant->float32_value));
                } break;

                case TAC_CONSTANT_FLOAT64:
                {
                    constant_string = string_view(format_string(context->scratch_arena, "f64 {}", constant->float64_value));
                } break;
            }

            ASSERT(!strings_are_equal(constant_string, string_view("")));

            append_string(builder, string_view(constant_string));
        } break;

        case TAC_OPERAND_PARAMETER_INDEX:
        {
            append_string(builder, string_view(format_string(context->scratch_arena, " ARGUMENT {}", operand->parameter_index.index)));
        } break;
    }
}

internal String_View
convert_ssa_to_string(Arena* arena, Compilation_Context* context)
{
    const Tac* tac = &context->tac;

    String_Builder builder = {0};
    create_string_builder(&builder, arena);

    for (Index function_index = 0;
         function_index < tac->functions_count;
         ++function_index)
    {
        const Tac_Function* tac_function = &tac->functions[function_index];
        const Ast_Function_Definition* ast_definition = tac_function->ast_function_definition;

        append_string(&builder, ast_definition->name.token.lexeme);
        append_string(&builder, string_view(":\n"));

        Index current_instruction_index = 1;

        Conversion_Context conversion_context = {0};

        for (Index block_index = 0;
             block_index < tac_function->cfg_blocks_count;
             ++block_index)
        {
            const Cfg_Block* block = &tac_function->cfg_blocks[block_index];

            if (block->phi_nodes_count > 0)
            {
                append_string(&builder, string_view("       |\n"));
            }

            for (Index phi_node_index = 0;
                 phi_node_index < block->phi_nodes_count;
                 ++phi_node_index)
            {
                append_string(&builder, string_view("       | "));
                append_string(&builder, string_view("          PHI             "));
                const Phi_Node* phi_node = &block->phi_nodes[phi_node_index];

                ASSERT(phi_node->previous_variables_count == block->predecessors_count);

                Tac_Operand destination = {0};
                destination.kind = TAC_OPERAND_VARIABLE;
                destination.variable_id = phi_node->destination;

                convert_tac_operand_to_string(context, &builder, &destination, &conversion_context);

                for (Index argument_index = 0;
                     argument_index < phi_node->previous_variables_count;
                     ++argument_index)
                {
                    Tac_Operand previous_variable = {0};
                    previous_variable.kind = TAC_OPERAND_VARIABLE;
                    previous_variable.variable_id = phi_node->previous_variables[argument_index];

                    if (previous_variable.variable_id.ssa_version != SSA_VERSION_UNSET)
                    {
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &previous_variable, &conversion_context);
                    }
                }

                append_string(&builder, string_view("\n"));
            }

            const Tac_Instructions_Range* range = &block->instructions_range;

            for (Index instruction_index = range->start_instruction_index;
                 instruction_index < range->end_instruction_index;
                 ++instruction_index)
            {
                const Tac_Instruction* instruction = &tac_function->instructions[instruction_index];

                if (instruction->operation == TAC_NOP)
                {
                    // NOTE(vlad): Sanity check.
                    ASSERT(instruction->destination.kind == TAC_OPERAND_NONE);
                    ASSERT(instruction->first_argument.kind == TAC_OPERAND_NONE);
                    ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                    continue;
                }

                append_string(&builder, string_view(format_string(context->scratch_arena,
                                                                  "{left-pad-count: 6} | ",
                                                                  current_instruction_index)));
                current_instruction_index += 1;

                switch (instruction->operation)
                {
                    case TAC_NOP:
                    {
                        UNREACHABLE();
                    } break;

                    case TAC_ASSIGN:
                    {
                        append_string(&builder, string_view("          ASSIGN          "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_GET_ADDRESS:
                    {
                        append_string(&builder, string_view("          GET_ADDRESS     "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_LOAD_BY_ADDRESS:
                    {
                        append_string(&builder, string_view("          LOAD_BY_ADDRESS "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_STORE_BY_ADDRESS:
                    {
                        append_string(&builder, string_view("          STORE_BY_ADDRESS"));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_ADD:
                    {
                        append_string(&builder, string_view("          ADD             "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_SUBTRACT:
                    {
                        append_string(&builder, string_view("          SUBTRACT        "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_MULTIPLY:
                    {
                        append_string(&builder, string_view("          MULTIPLY        "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_DIVIDE:
                    {
                        append_string(&builder, string_view("          DIVIDE          "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_EQUAL:
                    {
                        append_string(&builder, string_view("          EQUAL           "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_NOT_EQUAL:
                    {
                        append_string(&builder, string_view("          NOT_EQUAL       "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_LESS:
                    {
                        append_string(&builder, string_view("          LESS            "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_LESS_OR_EQUAL:
                    {
                        append_string(&builder, string_view("          LESS_OR_EQUAL   "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_GREATER:
                    {
                        append_string(&builder, string_view("          GREATER         "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_GREATER_OR_EQUAL:
                    {
                        append_string(&builder, string_view("          GREATER_OR_EQUAL"));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_VARIABLE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind != TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->second_argument, &conversion_context);
                    } break;

                    case TAC_LABEL:
                    {
                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        ASSERT(instruction->destination.label_id.index != INVALID_TAC_INDEX);
                        append_string(&builder, string_view(format_string(context->scratch_arena, "LABEL_{}:", instruction->destination.label_id.index)));
                    } break;

                    case TAC_JUMP:
                    {
                        append_string(&builder, string_view("          JUMP            "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                    } break;

                    case TAC_JUMP_IF_TRUE:
                    {
                        append_string(&builder, string_view("          JUMP_IF_TRUE    "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_JUMP_IF_FALSE:
                    {
                        append_string(&builder, string_view("          JUMP_IF_FALSE   "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_LABEL);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_SET_PARAMETER:
                    {
                        append_string(&builder, string_view("          SET_PARAMETER   "));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_GET_PARAMETER:
                    {
                        append_string(&builder, string_view("          GET_PARAMETER   "));

                        ASSERT(instruction->destination.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->first_argument.kind == TAC_OPERAND_PARAMETER_INDEX);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                        append_string(&builder, string_view(","));
                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_CALL:
                    {
                        append_string(&builder, string_view("          CALL            "));
                        ASSERT(instruction->first_argument.kind != TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        if (instruction->destination.kind != TAC_OPERAND_NONE)
                        {
                            convert_tac_operand_to_string(context, &builder, &instruction->destination, &conversion_context);
                            append_string(&builder, string_view(","));
                        }

                        convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                    } break;

                    case TAC_RETURN:
                    {
                        append_string(&builder, string_view("          RETURN"));

                        ASSERT(instruction->destination.kind == TAC_OPERAND_NONE);
                        ASSERT(instruction->second_argument.kind == TAC_OPERAND_NONE);

                        if (instruction->first_argument.kind != TAC_OPERAND_NONE)
                        {
                            append_string(&builder, string_view("          "));
                            convert_tac_operand_to_string(context, &builder, &instruction->first_argument, &conversion_context);
                        }
                    } break;
                }

                append_string(&builder, string_view("\n"));
            }
        }

        if (function_index != tac->functions_count - 1)
        {
            append_string(&builder, string_view("\n"));
        }
    }

    return string_builder_to_string(&builder);
}

internal Bool
compare_outputs_and_optionally_canonize(Arena* scratch_arena,
                                        const String_View test_name,
                                        const String_View filename,
                                        const String_View output,
                                        const Bool canonize_output)
{
    const Read_File_Result result = platform_read_entire_text_file(scratch_arena, filename);

    const String_View canon = string_view(result.content);

    if (strings_are_equal(output, canon))
    {
        if (canonize_output)
        {
            println("{}: Outputs are the same, so there is no need to canonize", test_name);
        }

        return true;
    }

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

#include <eon_cfg.c>
#include <eon_compilation_context.c>
#include <eon_diagnostics.c>
#include <eon_lexer.c>
#include <eon_lexical_scopes.c>
#include <eon_parser.c>
#include <eon_tac.c>
#include <eon_types.c>
