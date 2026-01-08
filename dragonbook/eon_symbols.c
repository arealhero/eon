#include "eon_symbols.h"

#include <stdlib.h>
#include <string.h> // FIXME(vlad): Remove this header.

internal Symbol*
scope_symbols_find_symbol_in_this_scope(const Scope_Symbols* scope_symbols,
                                        const char* symbol_lexeme)
{
    for (ssize i = 0;
         i < scope_symbols->number_of_symbols;
         ++i)
    {
        if (strcmp(scope_symbols->symbols[i].lexeme, symbol_lexeme) == 0)
        {
            return &scope_symbols->symbols[i];
        }
    }

    return NULL;
}

internal void
scope_symbols_create(Scope_Symbols* scope_symbols,
                     const Scope_Symbols* previous_scope_symbols)
{
    scope_symbols->previous_scope_symbols = previous_scope_symbols;
}

internal void
scope_symbols_add_symbol(Scope_Symbols* scope_symbols,
                         const char* new_symbol_lexeme,
                         const Symbol new_symbol)
{
    if (scope_symbols_find_symbol_in_this_scope(scope_symbols, new_symbol_lexeme) != NULL)
    {
        ASSERT(0 && "This scope already has this symbol.");
    }

    if (scope_symbols->number_of_symbols >= scope_symbols->capacity_of_symbols)
    {
        scope_symbols->capacity_of_symbols = MAX(1, 2 * scope_symbols->capacity_of_symbols);
        scope_symbols->symbols = realloc(scope_symbols->symbols, (usize)scope_symbols->capacity_of_symbols);
    }

    scope_symbols->symbols[scope_symbols->number_of_symbols++] = new_symbol;
}

internal Symbol*
scope_symbols_get_symbol(const Scope_Symbols* scope_symbols, const char* symbol_lexeme)
{
    for (const Scope_Symbols* current_scope_symbols = scope_symbols;
         current_scope_symbols != NULL;
         current_scope_symbols = current_scope_symbols->previous_scope_symbols)
    {
        Symbol* found_symbol = scope_symbols_find_symbol_in_this_scope(scope_symbols, symbol_lexeme);
        if (found_symbol != NULL)
        {
            return found_symbol;
        }
    }

    return NULL;
}

internal void
scope_symbols_destroy(Scope_Symbols* scope_symbols)
{
    if (scope_symbols->symbols)
    {
        free(scope_symbols->symbols);
    }
}
