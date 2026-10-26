#pragma once

#include <eon/types.h>

#define compiler_offset_of(structure, field) (Size)(__builtin_offsetof(structure, field))
