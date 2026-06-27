#pragma once

#include <eon/macros.h>

// TODO(vlad): Introduce 'STATIC_ASSERT_C(expression, comment)'?
#define STATIC_ASSERT(expression) _Static_assert(expression, "Static assertion failed")
