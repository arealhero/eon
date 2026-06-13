#if !EON_PLATFORM_IO_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/io.h>" instead.
#endif

#include "win32_hacks.h"
#include <windows.h>

internal void
write_data_to_stdout(const Byte* data, const Size data_size)
{
    if (data_size == 0)
    {
        return;
    }

    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD written_bytes_count = 0;
    while (written_bytes_count != data_size)
    {
        const BOOL result = WriteFile(handle, data, (DWORD)data_size, &written_bytes_count, NULL);
        if (result == FALSE)
        {
            // NOTE(vlad): Is there something better we can do here?
            return;
        }
    }
}

#include "win32_restore_hacks.h" // IWYU pragma: export
