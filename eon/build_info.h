#pragma once

#if defined(_WIN32)
    #define OS_WINDOWS 1
    #define OS_LINUX   0
    #define OS_MAC     0
#elif defined(__gnu_linux__) || defined(__linux__)
    #define OS_WINDOWS 0
    #define OS_LINUX   1
    #define OS_MAC     0
#elif defined(__APPLE__) && defined(__MACH__)
    #define OS_WINDOWS 0
    #define OS_LINUX   0
    #define OS_MAC     1
#else
    #error Unknown target OS.
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
    #define ARCH_X86    0
    #define ARCH_X86_64 1
    #define ARCH_ARM32  0
    #define ARCH_ARM64  0
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
    #define ARCH_X86    1
    #define ARCH_X86_64 0
    #define ARCH_ARM32  0
    #define ARCH_ARM64  0
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_X86    0
    #define ARCH_X86_64 0
    #define ARCH_ARM32  0
    #define ARCH_ARM64  1
#elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_X86    0
    #define ARCH_X86_64 0
    #define ARCH_ARM32  1
    #define ARCH_ARM64  0
#else
    #error Unknown target architecture.
#endif

#if defined(ARCH_X86_64) || defined(ARCH_ARM64)
    #define ARCH_32BIT 0
    #define ARCH_64BIT 1
#elif defined(ARCH_X86) || defined(ARCH_ARM32)
    #define ARCH_32BIT 1
    #define ARCH_64BIT 0
#endif

#if ARCH_X86 || ARCH_X86_64 || ARCH_ARM32 || ARCH_ARM64
    #define ARCH_LITTLE_ENDIAN 1
#else
    #error Unknown target architecture's endianness.
#endif
