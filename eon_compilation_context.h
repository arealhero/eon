#pragma once

#include <eon/memory.h>

#include "eon_forward_declarations.h"

#include "eon_ast.h"
#include "eon_diagnostics.h"

struct Error
{
    Source_Location location;
    String_View message;
};
typedef struct Error Error;

struct Arena_Provider;
internal Arena* acquire_arena_from_provider(struct Arena_Provider* provider,
                                            const String_View arena_name,
                                            const Size number_of_bytes_to_reserve,
                                            const Size number_of_bytes_to_commit);
internal void release_arena_to_provider(struct Arena_Provider* provider, const Arena* arena);

struct Compilation_Context
{
    struct Arena_Provider* arena_provider;

    Arena* error_messages_arena;
    Arena* errors_arena;
    Arena* ast_arena;
    Arena* lexical_scopes_arena;
    Arena* symbols_arena;

    Source_File source_file;

    Error* errors;
    Size errors_count;
    Size errors_capacity;

    Ast ast;

    struct Symbol* symbols;
    Size symbols_count;
    Size symbols_capacity;

    struct Lexical_Scope* lexical_scopes;
    Size lexical_scopes_count;
    Size lexical_scopes_capacity;
};
typedef struct Compilation_Context Compilation_Context;

maybe_unused internal void create_compilation_context(Compilation_Context* context,
                                                      struct Arena_Provider* arena_provider,
                                                      const Source_File* source_file);
maybe_unused internal void destroy_compilation_context(Compilation_Context* context);

maybe_unused internal void emit_error(Compilation_Context* context,
                                      const Source_Location location,
                                      const String_View message);

maybe_unused internal Symbol_Id find_symbol_id(Compilation_Context* context,
                                               Lexical_Scope_Id this_lexical_scope_id,
                                               const String_View name);
