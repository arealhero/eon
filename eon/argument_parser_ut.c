#include <eon/unit_test.h>

#include "argument_parser.h"

internal inline void
reset_parser(Argument_Parser* parser)
{
    fill_with_zeros(parser->option_was_set, parser->options_count, Bool);

    parser->free_arguments_count = 0;
}

// TODO(vlad): Test arguments parser without options -- every argument must be in the free_arguments array.

internal void
test_example_compiler_arguments_parsing(Test_Context* test_context)
{
    struct Test_Configuration
    {
        Bool verbose;
        String_View arch;
        String_View object_file_type;
    };
    typedef struct Test_Configuration Test_Configuration;

    Test_Configuration configuration = {0};

    const Argument_Option options[] = {
        {
            .name = string_view("--verbose"),
            .kind = OPTION_WITHOUT_ARGUMENT,
            .flag = &configuration.verbose,
        },
        {
            .name = string_view("--arch"),
            .kind = OPTION_WITH_STRING_ARGUMENT,
            .string = &configuration.arch,
            .default_value.string = string_view("x86_64"),
        },
        {
            .name = string_view("--object-file-type"),
            .kind = OPTION_WITH_STRING_ARGUMENT,
            .string = &configuration.object_file_type,
            .default_value.string = string_view("elf64"),
        },
    };

    Argument_Parser parser = create_argument_parser(test_context->arena,
                                                    options,
                                                    NUMBER_OF_STATIC_ARRAY_ELEMENTS(options));

    {
        const String_View arguments[] = {
            string_view("filename"),
            string_view("--arch"),
            string_view("x86_64"),
            string_view("--object-file-type"),
            string_view("elf64"),
            string_view("--verbose"),
        };
        const Size arguments_count = NUMBER_OF_STATIC_ARRAY_ELEMENTS(arguments);

        reset_parser(&parser);

        configuration = (Test_Configuration){0};

        parse_arguments(&parser, arguments, arguments_count);

        ASSERT_EQUAL(configuration.verbose, true);
        ASSERT_STRINGS_ARE_EQUAL(configuration.arch, "x86_64");
        ASSERT_STRINGS_ARE_EQUAL(configuration.object_file_type, "elf64");

        ASSERT_EQUAL(parser.free_arguments_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(*parser.free_arguments[0], "filename");
    }

    {
        const String_View arguments[] = {
            string_view("--verbose"),
            string_view("--object-file-type"),
            string_view("elf64"),
            string_view("filename"),
        };
        const Size arguments_count = NUMBER_OF_STATIC_ARRAY_ELEMENTS(arguments);

        reset_parser(&parser);

        configuration = (Test_Configuration){0};

        parse_arguments(&parser, arguments, arguments_count);

        ASSERT_EQUAL(configuration.verbose, true);
        ASSERT_STRINGS_ARE_EQUAL(configuration.arch, "x86_64");
        ASSERT_STRINGS_ARE_EQUAL(configuration.object_file_type, "elf64");

        ASSERT_EQUAL(parser.free_arguments_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(*parser.free_arguments[0], "filename");
    }

    {
        const String_View arguments[] = {
            string_view("--arch"),
            string_view("aarch64"),
            string_view("--object-file-type"),
            string_view("pe64"),
            string_view("filename"),
        };
        const Size arguments_count = NUMBER_OF_STATIC_ARRAY_ELEMENTS(arguments);

        reset_parser(&parser);

        configuration = (Test_Configuration){0};

        parse_arguments(&parser, arguments, arguments_count);

        ASSERT_EQUAL(configuration.verbose, false);
        ASSERT_STRINGS_ARE_EQUAL(configuration.arch, "aarch64");
        ASSERT_STRINGS_ARE_EQUAL(configuration.object_file_type, "pe64");

        ASSERT_EQUAL(parser.free_arguments_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(*parser.free_arguments[0], "filename");
    }

    {
        const String_View arguments[] = {
            string_view("filename"),
        };
        const Size arguments_count = NUMBER_OF_STATIC_ARRAY_ELEMENTS(arguments);

        reset_parser(&parser);

        configuration = (Test_Configuration){0};

        parse_arguments(&parser, arguments, arguments_count);

        ASSERT_EQUAL(configuration.verbose, false);
        ASSERT_STRINGS_ARE_EQUAL(configuration.arch, "x86_64");
        ASSERT_STRINGS_ARE_EQUAL(configuration.object_file_type, "elf64");

        ASSERT_EQUAL(parser.free_arguments_count, 1);
        ASSERT_STRINGS_ARE_EQUAL(*parser.free_arguments[0], "filename");
    }
}

REGISTER_TESTS(
    test_example_compiler_arguments_parsing
)

#include "argument_parser.c"
