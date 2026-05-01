#pragma once

#include <eon/common.h>
#include <eon/types.h>

#include "eon_ast.h"

enum Tac_Operation
{
    TAC_NOP = 0,

    TAC_MOVE,
    TAC_ADD,
    TAC_SUBTRACT,
    TAC_MULTIPLY,
    TAC_DIVIDE,

    TAC_EQUAL,
    TAC_NOT_EQUAL,
    TAC_LESS,
    TAC_LESS_OR_EQUAL,
    TAC_GREATER,
    TAC_GREATER_OR_EQUAL,

    TAC_LABEL,

    TAC_JUMP,
    TAC_JUMP_IF_FALSE,

    TAC_PARAMETER,
    TAC_CALL,
    TAC_RETURN,
};
typedef enum Tac_Operation Tac_Operation;

enum Tac_Operand_Kind
{
    TAC_OPERAND_NONE = 0,
    TAC_OPERAND_TEMP,
    TAC_OPERAND_LABEL,
    TAC_OPERAND_CONSTANT,
};
typedef enum Tac_Operand_Kind Tac_Operand_Kind;

enum Tac_Operand_Type
{
    TAC_TYPE_UNDEFINED = 0,

    TAC_TYPE_BOOL,
    TAC_TYPE_INT32,
};
typedef enum Tac_Operand_Type Tac_Operand_Type;

struct Tac_Temp
{
    Tac_Operand_Type type;
    Index id;
};
typedef struct Tac_Temp Tac_Temp;

struct Tac_Constant
{
    Tac_Operand_Type type;
    union
    {
        s32 int32;
    };
};
typedef struct Tac_Constant Tac_Constant;

struct Tac_Label
{
    Index id;
};
typedef struct Tac_Label Tac_Label;

struct Tac_Operand
{
    Tac_Operand_Kind kind;

    union
    {
        Tac_Temp temp;
        Tac_Constant constant;
        Tac_Label label;
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

struct Tac_Builder
{
    Arena* instructions_arena;

    Tac_Instruction* instructions;
    Size instructions_count;
    Size instructions_capacity;

    Index next_temp_id;
    Index next_constant_id;
    Index next_label_id;
};
typedef struct Tac_Builder Tac_Builder;

internal void create_tac_builder(Tac_Builder* builder, Arena* instructions_arena);

internal Tac_Temp lower_expression_to_tac(Tac_Builder* builder,
                                          const Ast_Expression* expression);
