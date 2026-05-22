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
struct Type_Id
{
    Index index;
};
typedef struct Type_Id Type_Id;

struct Lexical_Scope;
typedef Index Lexical_Scope_Id;
enum {
    INVALID_LEXICAL_SCOPE_ID = 0,
    GLOBAL_LEXICAL_SCOPE_ID = 1,
};
