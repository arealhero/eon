#pragma once
#define EON_PLATFORM_SUBPROCESS_INCLUDED 1

#include <eon/common.h>
#include <eon/memory.h>

enum Spawn_Process_Status
{
    SPAWN_PROCESS_OK,
    SPAWN_PROCESS_FAILED,
};
typedef enum Spawn_Process_Status Spawn_Process_Status;

struct Spawn_Process_Result
{
    Spawn_Process_Status status;

    u32 exit_code;

    String_View stdout;
    String_View stderr;
};
typedef struct Spawn_Process_Result Spawn_Process_Result;

internal Spawn_Process_Result platform_spawn_subprocess_and_wait_until_done(Arena* arena,
                                                                            const String_View executable,
                                                                            const String_View* arguments,
                                                                            const Size arguments_count);

#if OS_LINUX
#    error This OS is not supported yet.
#elif OS_MAC
#    error This OS is not supported yet.
#elif OS_WINDOWS
#    include "win32/subprocess.c"
#else
#    error This OS is not supported yet.
#endif
