#pragma once

#if !EON_COMPILER_ATTRIBUTES_INCLUDED
#    error Do not use this file directly. Include "<eon/compiler/attributes.h>" instead.
#endif

#define maybe_unused __attribute__((unused))
#define no_return __attribute__((noreturn))
