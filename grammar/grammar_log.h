#pragma once

#include <eon/common.h>
#include <eon/string.h>

internal void show_grammar_error(Arena* scratch,
                                 String_View grammar,
                                 const ssize line,
                                 const ssize column,
                                 const ssize highlight_length);
