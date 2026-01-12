#pragma once

#include <eon/common.h>
#include <eon/string.h>

internal void show_grammar_error(Arena* scratch,
                                 String_View grammar,
                                 const s64 line,
                                 const s64 column,
                                 const Size highlight_length);
