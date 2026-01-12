#pragma once

#include <eon/common.h>
#include <eon/memory.h>
#include <eon/string.h>

enum IO_Bufferization_Policy
{
    IO_FLUSH_ON_NEWLINE = 0,
    IO_FLUSH_ON_OVERFLOW,
    IO_UNBUFFERED,
};
typedef enum IO_Bufferization_Policy IO_Bufferization_Policy;

struct IO_State
{
    Arena* arena;

    // FIXME(vlad): Change to cyclic buffer over 'mmap'-ed memory.
    Byte* stdout_buffer;
    Size stdout_buffer_index;

    IO_Bufferization_Policy bufferization_policy;
};
internal struct IO_State global_io_state = {0};

internal void init_io_state(const Size initial_arena_size);

internal void print_impl(const String_View message);
internal void print_flush_stdout(void);

#define print(...) print_impl(string_view(format_string(global_io_state.arena __VA_OPT__(,) __VA_ARGS__)))
#define println(...)                            \
    do {                                        \
        print(__VA_ARGS__);                     \
        print_impl(string_view("\n"));          \
    } while (0)
