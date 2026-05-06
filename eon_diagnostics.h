#pragma once

#include <eon/types.h>
#include <eon/string.h>

#include "eon_forward_declarations.h"

typedef Index Source_File_Id;

struct Source_File
{
    String_View filename;
    String_View code;
};
typedef struct Source_File Source_File;

struct Source_Location
{
    Size offset_in_bytes;
    Size length_in_bytes; // TODO(vlad): Support encodings other than ASCII.

    Index line;
    Index column;
};
typedef struct Source_Location Source_Location;

struct Error
{
    Source_Location location;
    String_View message;
};
typedef struct Error Error;

maybe_unused internal String format_error_message(Arena* arena, struct Compilation_Context* context, const Error* error);
