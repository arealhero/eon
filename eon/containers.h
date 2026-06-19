#pragma once

#include <eon/build_info.h>
#include <eon/memory.h>

#if ASAN_ENABLED
#    define ensure_array_has_enough_capacity(arena, array, Type, requested_size) \
    do                                                                  \
    {                                                                   \
        /* NOTE(vlad): Always reallocate so we could find all use-after-move bugs. */ \
        const Size new_capacity = CONCATENATE(array, _count) + requested_size; \
        array = reallocate(arena,                                       \
                           array,                                       \
                           Type,                                        \
                           CONCATENATE(array, _capacity),               \
                           new_capacity);                               \
        CONCATENATE(array, _capacity) = new_capacity;                   \
    }                                                                   \
    while (0)
#else
#    define ensure_array_has_enough_capacity(arena, array, Type, requested_size) \
    do                                                                  \
    {                                                                   \
        if (CONCATENATE(array, _count) + requested_size > CONCATENATE(array, _capacity)) \
        {                                                               \
            const Size new_capacity = MAX(CONCATENATE(array, _count) + requested_size, \
                                          2 * CONCATENATE(array, _capacity)); \
            array = reallocate(arena,                                   \
                               array,                                   \
                               Type,                                    \
                               CONCATENATE(array, _capacity),           \
                               new_capacity);                           \
            CONCATENATE(array, _capacity) = new_capacity;               \
        }                                                               \
    }                                                                   \
    while (0)
#endif

#define grow_array_if_needed(arena, array, Type) ensure_array_has_enough_capacity(arena, array, Type, 1)

#define array(Type, name)                       \
    Type* name;                                 \
    Size CONCATENATE(name, _count);             \
    Size CONCATENATE(name, _capacity)

#define append_array(arena, array, Type, element)               \
    do                                                          \
    {                                                           \
        grow_array_if_needed(arena, array, Type);               \
        (array)[CONCATENATE(array, _count)++] = (element);      \
    }                                                           \
    while (0)                                                   \

#define stack(Type, name) array(Type, name)
#define stack_push(arena, stack, Type, element) append_array(arena, stack, Type, element)
#define stack_top(stack) &(stack)[CONCATENATE(stack, _count) - 1]
#define stack_pop(stack) CONCATENATE(stack, _count) -= 1;
