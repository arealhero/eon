#pragma once

#include "eon_forward_declarations.h"

#include <eon/containers.h>
#include <eon/memory.h>
#include <eon/types.h>

struct MIR_Block;

enum
{
    NO_REGISTER = 0,
};

enum Register_Kind
{
    REGISTER_UNDEFINED = 0,
    REGISTER_GPR32,
    REGISTER_GPR64,
    REGISTER_FP32,
    REGISTER_FP64,
    REGISTER_FLAGS,
};
typedef enum Register_Kind Register_Kind;

struct Virtual_Register
{
    Register_Kind kind;
    Index fixed_physical_register;
};
typedef struct Virtual_Register Virtual_Register;

enum MIR_Operand_Kind
{
    MIR_OPERAND_NONE = 0,
    MIR_OPERAND_VIRTUAL_REGISTER,
    MIR_OPERAND_IMMEDIATE_VALUE,
    MIR_OPERAND_FUNCTION,
    MIR_OPERAND_BLOCK,

    // TODO(vlad): Add PHYSICAL_REGISTER, STACK_SLOT, MEMORY, GLOBLAL, FLAGS, etc.
};
typedef enum MIR_Operand_Kind MIR_Operand_Kind;

struct MIR_Operand
{
    MIR_Operand_Kind kind;

    union
    {
        Virtual_Register* virtual_register;
        u64 immediate_value;

        Tac_Function_Label_Id function_label_id; // TODO(vlad): This should no longer belong to TAC alone, move it
                                                 //             somewhere else.

        struct MIR_Block* block;
    };
};
typedef struct MIR_Operand MIR_Operand;

enum MIR_Opcode
{
    MIR_NOP = 0,

    MIR_ADD,
    MIR_SUBTRACT,
    MIR_MULTIPLY,
    MIR_DIVIDE,
    // TODO(vlad): MIR_AND, MIR_OR, etc.

    MIR_EQUAL,
    MIR_NOT_EQUAL,
    MIR_LESS,
    MIR_LESS_OR_EQUAL,
    MIR_GREATER,
    MIR_GREATER_OR_EQUAL,

    MIR_LOAD32,
    MIR_STORE32,

    MIR_LOAD64,
    MIR_STORE64,

    MIR_GET_ADDRESS,

    MIR_MOVE,

    MIR_JUMP,
    MIR_JUMP_IF_TRUE,
    MIR_JUMP_IF_FALSE,

    MIR_PHI,

    MIR_CALL, MIR_RETURN,
};
typedef enum MIR_Opcode MIR_Opcode;

struct MIR_PHI_Argument
{
    Virtual_Register* virtual_register;
    struct MIR_Block* source_block;
};
typedef struct MIR_PHI_Argument MIR_PHI_Argument;

enum
{
    MAX_DEFINITIONS_IN_MIR_INSTRUCTION = 3,
    MAX_USES_IN_MIR_INSTRUCTION = 3, // NOTE(vlad): This does not include PHI nodes.
};

struct MIR_Instruction
{
    struct MIR_Instruction* previous_instruction;
    struct MIR_Instruction* next_instruction;

    MIR_Opcode opcode;

    union
    {
        struct
        {
            MIR_Operand definitions[MAX_DEFINITIONS_IN_MIR_INSTRUCTION];
            Size definitions_count;

            MIR_Operand uses[MAX_USES_IN_MIR_INSTRUCTION];
            Size uses_count;
        };

        // NOTE(vlad): For MIR_PHI.
        struct
        {
            MIR_PHI_Argument* phi_arguments;
            Size phi_arguments_count;
        };

        // NOTE(vlad): For MIR_CALL.
        struct
        {
            MIR_Operand* function_arguments;
            Size function_arguments_count;
        };
    };
};
typedef struct MIR_Instruction MIR_Instruction;

struct MIR_Block
{
    MIR_Instruction* first_instruction;
    MIR_Instruction* last_instruction;

    struct MIR_Block** predecessors;
    Size predecessors_count;

    struct MIR_Block** successors;
    Size successors_count;
};
typedef struct MIR_Block MIR_Block;

struct MIR_Function
{
    MIR_Block* first_block;
};
typedef struct MIR_Function MIR_Function;

struct MIR
{
    MIR_Function* functions;
    Size functions_count;

    array(Virtual_Register, virtual_registers);
};
typedef struct MIR MIR;

maybe_unused internal void lower_ssa_to_mir(struct Compilation_Context* context);
