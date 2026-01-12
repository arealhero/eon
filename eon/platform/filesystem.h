#pragma once
#define EON_PLATFORM_FILESYSTEM_INCLUDED 1

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

enum Read_File_Status
{
    // TODO(vlad): Add more specific reasons like 'READ_FILE_NOT_FOUND', 'READ_FILE_NO_PERMISSIONS' and such.
    //             Or maybe we can specify the reason in 'Read_File_Result' in a union.
    READ_FILE_FAILURE = 0,
    READ_FILE_SUCCESS,
};
typedef enum Read_File_Status Read_File_Status;

struct Read_File_Result
{
    Read_File_Status status;
    String content;
};
typedef struct Read_File_Result Read_File_Result;

internal Read_File_Result platform_read_entire_text_file(Arena* arena, const String_View filename);

#if OS_LINUX
#    include "linux_filesystem.c"
#else
#    error This OS is not supported yet.
#endif
