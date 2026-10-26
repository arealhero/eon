#if !EON_COMPILER_VARIADIC_ARGUMENTS_INCLUDED
#    error Do not use this file directly. Include "<eon/compiler/variadic_arguments.h>" instead.
#endif

typedef __builtin_va_list va_list;

#define va_start(list, last_argument) __builtin_va_start((list), (last_argument))
#define va_copy(destination, source) __builtin_va_copy((destination), (source))
#define va_arg(list, Type) __builtin_va_arg((list), Type)
#define va_end(list) __builtin_va_end(list)
