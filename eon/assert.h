#pragma once

#if !defined(EON_DISABLE_ASSERTS)
#    define EON_DISABLE_ASSERTS 0
#endif

#include <eon/build_info.h>
#include <eon/common.h>
#include <eon/io.h>
#include <eon/keywords.h>
#include <eon/macros.h>

#if COMPILER_MSVC
#    define DEBUGTRAP() __debugbreak()
#elif defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
#    define DEBUGTRAP() __builtin_debugtrap()
#elif COMPILER_GCC || COMPILER_CLANG
#    if ARCH_X86 || ARCH_X86_64
#        define DEBUGTRAP() __asm__ volatile("int3")
#    else
#        error Failed to define 'DEBUGTRAP()'.
#     endif
#else
#    error Failed to define 'DEBUGTRAP()'.
#endif

#if COMPILER_MSVC
#    if ARCH_X86
#        define TRAP() __asm { ud2 }
#    elif ARCH_X86_64
#        include <eon/platform/win32_hacks.h>
#        include <windows.h>
internal inline void
TRAP(void)
{
    static unsigned char code[] = { 0x0F, 0x0B, 0xC3 }; /* ud2; ret */
    DWORD dummy;
    VirtualProtect(code, sizeof(code), PAGE_EXECUTE_READWRITE, &dummy);
    ((void(*)(void))code)();
}
#        include <eon/platform/win32_restore_hacks.h> // IWYU pragma: export
#    else
#        error Failed to define 'TRAP()': unknown architecture.
#    endif
#elif defined(__has_builtin) && __has_builtin(__builtin_trap)
#    define TRAP() __builtin_trap()
#elif COMPILER_GCC || COMPILER_CLANG
#    define TRAP() __asm__ volatile("ud2")
#else
#    error Failed to define 'TRAP()'.
#endif

noreturn internal inline void
INTERNAL_exit(const String_View filename,
              const String_View line_number,
              const String_View message)
{
    print_message_directly_to_stdout(filename);
    print_message_directly_to_stdout(":");
    print_message_directly_to_stdout(line_number);
    print_message_directly_to_stdout(": FAIL: ");
    print_message_directly_to_stdout(message);
    print_message_directly_to_stdout("\n");

    // TODO(vlad): Print backtrace?
    // TODO(vlad): Use trap in debug builds only and 'quick_exit()' in release builds?
    DEBUGTRAP();

    // NOTE(vlad): GCC will emit an implicit call to `abort()` here.
    //             @tag(libc)
    TRAP();
}
#define FAIL(message) INTERNAL_exit(string_view(__FILE__), string_view(AS_STRING_LITERAL(__LINE__)), string_view(message))
#define UNREACHABLE() FAIL("This should be unreachable")

#if EON_DISABLE_ASSERTS
#    define ASSERT(expression) (void)((expression))
#    define SILENT_ASSERT(expression) (void)((expression))
#else
#    define ASSERT(expression) ASSERT_IMPL(expression, __func__, __FILE__, __LINE__)
#    define ASSERT_IMPL(expression, function, file, line)               \
    do                                                                  \
    {                                                                   \
        if (!(expression))                                              \
        {                                                               \
            println("{}:{}: Assertion failed in function {}: {}",       \
                    file, line, function, #expression);                 \
            /* FIXME(vlad): Print backtrace (use libunwind?). */        \
            FAIL("Assertion failed");                                   \
        }                                                               \
    } while (0)

// NOTE(vlad): 'SILENT_ASSERT' is used when the IO state was not initialized.
//             It can safely be removed when we will rewrite '_start' entrypoint
//             and remove libc dependency: our IO state will be initialized before
//             calling 'main()'.
//
//             @tag(libc)
#    define SILENT_ASSERT(expression, message)  \
    do                                          \
    {                                           \
        if (!(expression))                      \
        {                                       \
            FAIL(message);                      \
        }                                       \
    } while (0)
#endif
