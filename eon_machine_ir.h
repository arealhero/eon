#pragma once

#include <eon/containers.h>
#include <eon/memory.h>
#include <eon/types.h>

#include "eon_ast.h"
#include "eon_forward_declarations.h"

struct MIR_Block;

enum
{
    NO_REGISTER = 0,
    SPILLED_TO_STACK = -1,
};

enum Register_Kind
{
    REGISTER_UNDEFINED = 0,
    REGISTER_GPR32,
    REGISTER_GPR64,
    REGISTER_FP32,
    REGISTER_FP64,
};
typedef enum Register_Kind Register_Kind;

struct Virtual_Register
{
    Register_Kind kind;
    Index fixed_physical_register;
    Index assigned_physical_register;

    union
    {
        Index stack_slot_index; // NOTE(vlad): If 'assigned_physical_register' equals to 'SPILLED_TO_STACK'.
    };
};
typedef struct Virtual_Register Virtual_Register;

enum MIR_Operand_Kind
{
    MIR_OPERAND_NONE = 0,
    MIR_OPERAND_VIRTUAL_REGISTER,
    MIR_OPERAND_IMMEDIATE_VALUE,
    MIR_OPERAND_BLOCK,
    MIR_OPERAND_STACK_SLOT,

    // TODO(vlad): Add PHYSICAL_REGISTER, MEMORY, GLOBAL, FLAGS, etc.
};
typedef enum MIR_Operand_Kind MIR_Operand_Kind;

struct MIR_Stack_Slot
{
    Index offset_in_bytes;
    Size size_in_bytes;
};
typedef struct MIR_Stack_Slot MIR_Stack_Slot;

struct MIR_Operand
{
    MIR_Operand_Kind kind;

    union
    {
        Index virtual_register_index;
        u64 immediate_value;

        struct MIR_Block* block;

        Index stack_slot_index;
    };
};
typedef struct MIR_Operand MIR_Operand;

enum MIR_Opcode
{
    MIR_UNDEFINED = 0,

    MIR_NOP,

    // TODO(vlad): Should we use ADD32/ADD64 instead?
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

    MIR_GROW_STACK,
    MIR_SHRINK_STACK,

    MIR_GET_PARAMETER,

    MIR_MOVE,

    MIR_JUMP,
    MIR_JUMP_IF_FALSE,

    MIR_PHI,

    MIR_CALL, MIR_RETURN,
};
typedef enum MIR_Opcode MIR_Opcode;

struct MIR_PHI_Argument
{
    Index virtual_register_index;
    struct MIR_Block* source_block;
};
typedef struct MIR_PHI_Argument MIR_PHI_Argument;

enum
{
    MAX_DEFINITIONS_IN_MIR_INSTRUCTION = 3,
    MAX_USES_IN_MIR_INSTRUCTION = 3,

    MAX_IMPLICIT_DEFINITIONS_IN_MIR_INSTRUCTION = 7,
    MAX_IMPLICIT_USES_IN_MIR_INSTRUCTION = 3,
};

struct Coalescing_Hint
{
    Index first_virtual_register_index;
    Index second_virtual_register_index;
};
typedef struct Coalescing_Hint Coalescing_Hint;

struct MIR_Instruction
{
    struct MIR_Instruction* previous_instruction;
    struct MIR_Instruction* next_instruction;

    MIR_Opcode opcode;

    array(Coalescing_Hint, coalescing_hints);

    MIR_Operand definitions[MAX_DEFINITIONS_IN_MIR_INSTRUCTION];
    Size definitions_count;

    MIR_Operand uses[MAX_USES_IN_MIR_INSTRUCTION];
    Size uses_count;

    MIR_Operand implicit_definitions[MAX_IMPLICIT_DEFINITIONS_IN_MIR_INSTRUCTION];
    Size implicit_definitions_count;

    MIR_Operand implicit_uses[MAX_IMPLICIT_USES_IN_MIR_INSTRUCTION];
    Size implicit_uses_count;

    union
    {
        // NOTE(vlad): For MIR_PHI.
        struct
        {
            MIR_Operand phi_definition;

            array(MIR_PHI_Argument, phi_arguments);
        };

        // NOTE(vlad): For MIR_CALL.
        struct
        {
            Bool has_return_value;
            MIR_Operand return_operand;

            Tac_Function_Label_Id function_label_id; // TODO(vlad): This should no longer belong to TAC alone, move it
                                                     //             somewhere else.

            MIR_Operand* function_arguments;
            Size function_arguments_count;
        };
    };
};
typedef struct MIR_Instruction MIR_Instruction;

struct MIR_Register_Bitset
{
    u64* words;
    Size words_count;
};
typedef struct MIR_Register_Bitset MIR_Register_Bitset;

struct MIR_Parallel_Copy
{
    Index destination_virtual_register_index;
    Index destination_physical_register;

    Index source_virtual_register_index;
    Index source_physical_register;

    Bool move_instruction_was_emitted;
};
typedef struct MIR_Parallel_Copy MIR_Parallel_Copy;

struct MIR_Block
{
    Index sequence_number;

    MIR_Instruction* first_instruction;
    MIR_Instruction* last_instruction;

    struct MIR_Block** predecessors;
    Size predecessors_count;

    struct MIR_Block** successors;
    Size successors_count;

    MIR_Register_Bitset live_in_registers;
    MIR_Register_Bitset live_out_registers;

    array(MIR_Parallel_Copy, parallel_copies);
};
typedef struct MIR_Block MIR_Block;

struct MIR_Blocks_Layout
{
    MIR_Block** blocks;
    Size blocks_count;
};
typedef struct MIR_Blocks_Layout MIR_Blocks_Layout;

struct MIR_Function
{
    Arena* virtual_registers_arena;
    Arena* stack_slots_arena;

    const Ast_Function_Definition* ast_function_definition;
    const struct Tac_Function* tac_function;

    Bool blocks_layout_computed;
    union
    {
        MIR_Block* entry_block;
        MIR_Blocks_Layout blocks_layout;
    };

    array(Virtual_Register, virtual_registers);

    array(MIR_Stack_Slot, stack_slots);
    Index current_stack_offset_in_bytes;
};
typedef struct MIR_Function MIR_Function;

struct MIR
{
    MIR_Function* functions;
    Size functions_count;
};
typedef struct MIR MIR;

maybe_unused internal String_View physical_register_to_string(struct Compilation_Context* context,
                                                              const Index physical_register);

maybe_unused internal void lower_ssa_to_mir(struct Compilation_Context* context);
maybe_unused internal void add_isa_constraints_to_mir(struct Compilation_Context* context);
maybe_unused internal void allocate_registers(struct Compilation_Context* context);
maybe_unused internal void compute_layout_of_mir_blocks(struct Compilation_Context* context);
