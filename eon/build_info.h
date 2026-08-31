#pragma once

#if !defined(EON_SLOW_BUILD)
#    define EON_SLOW_BUILD 1
#endif

#if defined(_WIN32)
#    define OS_WINDOWS 1
#    define OS_LINUX   0
#    define OS_MAC     0
#elif defined(__gnu_linux__) || defined(__linux__)
#    define OS_WINDOWS 0
#    define OS_LINUX   1
#    define OS_MAC     0
#elif defined(__APPLE__) && defined(__MACH__)
#    define OS_WINDOWS 0
#    define OS_LINUX   0
#    define OS_MAC     1
#else
#    error Unknown target OS.
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
#    define ARCH_X86    0
#    define ARCH_X86_64 1
#    define ARCH_ARM32  0
#    define ARCH_ARM64  0
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
#    define ARCH_X86    1
#    define ARCH_X86_64 0
#    define ARCH_ARM32  0
#    define ARCH_ARM64  0
#elif defined(__aarch64__) || defined(_M_ARM64)
#    define ARCH_X86    0
#    define ARCH_X86_64 0
#    define ARCH_ARM32  0
#    define ARCH_ARM64  1
#elif defined(__arm__) || defined(_M_ARM)
#    define ARCH_X86    0
#    define ARCH_X86_64 0
#    define ARCH_ARM32  1
#    define ARCH_ARM64  0
#else
#    error Unknown target architecture.
#endif

#if ARCH_X86_64 || ARCH_ARM64
#    define ARCH_32BIT 0
#    define ARCH_64BIT 1
#elif ARCH_X86 || ARCH_ARM32
#    define ARCH_32BIT 1
#    define ARCH_64BIT 0
#endif

#if ARCH_X86 || ARCH_X86_64 || ARCH_ARM32 || ARCH_ARM64
#    define ARCH_LITTLE_ENDIAN 1
#else
#    error Unknown target architecture endianness.
#endif

// NOTE(vlad): Detecting sanitizers.

#if defined(__has_feature)
#    if __has_feature(address_sanitizer)
#        define ASAN_ENABLED 1
#    endif
#elif defined(__SANITIZE_ADDRESS__)
#    define ASAN_ENABLED 1
#else
#    define ASAN_ENABLED 0
#endif

// NOTE(vlad): Detecting the compiler.

#if defined(__clang__)
#    define COMPILER_CLANG 1
#    define COMPILER_GCC 0
#    define COMPILER_MSVC 0
#elif defined(__GNUC__)
#    define COMPILER_CLANG 0
#    define COMPILER_GCC 1
#    define COMPILER_MSVC 0
#elif defined(_MSC_VER)
#    define COMPILER_CLANG 0
#    define COMPILER_GCC 0
#    define COMPILER_MSVC 1
#else
#    error Failed to determine this compiler.
#endif

// NOTE(vlad): Detecting build type.

// FIXME(vlad): Use our own EON_INTERNAL_BUILD definition that is explicitly provided to the compiler during the build.
#if COMPILER_CLANG
#    if defined(__OPTIMIZE__)
#        define RELEASE_BUILD 1
#        define DEBUG_BUILD 0
#    else
#        define RELEASE_BUILD 0
#        define DEBUG_BUILD 1
#    endif
#elif COMPILER_GCC || COMPILER_MSVC
#    if NDEBUG
#        define RELEASE_BUILD 1
#        define DEBUG_BUILD 0
#    else
#        define RELEASE_BUILD 0
#        define DEBUG_BUILD 1
#    endif
#else
#    error This compiler is not supported yet.
#endif

#define PRAGMA(arg) _Pragma(#arg)

// NOTE(vlad): Defining pragmas.
#if COMPILER_CLANG
#    define GCC_PUSH_DIAGNOSTIC()
#    define GCC_IGNORE_WARNING(warning)
#    define GCC_POP_DIAGNOSTIC()

#    define MSVC_PUSH_DIAGNOSTIC()
#    define MSVC_IGNORE_WARNING(warning_number)
#    define MSVC_POP_DIAGNOSTIC()
#elif COMPILER_GCC
#    define GCC_PUSH_DIAGNOSTIC() PRAGMA(GCC diagnostic push)
#    define GCC_IGNORE_WARNING(warning) PRAGMA(GCC diagnostic ignored #warning)
#    define GCC_POP_DIAGNOSTIC() PRAGMA(GCC diagnostic pop)

#    define MSVC_PUSH_DIAGNOSTIC()
#    define MSVC_IGNORE_WARNING(warning_number)
#    define MSVC_POP_DIAGNOSTIC()
#elif COMPILER_MSVC
#    define GCC_PUSH_DIAGNOSTIC()
#    define GCC_IGNORE_WARNING(warning)
#    define GCC_POP_DIAGNOSTIC()

#    define MSVC_PUSH_DIAGNOSTIC() PRAGMA(warning(push))
#    define MSVC_IGNORE_WARNING(warning_number) PRAGMA(warning(disable: warning_number))
#    define MSVC_POP_DIAGNOSTIC() PRAGMA(warning(pop))
#else
#    error Unknown compiler found.
#endif
