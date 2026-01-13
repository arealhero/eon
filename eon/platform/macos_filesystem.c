#if !EON_PLATFORM_FILESYSTEM_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/filesystem.h>" instead.
#endif

#include <fcntl.h> // NOTE(vlad): For 'open'.
#include <sys/stat.h> // NOTE(vlad): For 'fstat'.
#include <unistd.h> // NOTE(vlad0): For 'read' and 'close'.

internal Read_File_Result
platform_read_entire_text_file(Arena* arena, const String_View filename)
{
    Read_File_Result result = {0};

    char* zero_terminated_filename = allocate_uninitialized_array(arena, filename.length + 1, char);
    copy_memory(as_bytes(zero_terminated_filename),
                as_bytes(filename.data),
                filename.length);
    zero_terminated_filename[filename.length] = '\0';

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
