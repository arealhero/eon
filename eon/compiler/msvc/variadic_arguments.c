#if !EON_COMPILER_VARIADIC_ARGUMENTS_INCLUDED
#    error Do not use this file directly. Include "<eon/compiler/variadic_arguments.h>" instead.
#endif

// NOTE(vlad): I tried using compiler intrinsics like '__crt_va_start()' and '__va_start()' (not sure that the latter is
//             a real intrinsic, but nevertheless) -- no luck, it either does not work or works with CRT only (e.g. in
//             ASAN builds).
//
//             That said, I found out that we can actually just include the <stdarg.h> header even in freestanding
//             environment.
#include <stdarg.h>
