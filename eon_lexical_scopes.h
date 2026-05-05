#pragma once

#include <eon/types.h>
#include <eon/string.h>

#include "eon_compilation_context.h"

struct Symbol
{
    String_View name;
    // TODO(vlad): Add Source_Location.

    Type_Id type_id;
    Bool is_mutable;
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

maybe_unused internal Bool create_lexical_scopes(Compilation_Context* context);
