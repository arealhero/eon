#pragma once

#include <eon/assert.h>
#include <eon/build_info.h>
#include <eon/memory.h>

#include <eon/sanitizers/asan.h>

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

#define local_array(Type, name)                 \
    Type* name = NULL;                          \
    Size CONCATENATE(name, _count) = 0;         \
    Size CONCATENATE(name, _capacity) = 0

#define append_array(arena, array, Type, element)                       \
    do                                                                  \
    {                                                                   \
        grow_array_if_needed(arena, array, Type);                       \
        ASAN_UNPOISON_ARRAY_ELEMENT(array, Type, CONCATENATE(array, _count)); \
        (array)[CONCATENATE(array, _count)++] = (element);              \
    }                                                                   \
    while (0)                                                           \

#define remove_last_array_element(array, Type)                          \
    do                                                                  \
    {                                                                   \
        ASSERT(CONCATENATE(array, _count) != 0);                        \
        CONCATENATE(array, _count) -= 1;                                \
        const Index last_element_index = CONCATENATE(array, _count);    \
        (array)[last_element_index] = (Type){0};                        \
        ASAN_POISON_ARRAY_ELEMENT(array, Type, last_element_index);     \
    }                                                                   \
    while (0)

#define reverse_array(array, Type)                      \
    do                                                  \
    {                                                   \
        const Size count = CONCATENATE(array, _count);  \
        for (Index i = 0;                               \
             i < count / 2;                             \
             ++i)                                       \
        {                                               \
            Type* lhs = &(array)[i];                    \
            Type* rhs = &(array)[count - 1 - i];        \
            Type temp = *lhs;                           \
            *lhs = *rhs;                                \
            *rhs = temp;                                \
        }                                               \
    }                                                   \
    while (0)

#define stack(Type, name) array(Type, name)
#define local_stack(Type, name) local_array(Type, name)

#define stack_push(arena, stack, Type, element) append_array(arena, stack, Type, element)
#define stack_top(stack) &(stack)[CONCATENATE(stack, _count) - 1]
#define stack_pop(stack)                        \
    do                                          \
    {                                           \
        ASSERT(CONCATENATE(stack, _count) > 0); \
        CONCATENATE(stack, _count) -= 1;        \
    }                                           \
    while (0)

// TODO(vlad): Speed up this implementation.
#define set(Type, name) array(Type, name)
#define local_set(Type, name) local_array(Type, name)

#define set_clear(set) CONCATENATE(set, _count) = 0

#define set_insert(arena, set, Type, element)           \
    do                                                  \
    {                                                   \
        Bool element_was_found = false;                 \
        for (Index i = 0;                               \
             i < CONCATENATE(set, _count);              \
             ++i)                                       \
        {                                               \
            if ((set)[i] == (element))                  \
            {                                           \
                element_was_found = true;               \
                break;                                  \
            }                                           \
        }                                               \
                                                        \
        if (!element_was_found)                         \
        {                                               \
            append_array(arena, set, Type, element);    \
        }                                               \
    }                                                   \
    while (0)

#define set_remove(set, Type, element)                          \
    do                                                          \
    {                                                           \
        for (Index i = 0;                                       \
             i < CONCATENATE(set, _count);                      \
             ++i)                                               \
        {                                                       \
            if ((set)[i] == (element))                          \
            {                                                   \
                (set)[i] = (set)[CONCATENATE(set, _count) - 1]; \
                remove_last_array_element(set, Type);           \
            }                                                   \
        }                                                       \
    }                                                           \
    while (0)

#define set_move(destination, source)                                   \
    do                                                                  \
    {                                                                   \
        (destination) = (source);                                       \
        CONCATENATE(destination, _count) = CONCATENATE(source, _count); \
        CONCATENATE(destination, _capacity) = CONCATENATE(source, _capacity); \
        set_clear(source);                                              \
    }                                                                   \
    while (0)

#define set_copy(arena, destination, source, Type)                      \
    do                                                                  \
    {                                                                   \
        set_clear(destination);                                         \
        ensure_array_has_enough_capacity(arena,                         \
                                         destination,                   \
                                         Type,                          \
                                         CONCATENATE(source, _count));  \
        for (Index i = 0;                                               \
             i < CONCATENATE(source, _count);                           \
             ++i)                                                       \
        {                                                               \
            append_array(arena, destination, Type, (source)[i]);        \
        }                                                               \
    }                                                                   \
    while (0)

// FIXME(vlad): Speed up this mess of a code.
#define sets_are_equal(lhs, rhs, Type, result)                  \
    do                                                          \
    {                                                           \
        const Size lhs_count = CONCATENATE(lhs, _count);        \
        const Size rhs_count = CONCATENATE(rhs, _count);        \
        if (lhs_count != rhs_count)                             \
        {                                                       \
            *result = false;                                    \
            break;                                              \
        }                                                       \
                                                                \
        *result = true;                                         \
        for (Index lhs_index = 0;                               \
             lhs_index < lhs_count;                             \
             ++lhs_index)                                       \
        {                                                       \
            const Type* lhs_element = &(lhs)[lhs_index];        \
            Bool element_was_found = false;                     \
            for (Index rhs_index = 0;                           \
                 rhs_index < rhs_count;                         \
                 ++rhs_index)                                   \
            {                                                   \
                const Type* rhs_element = &(rhs)[rhs_index];    \
                if (*lhs_element == *rhs_element)               \
                {                                               \
                    element_was_found = true;                   \
                    break;                                      \
                }                                               \
            }                                                   \
                                                                \
            if (!element_was_found)                             \
            {                                                   \
                *result = false;                                \
                break;                                          \
            }                                                   \
        }                                                       \
    }                                                           \
    while (0)
