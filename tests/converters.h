#pragma once

#include <eon/memory.h>
#include <eon/string.h>
#include <eon/keywords.h>
#include <eon_forward_declarations.h>

internal String_View convert_ssa_to_string(Arena* arena, struct Compilation_Context* context);
