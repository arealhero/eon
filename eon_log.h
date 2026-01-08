#pragma once

#include <eon/common.h>
#include <eon/string.h>

internal void log_print_code_line_with_highlighting(const String_View prefix,
                                                    const String_View code_line,
                                                    const ssize highlight_start,
                                                    const ssize highlight_length);
