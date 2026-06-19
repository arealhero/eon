#pragma once

#include <eon/memory.h>
#include <eon/containers.h>

#include "eon_forward_declarations.h"

#include "eon_ast.h"
#include "eon_cfg.h"
#include "eon_diagnostics.h"
#include "eon_tac.h"

struct Arena_Provider;
internal Arena* acquire_arena_from_provider(struct Arena_Provider* provider,
                                            const String_View arena_name,
                                            const Size number_of_bytes_to_reserve,
                                            const Size number_of_bytes_to_commit);
internal void request_arena_reset(struct Arena_Provider* provider, const Arena* arena);
internal void release_arena_to_provider(struct Arena_Provider* provider, const Arena* arena);

struct Compilation_Context
{
    struct Arena_Provider* arena_provider;

    Arena* scratch_arena;
    Arena* keywords_arena;

    Arena* diagnostic_message_texts_arena;
    Arena* diagnostic_messages_arena;
    Arena* ast_arena;
    Arena* lexical_scopes_arena;
    Arena* symbols_arena;
    Arena* types_arena;
    Arena* parameter_type_ids_arena;

    Arena* tac_functions_arena;
    Arena* tac_function_labels_arena;
    Arena* tac_variables_arena;
    Arena* tac_constants_arena;
    Arena* tac_labels_arena;

    Arena* cfg_blocks_arena;

    Source_File source_file;

    array(Diagnostic_Message, diagnostic_messages);

    Ast ast;

    array(struct Symbol, symbols);
    array(struct Lexical_Scope, lexical_scopes);
    array(struct Type, types);

    Tac tac;

    Cfg cfg;
};
typedef struct Compilation_Context Compilation_Context;

maybe_unused internal void create_compilation_context(Compilation_Context* context,
                                                      struct Arena_Provider* arena_provider,
                                                      const Source_File* source_file);
maybe_unused internal void destroy_compilation_context(Compilation_Context* context);

maybe_unused internal Bool has_compilation_errors(const Compilation_Context* context);
maybe_unused internal inline Bool has_diagnostic_messages(const Compilation_Context* context);
maybe_unused internal void emit_diagnostic_message(Compilation_Context* context, const Diagnostic_Message* message);

// FIXME(vlad): Return 'Scattered_String'?
maybe_unused internal String dump_diagnostic_messages(Arena* messages_arena,
                                                      Compilation_Context* context,
                                                      const Message_Level max_level);

maybe_unused internal Symbol_Id create_symbol(Compilation_Context* context);

maybe_unused internal Symbol_Id find_symbol_id(Compilation_Context* context,
                                               Lexical_Scope_Id this_lexical_scope_id,
                                               const String_View name);

maybe_unused internal inline struct Symbol* get_symbol_for_identifier(Compilation_Context* context, const Ast_Identifier* identifier);
maybe_unused internal inline struct Symbol* get_symbol_by_id(Compilation_Context* context, const Symbol_Id symbol_id);

maybe_unused internal Type_Id create_type(Compilation_Context* context);
maybe_unused internal inline struct Type* get_type_by_id(Compilation_Context* context, const Type_Id type_id);
maybe_unused internal struct Type* get_type_for_identifier(Compilation_Context* context, const Ast_Identifier* identifier);

// FIXME(vlad): Move this to eon_types.
maybe_unused internal Type_Id find_root_type_id(Compilation_Context* context, const Type_Id type_id);

maybe_unused internal inline Bool type_ids_are_equal(Compilation_Context* context, const Type_Id lhs, const Type_Id rhs);
