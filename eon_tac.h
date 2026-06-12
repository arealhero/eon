#pragma once

#include <eon/common.h>
#include <eon/types.h>

#include "eon_ast.h"
#include "eon_forward_declarations.h"

enum Tac_Operation
{
    TAC_NOP = 0,

    TAC_ASSIGN,

    TAC_ADD,
    TAC_SUBTRACT,
    TAC_MULTIPLY,
    TAC_DIVIDE,
    // FIXME(vlad): Add 'TAC_MOD'.

    TAC_EQUAL,
    TAC_NOT_EQUAL,
    TAC_LESS,
    TAC_LESS_OR_EQUAL,
    TAC_GREATER,
    TAC_GREATER_OR_EQUAL,

    // FIXME(vlad): Add logical operations.

    TAC_LABEL,
    TAC_JUMP,
    TAC_JUMP_IF_FALSE,

    TAC_SET_PARAMETER,
    TAC_GET_PARAMETER,

    TAC_CALL,
    TAC_RETURN,
};
typedef enum Tac_Operation Tac_Operation;

enum Tac_Operand_Kind
{
    TAC_OPERAND_NONE = 0,

    TAC_OPERAND_FUNCTION_LABEL,
    TAC_OPERAND_VARIABLE,
    TAC_OPERAND_LABEL,
    TAC_OPERAND_CONSTANT,
    TAC_OPERAND_PARAMETER_INDEX,
};
typedef enum Tac_Operand_Kind Tac_Operand_Kind;

struct Tac_Function_Label
{
    Index id;
    Symbol_Id symbol_id;
};
typedef struct Tac_Function_Label Tac_Function_Label;

struct Tac_Variable
{
    Index id;
    Type_Id type_id;

    Bool is_temporary;
    union
    {
        Symbol_Id symbol_id;
    };
};
typedef struct Tac_Variable Tac_Variable;

struct Tac_Label
{
    Index id;
};
typedef struct Tac_Label Tac_Label;

struct Tac_Constant
{
    Index id;
    Type_Id type_id;

    union
    {
        // TODO(vlad): Support string literals.
        const Ast_Number* ast_number;
    };
};
typedef struct Tac_Constant Tac_Constant;

struct Tac_Parameter_Index
{
    Index index;
};
typedef struct Tac_Parameter_Index Tac_Parameter_Index;

struct Tac_Operand
{
    Tac_Operand_Kind kind;

    union
    {
        Tac_Function_Label function_label;
        Tac_Variable variable;
        Tac_Constant constant;
        Tac_Label label;
        Tac_Parameter_Index parameter_index;
    };
};
typedef struct Tac_Operand Tac_Operand;

struct Tac_Instruction
{
    Tac_Operation operation;
    Tac_Operand destination;
    Tac_Operand first_argument;
    Tac_Operand second_argument;
};
typedef struct Tac_Instruction Tac_Instruction;

struct Tac_Function
{
    Arena* instructions_arena;

    Tac_Function_Label label;

    Tac_Instruction* instructions;
    Size instructions_count;
    Size instructions_capacity;
};
typedef struct Tac_Function Tac_Function;

struct Tac
{
    Tac_Function* functions;
    Size functions_count;
    Size functions_capacity;

    Tac_Function_Label* function_labels;
    Size function_labels_count;
    Size function_labels_capacity;

    Index next_function_label_id;
    Index next_variable_id;
    Index next_constant_id;
    Index next_label_id;
};
typedef struct Tac Tac;

maybe_unused internal void lower_ast_to_tac(struct Compilation_Context* context);
