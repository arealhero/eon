#pragma once
#define EON_COMPILER_ATTRIBUTES_INCLUDED 1

#include <eon/build_info.h>

#if COMPILER_CLANG
#    include "clang/attributes.h"
#elif COMPILER_GCC
#    include "gcc/attributes.h"
#elif COMPILER_MSVC
#    include "msvc/attributes.h"
#else
#    error This compiler is not supported yet.
#endif

