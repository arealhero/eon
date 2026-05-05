#pragma once

#include <eon/types.h>

enum {
    INVALID_SYMBOL_ID = 0,
};

typedef Index Symbol_Id;

enum {
    INVALID_TYPE_ID = 0,
};

typedef Index Type_Id;

enum {
    INVALID_LEXICAL_SCOPE_ID = 0,
    GLOBAL_LEXICAL_SCOPE_ID = 1,
};

typedef Index Lexical_Scope_Id;

struct Symbol;
struct Lexical_Scope;
