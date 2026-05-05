#include "eon_compilation_context.h"

#include "eon_lexical_scopes.h"

internal void
create_compilation_context(Compilation_Context* context)
{
    context->ast_arena = create_arena("ast", GiB(1), MiB(1));
    context->lexical_scopes_arena = create_arena("lexical-scopes", GiB(1), MiB(1));
    context->symbols_arena = create_arena("symbols", GiB(1), MiB(1));
    // Arena* errors_arena = create_arena("errors", GiB(1), MiB(1));
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

    destroy_arena(context->ast_arena);
    destroy_arena(context->lexical_scopes_arena);
    destroy_arena(context->symbols_arena);
}
