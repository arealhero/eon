#pragma once

#include <eon/types.h>

typedef char* C_String;

struct String_View
{
    const char* data;
    Size length;
};
typedef struct String_View String_View;

struct String
{
    char* data;
    Size length;
};
typedef struct String String;
