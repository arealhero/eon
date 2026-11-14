#include "argument_parser.h"

#include <eon/build_info.h>

internal inline Argument_Parser
create_argument_parser(Arena* arena, const Argument_Option* options, const Size options_count)
{
    Argument_Parser parser = {0};
    parser.arena = arena;
    parser.options = options;
    parser.options_count = options_count;
    parser.option_was_set = allocate_array(arena, options_count, Bool);
    return parser;
}

// TODO(vlad): Improve error reporting here.
internal void
parse_arguments(Argument_Parser* parser,
                const String_View* arguments,
                const Size arguments_count)
{
#if EON_SLOW_BUILD
    for (Index option_index = 0;
         option_index < parser->options_count;
         ++option_index)
    {
        const Argument_Option* option = &parser->options[option_index];

        // NOTE(vlad): Validating option name.
        {
            String_View prefix = {0};
            prefix.data = option->name.data;
            prefix.length = MIN(2, option->name.length);

            if (!strings_are_equal(prefix, "--"))
            {
                FAIL("Option name must start with '--'");
            }

            if (option->name.length == 2)
            {
                FAIL("Option name '--' is reserved.");
            }

            const char first_character = option->name.data[2];
            if (!is_ascii_letter(first_character) && !is_ascii_digit(first_character))
            {
                FAIL("Option name must start with a letter or a digit.");
            }
        }

        for (Index another_option_index = 0;
             another_option_index < parser->options_count;
             ++another_option_index)
        {
            if (option_index != another_option_index)
            {
                const Argument_Option* another_option = &parser->options[another_option_index];

                if (strings_are_equal(option->name, another_option->name))
                {
                    FAIL("Option names must be unique.");
                }
            }
        }
    }
#endif

    for (Index argument_index = 0;
         argument_index < arguments_count;
         ++argument_index)
    {
        const String_View argument = arguments[argument_index];

        Bool option_was_handled = false;

        for (Index option_index = 0;
             option_index < parser->options_count;
             ++option_index)
        {
            const Argument_Option* option = &parser->options[option_index];

            if (strings_are_equal(argument, option->name))
            {
                switch (option->kind)
                {
                    case OPTION_WITHOUT_ARGUMENT:
                    {
                        *(option->flag) = true;
                    } break;

                    case OPTION_WITH_STRING_ARGUMENT:
                    {
                        argument_index += 1;

                        if (argument_index < arguments_count)
                        {
                            *(option->string) = arguments[argument_index];
                        }
                        else
                        {
                            FAIL("This option requires a value");
                        }
                    } break;
                }

                parser->option_was_set[option_index] = true;

                option_was_handled = true;
                break;
            }
        }

        if (!option_was_handled)
        {
            append_array(parser->arena,
                         parser->free_arguments,
                         const String_View*,
                         arguments + argument_index);
        }
    }

    for (Index option_index = 0;
         option_index < parser->options_count;
         ++option_index)
    {
        const Argument_Option* option = &parser->options[option_index];

        if (!parser->option_was_set[option_index])
        {
            if (option->required)
            {
                FAIL("Required option was not set.");
            }

            switch (option->kind)
            {
                case OPTION_WITHOUT_ARGUMENT:
                {
                    *(option->flag) = option->default_value.flag;
                } break;

                case OPTION_WITH_STRING_ARGUMENT:
                {
                    *(option->string) = option->default_value.string;
                } break;
            }
        }
    }
}
