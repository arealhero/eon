#pragma once

#include <eon/containers.h>
#include <eon/memory.h>
#include <eon/types.h>

enum Argument_Option_Kind
{
    OPTION_WITHOUT_ARGUMENT,
    OPTION_WITH_STRING_ARGUMENT,
};
typedef enum Argument_Option_Kind Argument_Option_Kind;

struct Argument_Option
{
    String_View name;

    Argument_Option_Kind kind;

    union
    {
        Bool* flag;
        String_View* string;
    };

    Bool required;

    union
    {
        Bool flag;
        String_View string;
    } default_value;
};
typedef struct Argument_Option Argument_Option;

struct Argument_Parser
{
    Arena* arena;

    array_view(const Argument_Option, options);
    Bool* option_was_set;

    array(const String_View*, free_arguments);
};
typedef struct Argument_Parser Argument_Parser;

internal inline Argument_Parser create_argument_parser(Arena* arena,
                                                       const Argument_Option* options,
                                                       const Size options_count);

internal void parse_arguments(Argument_Parser* parser,
                              const String_View* arguments,
                              const Size arguments_count);
