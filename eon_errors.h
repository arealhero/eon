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
    // FIXME(vlad): Add 'error_messages_arena' so that we do not reallocate 'errors' every time.

    Error* errors;
    Size errors_count;
    Size errors_capacity;
};
typedef struct Errors Errors;

// TODO(vlad): Remove 'maybe_unused' here?
maybe_unused internal void errors_create(Errors* errors, Arena* errors_arena);
maybe_unused internal void add_error(Errors* errors, Error* error);
maybe_unused internal void print_error(Arena* scratch, const Error* error);
maybe_unused internal void clear_errors(Errors* errors);
maybe_unused internal void errors_destroy(Errors* errors);

