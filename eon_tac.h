#pragma once

#include <eon/common.h>
#include <eon/types.h>

#include "eon_ast.h"
#include "eon_forward_declarations.h"

#define DEFINE_TAC_ID_FOR(name)                                         \
    struct CONCATENATE(name, _Id)                                       \
    {                                                                   \
        Index index;                                                    \
    };                                                                  \
    typedef struct CONCATENATE(name, _Id) CONCATENATE(name, _Id)

enum Tac_Operation
{
    TAC_NOP = 0,

    TAC_ASSIGN,
    TAC_GET_ADDRESS,      // NOTE(vlad): destination = first_argument&
    TAC_LOAD_BY_ADDRESS,  // NOTE(vlad): destination = first_argument*
    TAC_STORE_BY_ADDRESS, // NOTE(vlad): destination* = first_argument

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
    TAC_JUMP_IF_TRUE,
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
    Symbol_Id symbol_id;
};
typedef struct Tac_Function_Label Tac_Function_Label;
// NOTE(vlad): "Tac_Function_Label_Id" is defined inside 'eon_forward_declarations.h'.

struct Tac_Variable
{
    Type_Id type_id;

    Bool is_temporary;
    union
    {
        Symbol_Id symbol_id;
    };
};
typedef struct Tac_Variable Tac_Variable;
DEFINE_TAC_ID_FOR(Tac_Variable);

struct Tac_Constant
{
    Type_Id type_id;

    union
    {
        // TODO(vlad): Support string literals.
        const Ast_Number* ast_number;
    };
};
typedef struct Tac_Constant Tac_Constant;
DEFINE_TAC_ID_FOR(Tac_Constant);

struct Tac_Label
{
    Tac_Instruction_Id instruction_id;
};
typedef struct Tac_Label Tac_Label;
DEFINE_TAC_ID_FOR(Tac_Label);

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
        Tac_Function_Label_Id function_label_id;
        Tac_Variable_Id variable_id;
        Tac_Constant_Id constant_id;
        Tac_Label_Id label_id;
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

    Tac_Function_Label_Id label_id;

    array(Tac_Instruction, instructions);
};
typedef struct Tac_Function Tac_Function;

struct Tac
{
    array(Tac_Function, functions);

    array(Tac_Function_Label, function_labels);
    array(Tac_Variable, variables);
    array(Tac_Constant, constants);
    array(Tac_Label, labels);
};
typedef struct Tac Tac;

maybe_unused internal void lower_ast_to_tac(struct Compilation_Context* context);

maybe_unused internal inline Tac_Function* get_tac_function_by_label(Tac* tac, const Tac_Function_Label_Id label_id);
maybe_unused internal inline Tac_Function_Label* get_tac_function_label_by_id(Tac* tac, const Tac_Function_Label_Id id);
maybe_unused internal inline Tac_Variable* get_tac_variable_by_id(Tac* tac, const Tac_Variable_Id id);
maybe_unused internal inline Tac_Constant* get_tac_constant_by_id(Tac* tac, const Tac_Constant_Id id);
maybe_unused internal inline Tac_Label* get_tac_label_by_id(Tac* tac, const Tac_Label_Id id);

#undef DEFINE_TAC_ID_FOR
