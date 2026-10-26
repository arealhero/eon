#if !EON_PLATFORM_IO_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/io.h>" instead.
#endif

#include <eon/platform/linux/linux_interface.h>

internal void
write_data_to_stdout(const Byte* data, const Size data_size)
{
    if (data_size == 0)
    {
        return;
    }

    Size written_bytes_count = 0;

    while (written_bytes_count != data_size)
    {
        written_bytes_count = write(STDOUT_FD, data, (USize)data_size);
        if (written_bytes_count == -1)
        {
            // NOTE(vlad): Is there something better we can do here?
            return;
        }
    }
}
