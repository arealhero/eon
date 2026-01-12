#pragma once

#include <eon/macros.h>

// TODO(vlad): Introduce 'STATIC_ASSERT_C(expression, comment)'
//             with CONCATENATE(comment, __COUNTER__)?
// TODO(vlad): Test if __COUNTER__ is available, otherwise use __LINE__.
#define STATIC_ASSERT(expression)                                       \
    typedef int CONCATENATE(Static_Assert_, __COUNTER__)[(expression) ? 1 : -1]

// // TODO(vlad): Use '_Static_assert' from C11?
// #define STATIC_ASSERT(expression) _Static_assert(expression, "Static assertion failed");
