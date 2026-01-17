#include "io.h"

#include <eon/memory.h>
#include <eon/platform/memory.h>
#include <eon/sanitizers/asan.h>

#include <stdlib.h> // NOTE(vlad): For 'atexit'.
#include <unistd.h> // NOTE(vlad): For 'write'.

#define STDOUT_BUFFER_SIZE KiB(4)

#define STDIN_FD  0
#define STDOUT_FD 1
#define STDERR_FD 2

internal void
deinit_io_state(void)
{
    // NOTE(vlad): There's no need to release memory manually: OS will reclaim it automatically.

    print_flush_stdout();

    // NOTE(vlad): Poisoning 'stdout_buffer' just in case. Also note that
    //             one can change bufferization policy to 'IO_UNBUFFERED' and
    //             circumvent this poisoning. I tried to poison 'global_io_state',
    //             it does not work: 'ASAN_POISON_MEMORY_REGION' works without issues,
    //             but e.g. 'global_io_state.bufferization_policy' can still be changed
    //             and read from.
    ASAN_POISON_MEMORY_REGION(global_io_state.stdout_buffer, STDOUT_BUFFER_SIZE);
}

internal void
init_io_state(const Size initial_arena_size)
{
    global_io_state.arena = arena_create("IO", initial_arena_size, KiB(0));
    global_io_state.stdout_buffer = as_bytes(platform_reserve_memory(STDOUT_BUFFER_SIZE));
    global_io_state.stdout_buffer_index = 0;
    global_io_state.bufferization_policy = IO_FLUSH_ON_NEWLINE;

    SILENT_ASSERT(global_io_state.arena != NULL, "Could not create global IO arena");
    SILENT_ASSERT(global_io_state.stdout_buffer != NULL, "Could not initialize global stdout buffer");

    // TODO(vlad): Commit memory only if needed (like we do that in arenas).
    platform_commit_memory(global_io_state.stdout_buffer, STDOUT_BUFFER_SIZE);

    atexit(deinit_io_state);
}

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

internal void
add_data_to_stdout_buffer_and_flush_if_needed(const Byte* data, Size data_size)
{
    Size overflow_bytes_count = 0;
    do
    {
        const Size available_bytes_count_in_buffer = STDOUT_BUFFER_SIZE - global_io_state.stdout_buffer_index;

        overflow_bytes_count = MAX(0, data_size - available_bytes_count_in_buffer);
        const Size bytes_to_write = data_size - overflow_bytes_count;

        copy_memory(global_io_state.stdout_buffer + global_io_state.stdout_buffer_index,
                    data,
                    bytes_to_write);
        global_io_state.stdout_buffer_index += bytes_to_write;

        if (overflow_bytes_count != 0)
        {
            write_data_to_stdout(global_io_state.stdout_buffer, STDOUT_BUFFER_SIZE);
            global_io_state.stdout_buffer_index = 0;

            data += bytes_to_write;
            data_size = overflow_bytes_count;
        }
    }
    while (overflow_bytes_count != 0);
}

internal void
print_impl(const String_View message)
{
    if (global_io_state.bufferization_policy == IO_UNBUFFERED)
    {
        write_data_to_stdout(as_bytes(message.data), message.length);
        return;
    }

    add_data_to_stdout_buffer_and_flush_if_needed(as_bytes(message.data), message.length);

    if (global_io_state.bufferization_policy == IO_FLUSH_ON_NEWLINE)
    {
        Size flush_region_start_index = 0;

        for (Size current_index = 0;
             current_index < global_io_state.stdout_buffer_index;
             ++current_index)
        {
            if (global_io_state.stdout_buffer[current_index] == '\n')
            {
                const Size flush_region_size = current_index - flush_region_start_index + 1;
                write_data_to_stdout(global_io_state.stdout_buffer + flush_region_start_index,
                                     flush_region_size);

                flush_region_start_index = current_index + 1;
            }
        }

        if (flush_region_start_index != 0)
        {
            const Size size_of_region_to_move = global_io_state.stdout_buffer_index - flush_region_start_index;

            move_memory(global_io_state.stdout_buffer,
                        global_io_state.stdout_buffer + flush_region_start_index,
                        size_of_region_to_move);

            global_io_state.stdout_buffer_index = size_of_region_to_move;
        }
    }
}

internal void
print_flush_stdout(void)
{
    write_data_to_stdout(global_io_state.stdout_buffer, global_io_state.stdout_buffer_index);
}

internal inline void
INTERNAL_print_message_directly_to_stdout(const String_View message)
{
    write_data_to_stdout(as_bytes(message.data), message.length);
}
