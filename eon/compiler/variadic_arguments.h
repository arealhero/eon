#pragma once
#define EON_COMPILER_VARIADIC_ARGUMENTS_INCLUDED 1

#include <eon/build_info.h>

#if COMPILER_CLANG
#    include "clang/variadic_arguments.c"
#elif COMPILER_GCC
#    include "gcc/variadic_arguments.c"
#else
#    error This compiler is not supported yet.
#endif
