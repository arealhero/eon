#pragma once

#include "eon_parser.h"

#include <eon/memory.h>

struct Interpreter_Variable
{
    Ast_Identifier name;
    Ast_Type* type;

    // FIXME(vlad): Support other types.
    s32 value;
};
typedef struct Interpreter_Variable Interpreter_Variable;

struct Interpreter_Lexical_Scope
{
    Interpreter_Variable* variables;
    Size variables_count;
    Size variables_capacity;
};
typedef struct Interpreter_Lexical_Scope Interpreter_Lexical_Scope;

struct Interpreter
{
    Interpreter_Lexical_Scope lexical_scope;
};
typedef struct Interpreter Interpreter;

enum Interpreter_Run_Status
{
    INTERPRETER_RUN_OK,
    INTERPRETER_RUN_COMPILE_ERROR,
    INTERPRETER_RUN_RUNTIME_ERROR,
};
typedef enum Interpreter_Run_Status Interpreter_Run_Status;

struct Run_Result
{
    Interpreter_Run_Status status;
    union
    {
        String error;
        s32 return_value;
    };
};
typedef struct Run_Result Run_Result;

internal void interpreter_create(Interpreter* interpreter);
internal Run_Result interpreter_run(Arena* runtime_arena,
                                    Arena* result_arena,
                                    Interpreter* interpreter,
                                    const Ast* ast,
                                    const String_View name_of_the_function_to_run);
internal void interpreter_destroy(Interpreter* interpreter);

