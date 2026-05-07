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

enum Message_Level
{
    MESSAGE_LEVEL_ERROR = 0,
    MESSAGE_LEVEL_NOTE,

    MAX_MESSAGE_LEVEL,
};
typedef enum Message_Level Message_Level;

struct Diagnostic_Message
{
    Message_Level level;
    Source_Location location;
    String_View text;
};
typedef struct Diagnostic_Message Diagnostic_Message;

maybe_unused internal inline String_View source_location_to_string(struct Compilation_Context* context,
                                                                   const Source_Location* location);
maybe_unused internal inline void extend_location(Source_Location* location, const Source_Location* to_location);
maybe_unused internal String format_diagnostic_message(Arena* arena,
                                                       struct Compilation_Context* context,
                                                       const Diagnostic_Message* message);
