#pragma once

#include <eon/types.h>
#include <eon/string.h>

#include "eon_compilation_context.h"

enum Symbol_Kind
{
    SYMBOL_UNDEFINED = 0,

    SYMBOL_UNKNOWN,

    SYMBOL_FUNCTION,
    SYMBOL_VARIABLE,
    SYMBOL_TYPE,
};
typedef enum Symbol_Kind Symbol_Kind;

struct Symbol
{
    Symbol_Kind kind;

    String_View name;
    Source_Location location;

    Type_Id type_id;
    Bool is_mutable;
    Bool is_builtin;
};
typedef struct Symbol Symbol;

struct Lexical_Scope
{
    Arena* symbol_ids_arena;

    Lexical_Scope_Id parent_lexical_scope_id;

    Type_Id required_return_type_index;

    Symbol_Id* symbol_ids;
    Size symbol_ids_count;
    Size symbol_ids_capacity;
};
typedef struct Lexical_Scope Lexical_Scope;

maybe_unused internal void create_lexical_scopes(Compilation_Context* context);
