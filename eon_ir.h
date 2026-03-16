#pragma once

#include <eon/types.h>
#include <eon/memory.h>

#include "eon_parser.h"

enum SSA_Type
{
    SSA_TYPE_UNDEFINED = 0,

    // NOTE(vlad): x = y op z OR x = y[i]
    SSA_TYPE_BINARY_OPERATION,

    // NOTE(vlad): x = op y OR x = y& OR x = y*
    SSA_TYPE_UNARY_OPERATION,

    // NOTE(vlad): x = y
    SSA_TYPE_ASSIGNMENT,

    // TODO(vlad): Merge assignments via flags and optional params?

    // NOTE(vlad): x* = y
    SSA_TYPE_DEREFERENCED_ASSIGNMENT,

    // NOTE(vlad): x[i] = y
    SSA_TYPE_INDEXED_ASSIGNMENT,

    // NOTE(vlad): goto LABEL
    SSA_TYPE_JUMP,

    // NOTE(vlad): if x relop y goto LABEL
    SSA_TYPE_CONDITIONAL_JUMP,

    // NOTE(vlad): return OR return x
    SSA_TYPE_RETURN,

    // NOTE(vlad): result = call(arg_1, arg_2, ..., arg_n)
    SSA_TYPE_CALL,
};
typedef enum SSA_Type SSA_Type;

struct SSA_Variable
{
    String_View name;
    Index version;
};
typedef struct SSA_Variable SSA_Variable;

internal inline Bool
is_temporary_variable(const SSA_Variable* variable)
{
    return variable->name.length == 0;
}

enum SSA_Operand_Type
{
    SSA_OPERAND_TYPE_UNDEFINED = 0,

    SSA_OPERAND_TYPE_VARIABLE,
    SSA_OPERAND_TYPE_INT32,
};
typedef enum SSA_Operand_Type SSA_Operand_Type;

struct SSA_Operand
{
    SSA_Operand_Type type;
    union
    {
        SSA_Variable variable;
        s32 s32_value;
    };
};
typedef struct SSA_Operand SSA_Operand;

enum SSA_Binary_Operation_Type
{
    SSA_BINARY_OPERATION_TYPE_UNDEFINED = 0,

    SSA_BINARY_OPERATION_TYPE_ADD,
    SSA_BINARY_OPERATION_TYPE_SUBTRACT,
    SSA_BINARY_OPERATION_TYPE_MULTIPLY,
    SSA_BINARY_OPERATION_TYPE_DIVIDE,
};
typedef enum SSA_Binary_Operation_Type SSA_Binary_Operation_Type;

// FIXME(vlad): Support arrays.
struct SSA_Binary_Operation
{
    SSA_Variable result;
    SSA_Operand lhs;
    SSA_Operand rhs;
    SSA_Binary_Operation_Type operation_type;
};
typedef struct SSA_Binary_Operation SSA_Binary_Operation;

struct SSA_Assignment
{
    SSA_Variable result;
    SSA_Operand operand;
};
typedef struct SSA_Assignment SSA_Assignment;

struct SSA_Return
{
    Bool is_empty;
    SSA_Operand return_value;
};
typedef struct SSA_Return SSA_Return;

struct IR_Instruction
{
    SSA_Type type;

    union
    {
        SSA_Assignment assignment;
        SSA_Binary_Operation binary_operation;
        SSA_Return return_instruction;
    };
};
typedef struct IR_Instruction IR_Instruction;

struct IR_Label
{
    String_View name;
    Index ssa_index;
};
typedef struct IR_Label IR_Label;

struct IR
{
    Arena* instructions_arena;
    Arena* labels_arena;

    IR_Instruction* instructions;
    Size instructions_count;
    Size instructions_capacity;

    // TODO(vlad): Rename to 'function_labels'?
    IR_Label* labels;
    Size labels_count;
    Size labels_capacity;

    Index next_temporary_variable_index;
};
typedef struct IR IR;

internal void ir_create(IR* ir,
                        Arena* instructions_arena,
                        Arena* labels_arena);
internal void ir_destroy(IR* ir);

internal Bool convert_ast_to_ir(const Ast* ast, IR* ir);
