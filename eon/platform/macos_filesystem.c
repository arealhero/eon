#if !EON_PLATFORM_FILESYSTEM_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/filesystem.h>" instead.
#endif

#include <fcntl.h> // NOTE(vlad): For 'open'.
#include <sys/stat.h> // NOTE(vlad): For 'fstat'.
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h> // NOTE(vlad0): For 'read', 'write', 'close', and 'access'.

internal Read_File_Result
platform_read_entire_text_file(Arena* arena, const String_View filename)
{
    Read_File_Result result = {0};

    const char* zero_terminated_filename = to_c_string(arena, filename);
    const int fd = open(zero_terminated_filename, O_RDONLY);
    if (fd == -1)
    {
        result.status = READ_FILE_FAILURE;
        // TODO(vlad): Fetch a reason from errno.
        return result;
    }

    struct stat file_stats = {0};
    fstat(fd, &file_stats);

    // TODO(vlad): Handle symlinks: 'fstat' would return the symlink's size, not the size of the target file.
    const Size content_length = file_stats.st_size;

    if (content_length == 0)
    {
        result.status = READ_FILE_SUCCESS;
        result.content.data = NULL;
        result.content.length = 0;
    }
    else
    {
        // TODO(vlad): Save arena position and restore it if we could not read the file.
        char* content = allocate_uninitialized_array(arena, content_length, char);
        const Size read_result = read(fd, content, (USize)content_length);

        if (read_result == -1)
        {
            result.status = READ_FILE_FAILURE;
            // TODO(vlad): Handle EINTR and maybe EAGAIN?
        }
        else
        {
            // TODO(vlad): Check that 'read_result == content_length'.
            result.status = READ_FILE_SUCCESS;
            result.content.data = content;
            result.content.length = read_result;
        }
    }

    close(fd);

    return result;
}

internal void
platform_write_string_to_file(Arena* scratch_arena,
                              const String_View filename,
                              const String_View content)
{
    const char* zero_terminated_filename = to_c_string(scratch_arena, filename);
    const int fd = open(zero_terminated_filename,
                        O_WRONLY | O_CREAT | O_TRUNC | O_EXLOCK,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1)
    {
        FAIL("Failed to open file");
    }

    const Size written_bytes = write(fd, content.data, (USize)content.length);
    if (written_bytes == -1)
    {
        FAIL("Failed to write to a file");
    }

    ASSERT(written_bytes == content.length);

    close(fd);
}
