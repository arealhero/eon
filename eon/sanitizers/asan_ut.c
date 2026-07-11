#define ASAN_SET_DEFAULT_OPTIONS 0

#include <eon/unit_test.h>

#include <eon/build_info.h>
#include <eon/keywords.h>

#if ASAN_ENABLED == 0
#    if COMPILER_MSVC
#        error This test should be compiled with "/fsanitize=address" compile flag.
#    else
#        error This test should be compiled with "-fsanitize=address" compile flag.
#    endif
#endif

#include <eon/memory.h>
#include <eon/sanitizers/asan.h>

// XXX(vlad): Make this variable thread-local after implementing tests parallelization.
//            @tag(tests)
global_variable Bool global_sanitizer_was_triggered = false;

external void
__asan_on_error(void)
{
    global_sanitizer_was_triggered = true;
}

external const char*
__asan_default_options(void)
{
    return
        "halt_on_error=false:"
        "continue_on_error=2:"
        ASAN_DEFAULT_OPTIONS;
}

internal void
test_use_after_free_in_arenas(Test_Context* test_context)
{
    // NOTE(vlad): UAF after clearing the arena.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* freed_variable = allocate(arena, int);
        arena_clear(arena);

        *freed_variable = 10;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }

    // NOTE(vlad): UAF after clearing and reallocating arena's memory.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* freed_variable = allocate(arena, int);
        arena_clear(arena);
        int* another_variable = allocate(arena, int);
        UNUSED(another_variable);

        *freed_variable = 10;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }

    // NOTE(vlad): UAF after shrinking the arena.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* freed_variable = allocate(arena, int);
        arena_pop_to_position(arena, 0);

        *freed_variable = 10;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }

    // NOTE(vlad): UAF after buffer reallocation.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* values = allocate_array(arena, 10, int);

        int* first_value = &values[0];
        *first_value = 10;

        ASSERT_FALSE(global_sanitizer_was_triggered);

        values = reallocate(arena, values, int, 10, 20);

        *first_value = 20;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }
}

internal void
test_buffer_overruns_in_arenas(Test_Context* test_context)
{
    enum { TEST_ARRAY_SIZE = 10 };

    // NOTE(vlad): Overrun: single buffer case.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* values = allocate_array(arena, TEST_ARRAY_SIZE, int);

        values[TEST_ARRAY_SIZE] = 10;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }

    // NOTE(vlad): Overrun: multiple buffers case.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* values = allocate_array(arena, TEST_ARRAY_SIZE, int);
        int* other_values = allocate_array(arena, TEST_ARRAY_SIZE, int);
        UNUSED(other_values);

        values[TEST_ARRAY_SIZE] = 10;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }
}

internal void
test_buffer_underruns_in_arenas(Test_Context* test_context)
{
    enum { TEST_ARRAY_SIZE = 10 };

    // NOTE(vlad): Underrun: single buffer case.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* values = allocate_array(arena, TEST_ARRAY_SIZE, int);

        values[-1] = 10;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }

    // NOTE(vlad): Underrun: multiple buffers case.
    {
        global_sanitizer_was_triggered = false;

        Arena* arena = create_arena("asan-test-arena", MiB(1), MiB(1));

        int* values = allocate_array(arena, TEST_ARRAY_SIZE, int);
        int* other_values = allocate_array(arena, TEST_ARRAY_SIZE, int);
        UNUSED(values);

        other_values[-1] = 10;

        ASSERT_TRUE(global_sanitizer_was_triggered);

        destroy_arena(arena);
    }
}

REGISTER_TESTS(
    test_use_after_free_in_arenas,
    test_buffer_overruns_in_arenas,
    test_buffer_underruns_in_arenas
)
