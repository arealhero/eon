#pragma once

#include <eon/memory.h>
#include <eon/string_declarations.h>

internal Bool compare_outputs_and_optionally_canonize(Arena* scratch_arena,
                                                      const String_View test_name,
                                                      const String_View filename,
                                                      const String_View output,
                                                      const Bool canonize_output);
