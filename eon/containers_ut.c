#include <eon/unit_test.h>

#include "containers.h"

internal void
test_stack(Test_Context* test_context)
{
    struct Stack
    {
        stack(s32, values);
    };
    typedef struct Stack Stack;

    {
        Stack stack = {0};

        const s32 first_value = 10;
        const s32 second_value = 20;

        stack_push(test_context->arena, stack.values, s32, first_value);
        ASSERT_EQUAL(*stack_top(stack.values), first_value);

        stack_push(test_context->arena, stack.values, s32, second_value);
        ASSERT_EQUAL(*stack_top(stack.values), second_value);

        stack_pop(stack.values);
        ASSERT_EQUAL(*stack_top(stack.values), first_value);

        stack_pop(stack.values);
        ASSERT_EQUAL(stack.values_count, 0);
    }
}

REGISTER_TESTS(
    test_stack
)
