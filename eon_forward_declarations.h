#pragma once

#include <eon/types.h>

struct Compilation_Context;

struct Symbol;
typedef Index Symbol_Id;
enum {
    UNDEFINED_SYMBOL_ID = 0,
    INVALID_SYMBOL_ID = 1,
};

struct Type;
typedef Index Type_Id;
enum { INVALID_TYPE_ID = 0 };

struct Lexical_Scope;
typedef Index Lexical_Scope_Id;
enum {
    INVALID_LEXICAL_SCOPE_ID = 0,
    GLOBAL_LEXICAL_SCOPE_ID = 1,
};
