#pragma once

#include <eon/common.h>

struct Regex
{
};
typedef struct Regex Regex;

internal bool32 regex_build(const char* regex_string, Regex* regex);
internal bool32 regex_match(const Regex* regex, const char* input);
internal void regex_destroy(Regex* regex);
