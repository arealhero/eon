#pragma once

#include <eon/containers.h>
#include <eon/string.h>
#include <eon/types.h>

#include "eon_compilation_context.h"

enum Symbol_Kind
{
    SYMBOL_UNDEFINED = 0,

    SYMBOL_UNKNOWN,

    SYMBOL_WILDCARD,
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
    Bool binding_is_mutable;
    Bool is_builtin;

    Tac_Instruction_Id tac_instruction_id;
};
typedef struct Symbol Symbol;

struct Lexical_Scope
{
    Arena* symbol_ids_arena;

    Lexical_Scope_Id parent_lexical_scope_id;

    Type_Id required_return_type_index;

    array(Symbol_Id, symbol_ids);
};
typedef struct Lexical_Scope Lexical_Scope;

maybe_unused internal void create_lexical_scopes(Compilation_Context* context);
