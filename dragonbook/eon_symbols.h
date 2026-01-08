#pragma once

#include <eon/common.h>

struct Symbol
{
    const char* type; // NOTE(vlad): Unused for now. Also this is probably also a lexeme of the type.
    const char* lexeme;
};
typedef struct Symbol Symbol;

struct Scope_Symbols
{
    // TODO(vlad): Create a Scope_Symbols stack in arena instead of using signly-linked list.
    const struct Scope_Symbols* previous_scope_symbols;

    Symbol* symbols;
    ssize number_of_symbols;
    ssize capacity_of_symbols;
};
typedef struct Scope_Symbols Scope_Symbols;

internal void scope_symbols_create(Scope_Symbols* scope_symbols,
                                   const Scope_Symbols* previous_scope_symbols);

internal void scope_symbols_add_symbol(Scope_Symbols* scope_symbols,
                                       const char* new_symbol_lexeme, // TODO(vlad): Remove this argument and use 'new_symbol.lexeme'?
                                       const Symbol new_symbol);

internal Symbol* scope_symbols_get_symbol(const Scope_Symbols* scope_symbols,
                                          const char* symbol_lexeme);

internal void scope_symbols_destroy(Scope_Symbols* scope_symbols);
