#pragma once

#include <eon/keywords.h>
#include <eon/memory.h>
#include <eon/string.h>
#include <eon/types.h>

struct Error
{
    String_View filename;
    Index line;
    Index column;
    Size highlight_length;

    String_View code;
    String_View message;
};
typedef struct Error Error;

struct Errors
{
    Arena* errors_arena;

    Error* errors;
    Size errors_count;
    Size errors_capacity;
};
typedef struct Errors Errors;

internal void add_error(Errors* errors, Error* error);
internal void print_error(Arena* scratch, const Error* error);
internal void clear_errors(Errors* errors);

