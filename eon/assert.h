#pragma once

#if !defined(EON_DISABLE_ASSERTS)
    #define EON_DISABLE_ASSERTS 0
#endif

#include <eon/common.h>
#include <eon/io.h>
#include <eon/keywords.h>
#include <eon/macros.h>

#include <stdlib.h> // NOTE(vlad): for 'abort()'.

noreturn internal inline void
FAIL(void)
{
    // TODO(vlad): Print backtrace?
    // TODO(vlad): Use trap in debug builds only and 'quick_exit()' in release builds?
    __builtin_debugtrap();

    // NOTE(vlad): This line is needed to get rid of this compile error:
    //             "function declared 'noreturn' should not return".
    abort();
}

#if EON_DISABLE_ASSERTS
#  define ASSERT(expression) (void)((expression))
#  define SILENT_ASSERT(expression) (void)((expression))
#else
#  define ASSERT(expression) ASSERT_IMPL(expression, __func__, __FILE__, __LINE__)
#  define ASSERT_IMPL(expression, function, file, line)                 \
    do                                                                  \
    {                                                                   \
        if (!(expression))                                              \
        {                                                               \
            println("{}:{}: Assertion failed in function {}: {}",       \
                    file, line, function, #expression);                 \
            /* FIXME(vlad): Print backtrace (use libunwind?). */        \
            FAIL();                                                     \
        }                                                               \
    } while (0)

// NOTE(vlad): 'SILENT_ASSERT' is used when the IO state was not initialized.
//             It can safely be removed when we will rewrite '_start' entrypoint
//             and remove libc dependency: our IO state will be initialized before
//             calling 'main()'.
//
//             TODO(vlad): Support 'TAG(something)' in 'fixme.el'.
//             @libc
#  define SILENT_ASSERT(expression)             \
    do                                          \
    {                                           \
        if (!(expression))                      \
        {                                       \
            FAIL();                             \
        }                                       \
    } while (0)
#endif
