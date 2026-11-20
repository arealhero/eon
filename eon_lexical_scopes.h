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

    SYMBOL_GLOBAL_FUNCTION,
    SYMBOL_FOREIGN_FUNCTION,
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

    union
    {
        struct
        {
            struct Tac_Instruction* defined_at_tac_instruction;
        };

        // NOTE(vlad): For 'SYMBOL_GLOBAL_FUNCTION'.
        struct
        {
            Tac_Function_Label_Id tac_function_label_id;
        };

        // NOTE(vlad): For 'SYMBOL_FOREIGN_FUNCTION'.
        struct
        {
            const Ast_String_Literal* library_name;

            // TODO(vlad): Move these to the 'pe.c'.
            Bool symbol_was_emitted;
            Index symbol_table_index;
        };
    };
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
