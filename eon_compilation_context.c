#include "eon_compilation_context.h"

#include "eon_lexical_scopes.h"

internal void
create_compilation_context(Compilation_Context* context,
                           struct Arena_Provider* arena_provider,
                           const Source_File* source_file)
{
    context->arena_provider = arena_provider;

    context->error_messages_arena = acquire_arena_from_provider(arena_provider, string_view("error-messages"), GiB(1), MiB(1));
    context->errors_arena = acquire_arena_from_provider(arena_provider, string_view("errors"), GiB(1), MiB(1));
    context->ast_arena = acquire_arena_from_provider(arena_provider, string_view("ast"), GiB(1), MiB(1));
    context->lexical_scopes_arena = acquire_arena_from_provider(arena_provider, string_view("lexical-scopes"), GiB(1), MiB(1));
    context->symbols_arena = acquire_arena_from_provider(arena_provider, string_view("symbols"), GiB(1), MiB(1));

    context->source_file = *source_file;
}

internal void
destroy_compilation_context(Compilation_Context* context)
{
    for (Index scope_index = 0;
         scope_index < context->lexical_scopes_count;
         ++scope_index)
    {
        Lexical_Scope* scope = &context->lexical_scopes[scope_index];
        release_arena_to_provider(context->arena_provider, scope->symbol_ids_arena);
    }

    release_arena_to_provider(context->arena_provider, context->error_messages_arena);
    release_arena_to_provider(context->arena_provider, context->errors_arena);
    release_arena_to_provider(context->arena_provider, context->ast_arena);
    release_arena_to_provider(context->arena_provider, context->lexical_scopes_arena);
    release_arena_to_provider(context->arena_provider, context->symbols_arena);
}

internal void
emit_error(Compilation_Context* context,
           const Source_Location location,
           const String_View message)
{
    grow_array_if_needed(context->errors_arena, context->errors, Error);

    Error* error = &context->errors[context->errors_count++];
    error->location = location;
    error->message = message;
}

internal Symbol_Id
create_symbol(Compilation_Context* context)
{
    grow_array_if_needed(context->symbols_arena, context->symbols, Symbol);
    return context->symbols_count++;
}

internal Symbol_Id
find_symbol_id(Compilation_Context* context,
               Lexical_Scope_Id this_lexical_scope_id,
               const String_View name)
{
    while (this_lexical_scope_id != INVALID_LEXICAL_SCOPE_ID)
    {
        Lexical_Scope* scope = &context->lexical_scopes[this_lexical_scope_id];

        for (Index symbol_id_index = 0;
             symbol_id_index < scope->symbol_ids_count;
             ++symbol_id_index)
        {
            const Symbol_Id current_symbol_id = scope->symbol_ids[symbol_id_index];

            Symbol* current_symbol = &context->symbols[current_symbol_id];
            if (strings_are_equal(current_symbol->name, name))
            {
                return current_symbol_id;
            }
        }

        this_lexical_scope_id = scope->parent_lexical_scope_id;
    }

    return INVALID_SYMBOL_ID;
}

internal Symbol*
get_symbol_by_id(Compilation_Context* context, const Symbol_Id symbol_id)
{
    ASSERT(symbol_id != INVALID_SYMBOL_ID && symbol_id < context->symbols_count);
    return &context->symbols[symbol_id];
}

