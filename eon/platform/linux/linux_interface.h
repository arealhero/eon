#pragma once

#include <eon/assert.h>
#include <eon/static_assert.h>
#include <eon/types.h>

// @ref: https://www.chromium.org/chromium-os/developer-library/reference/linux-constants/syscalls/

// FIXME(vlad): Support x86.

#define __S16_TYPE       short int
#define __U16_TYPE       unsigned short int
#define __S32_TYPE       int
#define __U32_TYPE       unsigned int
#define __SLONGWORD_TYPE long int
#define __ULONGWORD_TYPE unsigned long int

#define __SYSCALL_SLONG_TYPE __SLONGWORD_TYPE
#define __SYSCALL_ULONG_TYPE __ULONGWORD_TYPE

typedef __SYSCALL_SLONG_TYPE __syscall_slong_t;

typedef __S32_TYPE clockid_t;
typedef __SYSCALL_SLONG_TYPE off_t;

global_variable Size LINUX_PAGE_SIZE;

internal inline long
linux_syscall0(long n)
{
    long ret;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n)
                      : "rcx", "r11", "memory");
    return ret;
}

internal inline long
linux_syscall1(long n, long a1)
{
    long ret;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n), "D"(a1)
                      : "rcx", "r11", "memory");
    return ret;
}

internal inline long
linux_syscall2(long n, long a1, long a2)
{
    long ret;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n), "D"(a1), "S"(a2)
                      : "rcx", "r11", "memory");
    return ret;
}

internal inline long
linux_syscall3(long n, long a1, long a2, long a3)
{
    long ret;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                      : "rcx", "r11", "memory");
    return ret;
}

internal inline long
linux_syscall4(long n, long a1, long a2, long a3, long a4)
{
    register long r10 __asm__("r10") = a4;

    long ret;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                      : "rcx", "r11", "memory");
    return ret;
}

internal inline long
linux_syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
    register long r10 __asm__("r10") = a4;
    register long r8  __asm__("r8") = a5;

    long ret;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
                      : "rcx", "r11", "memory");
    return ret;
}

internal inline long
linux_syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
    register long r10 __asm__("r10") = a4;
    register long r8  __asm__("r8") = a5;
    register long r9  __asm__("r9") = a6;

    long ret = 0;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
                      : "rcx", "r11", "memory");
    return ret;
}

enum
{
    SYS_read          = 0,
    SYS_write         = 1,
    SYS_open          = 2,
    SYS_close         = 3,
    SYS_fstat         = 5,
    SYS_mmap          = 9,
    SYS_mprotect      = 10,
    SYS_munmap        = 11,
    SYS_exit          = 60,
    SYS_arch_prctl    = 158,
    SYS_clock_gettime = 228,
    SYS_getrandom     = 318,
};

#define AT_NULL   0
#define AT_PAGESZ 6
#define AT_RANDOM 25

#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_RDWR      00000002
#define O_CREAT     00000100
#define O_EXCL      00000200
#define O_NOCTTY    00000400
#define O_TRUNC     00001000
#define O_APPEND    00002000
#define O_NONBLOCK  00004000
#define O_DIRECTORY 00200000
#define O_NOFOLLOW  00400000
#define O_NOATIME   01000000
#define O_CLOEXEC   02000000

#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000
#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100
#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010
#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define S_IRUGO   (S_IRUSR | S_IRGRP | S_IROTH)
#define S_IWUGO   (S_IWUSR | S_IWGRP | S_IWOTH)
#define S_IXUGO   (S_IXUSR | S_IXGRP | S_IXOTH)
#define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)

internal inline int
open(const char* path, const int flags, const u32 mode)
{
    long result = linux_syscall3(SYS_open,
                                 (long)(path),
                                 (long)(flags),
                                 (long)(mode));
    return (int)(result);
}

internal inline int
read(const int fd, void* buffer, const USize count)
{
    long result = linux_syscall3(SYS_read,
                                 (long)(fd),
                                 (long)(buffer),
                                 (long)(count));
    return (int)(result);
}

internal inline int
write(const int fd, const void* buffer, const USize count)
{
    long result = linux_syscall3(SYS_write,
                                 (long)(fd),
                                 (long)(buffer),
                                 (long)(count));
    return (int)(result);
}

internal inline int
close(const int fd)
{
    long result = linux_syscall1(SYS_close, (long)(fd));
    return (int)(result);
}

/* Protections are chosen from these bits, OR'd together.  The
   implementation does not necessarily support PROT_EXEC or PROT_WRITE
   without PROT_READ.  The only guarantees are that no writing will be
   allowed without PROT_WRITE and no access will be allowed for PROT_NONE. */

#define PROT_READ      0x1      /* Page can be read.  */
#define PROT_WRITE     0x2      /* Page can be written.  */
#define PROT_EXEC      0x4      /* Page can be executed.  */
#define PROT_NONE      0x0      /* Page can not be accessed.  */
#define PROT_GROWSDOWN 0x01000000 /* Extend change to start of growsdown vma (mprotect only).  */
#define PROT_GROWSUP   0x02000000 /* Extend change to start of growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED          0x01 /* Share changes.  */
#define MAP_PRIVATE         0x02 /* Changes are private.  */
#define MAP_SHARED_VALIDATE 0x03 /* Share changes and validate extension flags.  */
#define MAP_DROPPABLE       0x08 /* Zero memory under memory pressure.  */
#define MAP_TYPE            0x0f /* Mask for type of mapping.  */

/* Other flags.  */
#define MAP_FIXED     0x10      /* Interpret addr exactly.  */
#define MAP_FILE      0
#define MAP_ANONYMOUS 0x20      /* Don't use a file.  */
#define MAP_ANON      MAP_ANONYMOUS

#define MAP_FAILED ((void*) -1)

internal inline void*
mmap(void* address,
     const USize number_of_bytes,
     const int protection,
     const int flags,
     const int fd,
     const off_t offset)
{
    long result = linux_syscall6(SYS_mmap,
                                 (long)(address),
                                 (long)(number_of_bytes),
                                 protection,
                                 flags,
                                 fd,
                                 offset);
    if ((unsigned long)(result) > (unsigned long)(-4095))
    {
        return MAP_FAILED;
    }

    return (void*)(result);
}

internal inline int
mprotect(void* address, const USize number_of_bytes, const int protection)
{
    long result = linux_syscall3(SYS_mprotect,
                                 (long)(address),
                                 (long)(number_of_bytes),
                                 (long)(protection));
    return (int)(result);
}

internal inline int
munmap(void* address, const USize number_of_bytes)
{
    long result = linux_syscall2(SYS_munmap,
                                 (long)(address),
                                 (long)(number_of_bytes));
    return (int)(result);
}

#define CLOCK_REALTIME           0 /* Identifier for system-wide realtime clock.  */
#define CLOCK_MONOTONIC          1 /* Monotonic system-wide clock.  */
#define CLOCK_PROCESS_CPUTIME_ID 2 /* High-resolution timer from the CPU.  */
#define CLOCK_THREAD_CPUTIME_ID  3 /* Thread-specific CPU-time clock.  */
#define CLOCK_MONOTONIC_RAW      4 /* Monotonic system-wide clock, not adjusted for frequency scaling.  */
#define CLOCK_REALTIME_COARSE    5 /* Identifier for system-wide realtime clock, updated only on ticks.  */
#define CLOCK_MONOTONIC_COARSE   6 /* Monotonic system-wide clock, updated only on ticks.  */
#define CLOCK_BOOTTIME           7 /* Monotonic system-wide clock that includes time spent in suspension.  */
#define CLOCK_REALTIME_ALARM     8 /* Like CLOCK_REALTIME but also wakes suspended system.  */
#define CLOCK_BOOTTIME_ALARM     9 /* Like CLOCK_BOOTTIME but also wakes suspended system.  */
#define CLOCK_TAI                11 /* Like CLOCK_REALTIME but in International Atomic Time.  */

#define TIMER_ABSTIME            1 /* Flag to indicate time is absolute.  */

struct timespec
{
    u64 tv_sec;                   /* Seconds.  */
    u64 tv_nsec;                  /* Nanoseconds.  */
};

internal inline int
clock_gettime(clockid_t clock_id, struct timespec* ts)
{
    long result = linux_syscall2(SYS_clock_gettime, clock_id, (long)(ts));
    return (int)(result);
}

// FIXME(vlad): Rename to 'kstat'.
struct stat
{
    u64 st_dev;
    u64 st_ino;
    u64 st_nlink;
    u32 st_mode;
    u32 st_uid;                 // NOTE(vlad): User ID of the file's owner.
    u32 st_gid;                 // NOTE(vlad): Group ID of the file's group.
    s32 __padding;
    u64 st_rdev;                // NOTE(vlad): Device number, if device.
    s64 st_size;                // NOTE(vlad): Size of file, in bytes.
    s64 st_blksize;             // NOTE(vlad): Optimal block size for I/O.
    s64 st_blocks;              // NOTE(vlad): Number 512-byte blocks allocated.

    s64 st_atime;               // NOTE(vlad): Time of last access.
    u64 st_atime_nsec;          // NOTE(vlad): Nanoseconds of last access.

    s64 st_mtime;               // NOTE(vlad): Time of last modification.
    u64 st_mtime_nsec;          // NOTE(vlad): Nanoseconds of last modification.

    s64 st_ctime;               // NOTE(vlad): Time of last status change.
    u64 st_ctime_nsec;          // NOTE(vlad): Nanoseconds of last status change.

    s64 __unused[3];
};

STATIC_ASSERT(size_of(struct stat) == 144);

internal inline int
fstat(const int fd, struct stat* st)
{
    long result = linux_syscall2(SYS_fstat, fd, (long)(st));
    return (int)(result);
}

#define ARCH_SET_FS 0x1002

// TODO(vlad): According to arch_prctl(2), this syscall is supported only on Linux/x86-64 for 64-bit programs. We need
//             to disable it for 32-bit programs. How would the stack protector work then?
internal inline int
arch_prctl(const int operation, void* address)
{
    long result = linux_syscall2(SYS_arch_prctl, (long)(operation), (long)(address));
    return (int)(result);
}

no_return internal inline void
exit(const int exit_code)
{
    linux_syscall1(SYS_exit, exit_code);
    UNREACHABLE();
}

internal inline Size
getrandom(void* buffer, const Size size, const u32 flags)
{
    long result = linux_syscall3(SYS_getrandom, (long)(buffer), (long)(size), (long)(flags));
    return (Size)(result);
}
