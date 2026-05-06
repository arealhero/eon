#include "eon_compilation_context.h"

#include "eon_lexical_scopes.h"

internal void
create_compilation_context(Compilation_Context* context,
                           const Source_File* source_file)
{
    context->error_messages_arena = create_arena("error-messages", GiB(1), MiB(1));
    context->errors_arena = create_arena("errors", GiB(1), MiB(1));
    context->ast_arena = create_arena("ast", GiB(1), MiB(1));
    context->lexical_scopes_arena = create_arena("lexical-scopes", GiB(1), MiB(1));
    context->symbols_arena = create_arena("symbols", GiB(1), MiB(1));

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
        destroy_arena(scope->symbol_ids_arena);
    }

    destroy_arena(context->error_messages_arena);
    destroy_arena(context->errors_arena);
    destroy_arena(context->ast_arena);
    destroy_arena(context->lexical_scopes_arena);
    destroy_arena(context->symbols_arena);
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

    FAIL("Cannot find the requested symbol");
}

