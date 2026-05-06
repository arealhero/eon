#pragma once

#include <eon/types.h>
#include <eon/string.h>

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
