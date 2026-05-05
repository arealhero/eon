#pragma once

#include <eon/memory.h>

#include "eon_forward_declarations.h"

#include "eon_ast.h"
#include "eon_errors.h"

// struct Source_File
// {
//     Arena* contents_arena;

//     String_View filename;
//     String_View code;
// };
// typedef struct Source_File Source_File;

// struct Source_Location
// {
//     Source_File* source_file;

//     Index byte_offset;

//     Index line;
//     Index column;
//     Size length_in_bytes; // TODO(vlad): Support encodings other than ASCII.
// };
// typedef struct Source_Location Source_File_Region;

struct Compilation_Context
{
    Arena* ast_arena;
    Arena* lexical_scopes_arena;
    Arena* symbols_arena;

    // Source_File source_file;
    Errors errors;

    Ast ast;

    struct Symbol* symbols;
    Size symbols_count;
    Size symbols_capacity;

    struct Lexical_Scope* lexical_scopes;
    Size lexical_scopes_count;
    Size lexical_scopes_capacity;
};
typedef struct Compilation_Context Compilation_Context;

internal void create_compilation_context(Compilation_Context* context);
internal void destroy_compilation_context(Compilation_Context* context);

