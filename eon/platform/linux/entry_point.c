#include <eon/io.h>
#include <eon/static_assert.h>

#include <eon/compiler/builtins.h>

#include <eon/platform/linux/linux_interface.h>

// NOTE(vlad): Compiler can choose to emit a call to 'memset' when we initialize a struct
//             to zero like this: 'Struct var = {0};' (yes, even when '-fno-builtin' flag was provided).
//             Since we do not use libc, we need to provide the 'memset' ourselves.
//
//             Also we cannot mark it as internal *sigh*.
void*
memset(void* ptr,
       int value,
       const USize number_of_bytes)
{
    Byte* memory = as_bytes(ptr);

    for (USize i = 0;
         i < number_of_bytes;
         ++i)
    {
        memory[i] = (Byte)(value);
    }

    return ptr;
}

void*
memcpy(void* destination,
       const void* source,
       USize number_of_bytes)
{
    const Byte* from = (const Byte*)(source);
    Byte* to = as_bytes(destination);

    for (USize i = 0;
         i < number_of_bytes;
         ++i)
    {
        to[i] = from[i];
    }

    return destination;
}

internal int
eon_main(const String_View* arguments, const Size arguments_count);

__asm__(".globl _start                \n"
        "_start:                      \n"
        "    xor %ebp, %ebp           \n" // NOTE(vlad): Marking outermost frame (ABI requirement).
        "    mov %rsp, %rdi           \n"
        "    and $-16, %rsp           \n" // NOTE(vlad): Aligning stack to 16 bytes.
        "    call linux_runtime_start \n"
        "    int3                     \n");

#define AT_NULL   0
#define AT_PAGESZ 6
#define AT_RANDOM 25

internal inline u64
find_value_in_aux_vector(const u64* auxv, const u64 key)
{
    for (Index i = 0;
         auxv[i] != AT_NULL;
         i += 2)
    {
        if (auxv[i] == key)
        {
            return auxv[i + 1];
        }
    }

    return 0;
}

// NOTE(vlad): Setting up the stack protector.

struct Thread_Control_Block
{
    u64 padding[5];
    u64 stack_guard;
};
typedef struct Thread_Control_Block Thread_Control_Block;

global_variable Thread_Control_Block global_thread_control_block;

STATIC_ASSERT(compiler_offset_of(Thread_Control_Block, stack_guard) == 0x28);

without_stack_protector void
init_stack_protector(long* stack_pointer)
{
    const u64 FALLBACK_CANARY_VALUE = 0xdeadbeefcafe0000;

    if (arch_prctl(ARCH_SET_FS, &global_thread_control_block) < 0)
    {
        print_message_directly_to_stdout("Error: arch_ptrctl(ARCH_SET_FS) failed during stack protector initialization");
        exit(1);
    }

    u64 canary_value = 0;

    // NOTE(vlad): Trying to get the random value from the kernel's aux vector.
    {
        const long argc = stack_pointer[0];

        const char** envp = (const char**)(stack_pointer + 1 + argc + 1);
        while (*envp != NULL)
        {
            envp += 1;
        }

        envp += 1;

        canary_value = find_value_in_aux_vector((const u64*)(envp), AT_RANDOM);
    }

    // NOTE(vlad): If that failed, we try to call the 'getrandom' syscall.
    if (canary_value == 0)
    {
        getrandom(as_bytes(&canary_value), size_of(canary_value), 0);
    }

    // NOTE(vlad): glibc convention: LSB = 0.
    canary_value &= ~(u64)(0xff);

    // NOTE(vlad): If canary value is still zero, we fall back to the hardcoded value.
    if (canary_value == 0)
    {
        canary_value = FALLBACK_CANARY_VALUE;
    }

    global_thread_control_block.stack_guard = canary_value;
}

no_return void
__stack_chk_fail(void)
{
    FAIL("*** Stack smashing detected ***\n");
    exit(127);
}

internal int
platform_entry_point(const int argc, const char* argv[])
{
    // NOTE(vlad): Initializing LINUX_PAGE_SIZE.
    {
        const char** envp = argv + argc + 1;
        while (*envp)
        {
            envp += 1;
        }

        unsigned long* auxv = (unsigned long*)(envp + 1);

        for (Index i = 0;
             auxv[i] != AT_NULL;
             i += 2)
        {
            if (auxv[i] == AT_PAGESZ)
            {
                LINUX_PAGE_SIZE = (Size)(auxv[i + 1]);
                break;
            }
        }

        SILENT_ASSERT(LINUX_PAGE_SIZE != 0, "Failed to get PAGE_SIZE");
    }

    init_io_state(GiB(1));

    const Size number_of_bytes_to_allocate = size_of(String_View) * argc;

    String_View* arguments = mmap(NULL,
                                  (USize)number_of_bytes_to_allocate,
                                  PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE|MAP_ANONYMOUS,
                                  -1,
                                  0);
    const Size arguments_count = argc;

    for (Index argument_index = 0;
         argument_index < arguments_count;
         ++argument_index)
    {
        arguments[argument_index] = string_view(argv[argument_index]);
    }
    const int return_code = eon_main(arguments, arguments_count);

    deinit_io_state();

    return return_code;
}

no_return without_stack_protector void
linux_runtime_start(long* stack_pointer)
{
    init_stack_protector(stack_pointer);
    const int argc = (int)(stack_pointer[0]);
    const char** argv = (const char**)(stack_pointer + 1);
    const int return_code = platform_entry_point(argc, argv);
    exit(return_code);
}
