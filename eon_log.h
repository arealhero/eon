#pragma once

#include <eon/common.h>
#include <eon/string.h>

internal void log_print_code_line_with_highlighting(const String_View prefix,
                                                    const String_View code_line,
                                                    const Size highlight_start,
                                                    const Size highlight_length);
