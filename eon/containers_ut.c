#include <eon/unit_test.h>

#include "containers.h"

internal void
test_stack(Test_Context* test_context)
{
    {
        local_stack(s32, values);

        const s32 first_value = 10;
        const s32 second_value = 20;

        stack_push(test_context->arena, values, s32, first_value);
        ASSERT_EQUAL(*stack_top(values), first_value);

        stack_push(test_context->arena, values, s32, second_value);
        ASSERT_EQUAL(*stack_top(values), second_value);

        stack_pop(values);
        ASSERT_EQUAL(*stack_top(values), first_value);

        stack_pop(values);
        ASSERT_EQUAL(values_count, 0);
    }
}

internal void
test_set(Test_Context* test_context)
{
    local_set(s32, values);

    ASSERT_EQUAL(values_count, 0);

    set_insert(test_context->arena, values, s32, 1);
    ASSERT_EQUAL(values_count, 1);

    set_insert(test_context->arena, values, s32, 1);
    ASSERT_EQUAL(values_count, 1);

    set_insert(test_context->arena, values, s32, 2);
    ASSERT_EQUAL(values_count, 2);

    local_set(s32, copy);
    set_copy(test_context->arena, copy, values, s32);

    set_remove(values, s32, 1);
    ASSERT_EQUAL(values_count, 1);

    set_remove(values, s32, 1);
    ASSERT_EQUAL(values_count, 1);

    set_remove(values, s32, 2);
    ASSERT_EQUAL(values_count, 0);

    ASSERT_EQUAL(copy_count, 2);

    set_remove(copy, s32, 1);
    ASSERT_EQUAL(copy_count, 1);

    set_remove(copy, s32, 2);
    ASSERT_EQUAL(copy_count, 0);
}

REGISTER_TESTS(
    test_stack,
    test_set
)
