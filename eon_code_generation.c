#include "eon_code_generation.h"

#include "eon_compilation_context.h"
#include "eon_machine_ir.h"

#if ARCH_LITTLE_ENDIAN
struct x86_64_REX
{
    u8 b        : 1; // NOTE(vlad): Extend the destination register.
    u8 x        : 1; // NOTE(vlad): Extend the addressing index.
    u8 r        : 1; // NOTE(vlad): Extend the source register.
    u8 w        : 1; // NOTE(vlad): Select 64-bit operands.
    u8 __prefix : 4;
};
typedef struct x86_64_REX x86_64_REX;
STATIC_ASSERT(size_of(x86_64_REX) == 1);
#endif

enum x86_Condition_Code
{
    X86_CC_OVERFLOW                  = 0,
    X86_CC_NOT_OVERFLOW              = 1,
    X86_CC_UNSIGNED_LESS             = 2,
    X86_CC_UNSIGNED_GREATER_OR_EQUAL = 3,
    X86_CC_EQUAL                     = 4,
    X86_CC_NOT_EQUAL                 = 5,
    X86_CC_UNSIGNED_LESS_OR_EQUAL    = 6,
    X86_CC_UNSIGNED_GREATER          = 7,
    X86_CC_NEGATIVE                  = 8,
    X86_CC_NOT_NEGATIVE              = 9,
    X86_CC_PARITY                    = 10,
    X86_CC_NOT_PARITY                = 11,
    X86_CC_SIGNED_LESS               = 12,
    X86_CC_SIGNED_GREATER_OR_EQUAL   = 13,
    X86_CC_SIGNED_LESS_OR_EQUAL      = 14,
    X86_CC_SIGNED_GREATER            = 15,
};
typedef enum x86_Condition_Code x86_Condition_Code;

static const u8 COMPARISON_KIND_TO_X86_CC[] =
{
    [COMPARISON_EQUAL]                     = X86_CC_EQUAL,
    [COMPARISON_NOT_EQUAL]                 = X86_CC_NOT_EQUAL,

    [COMPARISON_UNSIGNED_LESS]             = X86_CC_UNSIGNED_LESS,
    [COMPARISON_UNSIGNED_GREATER_OR_EQUAL] = X86_CC_UNSIGNED_GREATER_OR_EQUAL,
    [COMPARISON_UNSIGNED_GREATER]          = X86_CC_UNSIGNED_GREATER,
    [COMPARISON_UNSIGNED_LESS_OR_EQUAL]    = X86_CC_UNSIGNED_LESS_OR_EQUAL,

    [COMPARISON_SIGNED_LESS]               = X86_CC_SIGNED_LESS,
    [COMPARISON_SIGNED_GREATER_OR_EQUAL]   = X86_CC_SIGNED_GREATER_OR_EQUAL,
    [COMPARISON_SIGNED_GREATER]            = X86_CC_SIGNED_GREATER,
    [COMPARISON_SIGNED_LESS_OR_EQUAL]      = X86_CC_SIGNED_LESS_OR_EQUAL,
};

// FIXME(vlad): Delete this function -- our "physical" registers' numbers must equal to the physical ones.
internal inline Byte
get_x86_64_register_number(const Index physical_register)
{
    switch (physical_register)
    {
        case X86_64_RAX: return 0;
        case X86_64_RCX: return 1;
        case X86_64_RDX: return 2;
        case X86_64_RBX: return 3;
        // case X86_64_RSP: return 4;
        // case X86_64_RBP: return 5;
        case X86_64_RSI: return 6;
        case X86_64_RDI: return 7;
        case X86_64_R8:  return 8;
        case X86_64_R9:  return 9;
        case X86_64_R10: return 10;
        case X86_64_R11: return 11;
        case X86_64_R12: return 12;
        case X86_64_R13: return 13;
        case X86_64_R14: return 14;
        case X86_64_R15: return 15;

        default:
        {
            UNREACHABLE();
        } break;
    }
}

internal inline void
emit_byte(Arena* arena,
          Buffer* buffer,
          const Byte byte)
{
    append_byte_to_buffer(arena, buffer, byte);
}

internal inline void
emit_x86_64_rex_byte(Arena* arena,
                     Buffer* buffer,
                     x86_64_REX rex)
{
    rex.__prefix = 0x4;
    emit_byte(arena, buffer, *(Byte*)(&rex));
}

internal inline void
emit_imm32(Arena* arena,
           Buffer* buffer,
           const u32 value)
{
    for (u64 offset = 0;
         offset < 4;
         ++offset)
    {
        const Byte byte = (Byte)(value >> (offset * 8));
        emit_byte(arena, buffer, byte);
    }
}

internal inline void
emit_imm64(Arena* arena,
           Buffer* buffer,
           const u64 value)
{
    for (u64 offset = 0;
         offset < 8;
         ++offset)
    {
        const Byte byte = (Byte)(value >> (offset * 8));
        emit_byte(arena, buffer, byte);
    }
}

internal void
x86_64_emit_add_or_subtract(Arena* binary_code_arena,
                            Buffer* binary_code_buffer,
                            const MIR_Function* function,
                            const MIR_Instruction* instruction)
{
    ASSERT(instruction->definitions_count == 1);
    ASSERT(instruction->uses_count == 2);

    ASSERT(instruction->implicit_definitions_count == 0);
    ASSERT(instruction->implicit_uses_count == 0);

    const MIR_Operand* destination = &instruction->definitions[0];
    const MIR_Operand* lhs = &instruction->uses[0];
    const MIR_Operand* rhs = &instruction->uses[1];

    ASSERT(destination->kind == MIR_OPERAND_VIRTUAL_REGISTER);
    const Virtual_Register* destination_virtual_register
        = &function->virtual_registers[destination->virtual_register_index];

    ASSERT(lhs->kind == MIR_OPERAND_VIRTUAL_REGISTER);
    const Virtual_Register* lhs_virtual_register
        = &function->virtual_registers[lhs->virtual_register_index];

    const Index destination_register = destination_virtual_register->assigned_physical_register;
    const Index lhs_register = lhs_virtual_register->assigned_physical_register;

    ASSERT(destination_register != NO_REGISTER);
    ASSERT(lhs_register != NO_REGISTER);

    ASSERT(destination_register == lhs_register);

    const Index destination_register_number = get_x86_64_register_number(destination_register);

    switch (rhs->kind)
    {
        case MIR_OPERAND_NONE:
        case MIR_OPERAND_PHYSICAL_REGISTER:
        case MIR_OPERAND_BLOCK:
        case MIR_OPERAND_STACK_SLOT:
        case MIR_OPERAND_POINTER:
        {
            UNREACHABLE();
        } break;

        case MIR_OPERAND_VIRTUAL_REGISTER:
        {
            ASSERT(lhs->kind == MIR_OPERAND_VIRTUAL_REGISTER);
            const Virtual_Register* rhs_virtual_register
                = &function->virtual_registers[rhs->virtual_register_index];

            const Index rhs_register = rhs_virtual_register->assigned_physical_register;
            ASSERT(rhs_register != NO_REGISTER);
            ASSERT(destination_register != rhs_register);

            const Index rhs_register_number = get_x86_64_register_number(rhs_register);

            x86_64_REX rex = {0};
            rex.w = true;

            if (destination_register >= X86_64_R8)
            {
                rex.r = true;
            }

            if (rhs_register >= X86_64_R8)
            {
                rex.b = true;
            }

            const Byte operation_opcode = (instruction->opcode == MIR_ADD) ? 0x03 : 0x2B;

            emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
            emit_byte(binary_code_arena, binary_code_buffer, operation_opcode);

            Byte modrm = 0xC0;
            modrm |= (destination_register_number & 0x07) << 3;
            modrm |= (rhs_register_number & 0x07);
            emit_byte(binary_code_arena, binary_code_buffer, modrm);
        } break;

        case MIR_OPERAND_IMMEDIATE_VALUE:
        {
            x86_64_REX rex = {0};
            rex.w = true;

            if (destination_register >= X86_64_R8)
            {
                rex.b = true;
            }

            emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);

            const u64 value = rhs->immediate_value;
            const Byte alu_operation = (instruction->opcode == MIR_ADD) ? 0 : 5;

            if (x86_fits_imm8(value))
            {
                emit_byte(binary_code_arena, binary_code_buffer, 0x83);

                Byte modrm = 0xC0;
                modrm |= alu_operation << 3;
                modrm |= (destination_register_number & 0x07);

                emit_byte(binary_code_arena, binary_code_buffer, modrm);
                emit_byte(binary_code_arena, binary_code_buffer, (Byte)(value));
            }
            else
            {
                ASSERT(x86_fits_imm32(value));

                emit_byte(binary_code_arena, binary_code_buffer, 0x81);

                Byte modrm = 0xC0;
                modrm |= alu_operation << 3;
                modrm |= (destination_register_number & 0x07);

                emit_byte(binary_code_arena, binary_code_buffer, modrm);
                emit_imm32(binary_code_arena, binary_code_buffer, (u32)(value));
            }
        } break;
    }
}

internal void
x86_64_emit_compare(Arena* binary_code_arena,
                    Buffer* binary_code_buffer,
                    const MIR_Function* function,
                    const MIR_Instruction* instruction)
{
    ASSERT(instruction->definitions_count == 1);
    ASSERT(instruction->uses_count == 2);

    ASSERT(instruction->implicit_definitions_count == 0);
    ASSERT(instruction->implicit_uses_count == 0);

    const MIR_Operand* destination_operand = &instruction->definitions[0];
    ASSERT(destination_operand->kind == MIR_OPERAND_VIRTUAL_REGISTER);

    const Virtual_Register* destination_virtual_register
        = &function->virtual_registers[destination_operand->virtual_register_index];

    const Index destination_register = destination_virtual_register->assigned_physical_register;
    ASSERT(destination_register != NO_REGISTER);
    const Byte destination_register_number = get_x86_64_register_number(destination_register);

    const MIR_Operand* first_argument_operand = &instruction->uses[0];
    const MIR_Operand* second_argument_operand = &instruction->uses[1];

    ASSERT(first_argument_operand->kind == MIR_OPERAND_VIRTUAL_REGISTER);
    if (second_argument_operand->kind != MIR_OPERAND_IMMEDIATE_VALUE)
    {
        FAIL("[MIR] Register-register comparisons are not supported yet.");
    }

    const Virtual_Register* first_argument_virtual_register
        = &function->virtual_registers[first_argument_operand->virtual_register_index];

    const Index first_argument_register = first_argument_virtual_register->assigned_physical_register;
    ASSERT(first_argument_register != NO_REGISTER);
    const Byte first_argument_register_number = get_x86_64_register_number(first_argument_register);

    const u64 value_to_compare_with = second_argument_operand->immediate_value;

    if (destination_register == first_argument_register)
    {
        // NOTE(vlad): Emitting cmp instruction.
        {
            x86_64_REX rex = {0};
            rex.w = true;

            if (first_argument_register >= X86_64_R8)
            {
                rex.b = true;
            }

            if (x86_fits_imm8(value_to_compare_with))
            {
                emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
                emit_byte(binary_code_arena, binary_code_buffer, 0x83);

                Byte modrm = 0xC0;
                modrm |= 7 << 3; // NOTE(vlad): Opcode extension /7 => CMP.
                modrm |= (first_argument_register_number & 0x07);
                emit_byte(binary_code_arena, binary_code_buffer, modrm);
                emit_byte(binary_code_arena, binary_code_buffer, (Byte)(value_to_compare_with));
            }
            else if (x86_fits_imm32(value_to_compare_with))
            {
                emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
                emit_byte(binary_code_arena, binary_code_buffer, 0x81);

                Byte modrm = 0xC0;
                modrm |= 7 << 3; // NOTE(vlad): Opcode extension /7 => CMP.
                modrm |= (first_argument_register_number & 0x07);
                emit_byte(binary_code_arena, binary_code_buffer, modrm);
                emit_imm32(binary_code_arena, binary_code_buffer, (u32)(value_to_compare_with));
            }
            else
            {
                FAIL("[MIR] Comparisons with 64-bit values are not supported yet.");
            }
        }

        // NOTE(vlad): Emitting setcc instruction.
        {
            // FIXME(vlad): Replace this '4' with an RSP number.
            if (destination_register_number >= 4)
            {
                x86_64_REX rex = {0};
                rex.b = true;

                emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
            }

            emit_byte(binary_code_arena, binary_code_buffer, 0x0F);

            const x86_Condition_Code condition_code = COMPARISON_KIND_TO_X86_CC[instruction->comparison_kind];
            const Byte comparison_code_byte = (0x90 + (Byte)condition_code);
            emit_byte(binary_code_arena, binary_code_buffer, comparison_code_byte);

            Byte modrm = 0xC0;
            modrm |= (destination_register_number & 0x07);
            emit_byte(binary_code_arena, binary_code_buffer, modrm);
        }

        // NOTE(vlad): Emitting movzx instruction.
        {
            const Bool must_emit_the_rex_byte = ((first_argument_register >= X86_64_R8)
                                                 // FIXME(vlad): Replace this '4' with an RSP number.
                                                 || (destination_register_number >= 4));

            if (must_emit_the_rex_byte)
            {
                x86_64_REX rex = {0};
                if (first_argument_register >= X86_64_R8)
                {
                    rex.r = true;
                }

                // FIXME(vlad): Replace this '4' with an RSP number.
                if (destination_register_number >= 4)
                {
                    rex.b = true;
                }

                ASSERT(rex.r != false || rex.b != false);

                emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
            }

            emit_byte(binary_code_arena, binary_code_buffer, 0x0F);
            emit_byte(binary_code_arena, binary_code_buffer, 0xB6);

            Byte modrm = 0xC0;
            modrm |= (destination_register_number & 0x07) << 3;
            modrm |= (first_argument_register_number & 0x07);
            emit_byte(binary_code_arena, binary_code_buffer, modrm);
        }

        return;
    }

    // NOTE(vlad): Clearing the destination register.
    {
        x86_64_REX rex = {0};
        rex.w = true;

        if (destination_register >= X86_64_R8)
        {
            rex.b = true;
        }

        emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);

        emit_byte(binary_code_arena, binary_code_buffer, 0x31); // NOTE(vlad): XOR r/m64, r64.

        Byte modrm = 0xC0;
        modrm |= (destination_register_number & 0x07) << 3;
        modrm |= (destination_register_number & 0x07);
        emit_byte(binary_code_arena, binary_code_buffer, modrm);
    }

    // NOTE(vlad): Emitting the cmp instruction.
    {
        x86_64_REX rex = {0};
        rex.w = true;

        if (first_argument_register >= X86_64_R8)
        {
            rex.b = true;
        }

        if (x86_fits_imm8(value_to_compare_with))
        {
            emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
            emit_byte(binary_code_arena, binary_code_buffer, 0x83);

            Byte modrm = 0xC0;
            modrm |= 7 << 3; // NOTE(vlad): Opcode extension /7 => CMP.
            modrm |= (first_argument_register_number & 0x07);
            emit_byte(binary_code_arena, binary_code_buffer, modrm);
            emit_byte(binary_code_arena, binary_code_buffer, (Byte)(value_to_compare_with));
        }
        else if (x86_fits_imm32(value_to_compare_with))
        {
            emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
            emit_byte(binary_code_arena, binary_code_buffer, 0x81);

            Byte modrm = 0xC0;
            modrm |= 7 << 3; // NOTE(vlad): Opcode extension /7 => CMP.
            modrm |= (first_argument_register_number & 0x07);
            emit_byte(binary_code_arena, binary_code_buffer, modrm);
            emit_imm32(binary_code_arena, binary_code_buffer, (u32)(value_to_compare_with));
        }
        else
        {
            FAIL("[MIR] Comparisons with 64-bit values are not supported yet.");
        }
    }

    // NOTE(vlad): Emitting the setCC instruction.
    {
        // FIXME(vlad): Replace this '4' with an RSP number.
        if (destination_register_number >= 4)
        {
            x86_64_REX rex = {0};
            rex.b = true;

            emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
        }

        emit_byte(binary_code_arena, binary_code_buffer, 0x0F);

        const x86_Condition_Code condition_code = COMPARISON_KIND_TO_X86_CC[instruction->comparison_kind];
        const Byte comparison_code_byte = (0x90 + (Byte)condition_code);
        emit_byte(binary_code_arena, binary_code_buffer, comparison_code_byte);

        Byte modrm = 0xC0;
        modrm |= (destination_register_number & 0x07);
        emit_byte(binary_code_arena, binary_code_buffer, modrm);
    }
}

internal void
x86_64_emit_push_or_pop_register(Arena* binary_code_arena,
                                 Buffer* binary_code_buffer,
                                 const MIR_Function* function,
                                 const MIR_Instruction* instruction)
{
    UNUSED(function);

    ASSERT(instruction->definitions_count == 0);
    ASSERT(instruction->uses_count == 1);

    const MIR_Operand* register_operand = &instruction->uses[0];
    ASSERT(register_operand->kind == MIR_OPERAND_PHYSICAL_REGISTER);

    const Index physical_register = register_operand->physical_register;

    if (physical_register >= X86_64_R8)
    {
        x86_64_REX rex = {0};
        rex.b = true;

        emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
    }

    const Byte opcode = (instruction->opcode == MIR_PUSH_REGISTER_ON_STACK) ? 0x50 : 0x58;

    const Byte physical_register_number = get_x86_64_register_number(physical_register);
    emit_byte(binary_code_arena, binary_code_buffer, opcode + (physical_register_number & 0x07));
}

internal void
x86_64_emit_grow_or_shrink_stack(Arena* binary_code_arena,
                                 Buffer* binary_code_buffer,
                                 const MIR_Function* function,
                                 const MIR_Instruction* instruction)
{
    UNUSED(function);

    ASSERT(instruction->definitions_count == 0);
    ASSERT(instruction->uses_count == 1);

    ASSERT(instruction->implicit_definitions_count == 0);
    ASSERT(instruction->implicit_uses_count == 0);

    const MIR_Operand* amount_operand = &instruction->uses[0];
    ASSERT(amount_operand->kind == MIR_OPERAND_IMMEDIATE_VALUE);

    const u64 amount = amount_operand->immediate_value;

    x86_64_REX rex = {0};
    rex.w = true;
    emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);

    // NOTE(vlad): /5 => SUB (grow), /0 => ADD (shrink).
    const Byte opcode_extension = (instruction->opcode == MIR_GROW_STACK) ? 5 : 0;

    Byte modrm = 0xC0;
    modrm |= opcode_extension << 3;
    modrm |= 4; // TODO(vlad): Change to the RSP register number.

    if (x86_fits_imm8(amount))
    {
        emit_byte(binary_code_arena, binary_code_buffer, 0x83);
        emit_byte(binary_code_arena, binary_code_buffer, modrm);
        emit_byte(binary_code_arena, binary_code_buffer, (Byte)(amount));
    }
    else
    {
        ASSERT(x86_fits_imm32(amount));

        emit_byte(binary_code_arena, binary_code_buffer, 0x81);
        emit_byte(binary_code_arena, binary_code_buffer, modrm);
        emit_imm32(binary_code_arena, binary_code_buffer, (u32)(amount));
    }
}

internal void
x86_64_emit_move(Arena* binary_code_arena,
                 Buffer* binary_code_buffer,
                 const MIR_Function* function,
                 const MIR_Instruction* instruction)
{
    ASSERT(instruction->definitions_count == 1);
    ASSERT(instruction->uses_count == 1);

    ASSERT(instruction->implicit_definitions_count == 0);
    ASSERT(instruction->implicit_uses_count == 0);

    const MIR_Operand* source_operand = &instruction->uses[0];
    const MIR_Operand* destination_operand = &instruction->definitions[0];

    ASSERT(destination_operand->kind == MIR_OPERAND_VIRTUAL_REGISTER);

    const Virtual_Register* destination_virtual_register
        = &function->virtual_registers[destination_operand->virtual_register_index];

    const Index destination_register = destination_virtual_register->assigned_physical_register;
    ASSERT(destination_register != NO_REGISTER);

    switch (source_operand->kind)
    {
        case MIR_OPERAND_NONE:
        case MIR_OPERAND_PHYSICAL_REGISTER:
        case MIR_OPERAND_BLOCK:
        case MIR_OPERAND_STACK_SLOT:
        case MIR_OPERAND_POINTER:
        {
            UNREACHABLE();
        } break;

        case MIR_OPERAND_VIRTUAL_REGISTER:
        {
            const Virtual_Register* source_virtual_register
                = &function->virtual_registers[source_operand->virtual_register_index];

            const Index source_register = source_virtual_register->assigned_physical_register;
            ASSERT(source_register != NO_REGISTER);

            x86_64_REX rex = {0};

            // TODO(vlad): Drop the rex byte if registers are 32-bit wide.
            rex.w = true;

            if (source_register >= X86_64_R8)
            {
                rex.r = true;
            }

            if (destination_register >= X86_64_R8)
            {
                rex.b = true;
            }

            emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
            emit_byte(binary_code_arena, binary_code_buffer, 0x89);

            const Index source_register_number = get_x86_64_register_number(source_register);
            const Index destination_register_number = get_x86_64_register_number(destination_register);

            Byte modrm = 0xC0;
            modrm |= (source_register_number & 0x07) << 3;
            modrm |= (destination_register_number & 0x07);

            emit_byte(binary_code_arena, binary_code_buffer, modrm);
        } break;

        case MIR_OPERAND_IMMEDIATE_VALUE:
        {
            x86_64_REX rex = {0};

            // TODO(vlad): Implement short-constant optimization using the sign-extended 32-bit immediate
            //             value if possible.

            // TODO(vlad): Drop the rex byte if the value fits into the 32-bit non-extended register.
            rex.w = true;

            if (destination_register >= X86_64_R8)
            {
                rex.b = true;
            }

            const u64 value = source_operand->immediate_value;
            const Byte register_number = get_x86_64_register_number(destination_register);

            emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
            emit_byte(binary_code_arena, binary_code_buffer, 0xB8 + (register_number & 0x7));
            emit_imm64(binary_code_arena, binary_code_buffer, value);
        } break;
    }
}

internal void
x86_64_emit_jump(Arena* binary_code_arena,
                 Buffer* binary_code_buffer,
                 const MIR_Function* function,
                 const MIR_Instruction* instruction)
{
    UNUSED(function);

    ASSERT(instruction->definitions_count == 0);
    ASSERT(instruction->uses_count == 1);

    ASSERT(instruction->implicit_definitions_count == 0);
    ASSERT(instruction->implicit_uses_count == 0);

    const MIR_Operand* destination_operand = &instruction->uses[0];
    ASSERT(destination_operand->kind == MIR_OPERAND_BLOCK);

    const Size JMP_INSTRUCTION_SIZE = 5;

    const MIR_Block* destination_block = destination_operand->block;

    // NOTE(vlad): These offsets can be equal to zero during the prepass.
    const Size destination_offset = destination_block->offset_from_the_function_start;
    const Size current_offset = binary_code_buffer->bytes_count;

    // TODO(vlad): Implement the near-jump optimization when the relative offset fits into the imm8.
    // NOTE(vlad): We can do the arithmetic in u32 because it does not matter bitwise.
    const u32 relative_offset = (u32)(destination_offset - (current_offset + JMP_INSTRUCTION_SIZE));

    emit_byte(binary_code_arena, binary_code_buffer, 0xE9);
    emit_imm32(binary_code_arena, binary_code_buffer, relative_offset);
}

internal void
x86_64_emit_jump_if_false(Arena* binary_code_arena,
                          Buffer* binary_code_buffer,
                          const MIR_Function* function,
                          const MIR_Instruction* instruction)
{
    ASSERT(instruction->definitions_count == 0);
    ASSERT(instruction->uses_count == 2);

    ASSERT(instruction->implicit_definitions_count == 0);
    ASSERT(instruction->implicit_uses_count == 0);

    const MIR_Operand* condition_operand = &instruction->uses[0];
    const MIR_Operand* destination_operand = &instruction->uses[1];

    ASSERT(condition_operand->kind == MIR_OPERAND_VIRTUAL_REGISTER);
    ASSERT(destination_operand->kind == MIR_OPERAND_BLOCK);

    // NOTE(vlad): Emitting test instruction.
    {
        const Virtual_Register* condition_virtual_register
            = &function->virtual_registers[condition_operand->virtual_register_index];

        const Index condition_register = condition_virtual_register->assigned_physical_register;
        ASSERT(condition_register != NO_REGISTER);

        x86_64_REX rex = {0};
        if (condition_register >= X86_64_R8)
        {
            rex.r = true;
            rex.b = true;
        }

        emit_x86_64_rex_byte(binary_code_arena, binary_code_buffer, rex);
        emit_byte(binary_code_arena, binary_code_buffer, 0x85);

        const Byte condition_register_number = get_x86_64_register_number(condition_register);

        Byte modrm = 0xC0;
        modrm |= (condition_register_number & 0x07) << 3;
        modrm |= (condition_register_number & 0x07);

        emit_byte(binary_code_arena, binary_code_buffer, modrm);
    }

    // NOTE(vlad): Emitting Jcc instruction.
    {
        const Size JCC_INSTRUCTION_SIZE = 6;

        const MIR_Block* destination_block = destination_operand->block;

        // NOTE(vlad): These offsets can be equal to zero during the prepass.
        const Size destination_offset = destination_block->offset_from_the_function_start;
        const Size current_offset = binary_code_buffer->bytes_count;

        // TODO(vlad): Implement the near-jump optimization when the relative offset fits into the imm8.
        // NOTE(vlad): We can do the arithmetic in u32 because it does not matter bitwise.
        const u32 relative_offset = (u32)(destination_offset - (current_offset + JCC_INSTRUCTION_SIZE));

        const x86_Condition_Code condition_code = COMPARISON_KIND_TO_X86_CC[COMPARISON_EQUAL];

        emit_byte(binary_code_arena, binary_code_buffer, 0x0F);
        emit_byte(binary_code_arena, binary_code_buffer, 0x80 + (Byte)(condition_code));
        emit_imm32(binary_code_arena, binary_code_buffer, relative_offset);
    }
}

internal Bool
x86_64_emit_call(const Compilation_Context* context,
                 Arena* binary_code_arena,
                 Buffer* binary_code_buffer,
                 const MIR_Function* function,
                 const MIR_Instruction* instruction,
                 MIR_Relocation* out_relocation)
{
    ASSERT(instruction->definitions_count == 0);
    ASSERT(instruction->uses_count == 0);

    const Size current_offset = binary_code_buffer->bytes_count;
    const Size call_site_offset = function->offset_of_code_in_the_text_section + current_offset;

    if (instruction->is_a_foreign_function)
    {
        MIR_Relocation relocation = {0};
        relocation.offset_of_displacement_in_the_text_section = call_site_offset + 1; // NOTE(vlad): Skipping the call opcode.
        relocation.foreign_function_symbol_id = instruction->foreign_function_symbol_id;

        *out_relocation = relocation;

        emit_byte(binary_code_arena, binary_code_buffer, 0xE8);
        emit_imm32(binary_code_arena, binary_code_buffer, 0);

        return true;
    }
    else
    {
        const Tac_Function_Label_Id callee_function_label_id = instruction->function_label_id;

        // NOTE(vlad): TAC function indices start with 1.
        const MIR_Function* callee_function = &context->mir.functions[callee_function_label_id.index - 1];

        const Size CALL_INSTRUCTION_SIZE = 5;

        // NOTE(vlad): We can do the arithmetic in u32 because it does not matter bitwise.
        const u32 callee_relative_offset = (u32)(callee_function->offset_of_code_in_the_text_section
                                                 - (call_site_offset + CALL_INSTRUCTION_SIZE));

        emit_byte(binary_code_arena, binary_code_buffer, 0xE8);
        emit_imm32(binary_code_arena, binary_code_buffer, callee_relative_offset);

        return false;
    }
}

internal void
x86_64_emit_return(Arena* binary_code_arena,
                   Buffer* binary_code_buffer,
                   const MIR_Function* function,
                   const MIR_Instruction* instruction)
{
    UNUSED(function);

    ASSERT(instruction->definitions_count == 0);
    ASSERT(instruction->uses_count == 0);

    ASSERT(instruction->implicit_definitions_count == 0);
    // NOTE(vlad): RETURN instruction can have an implicit use: the return value register.

    emit_byte(binary_code_arena, binary_code_buffer, 0xC3);
}

internal void
emit_binary_code(Compilation_Context* context)
{
    MIR* mir = &context->mir;

    ASSERT(context->target_architecture == TARGET_ARCH_X86_64);

    // NOTE(vlad): Calculating each function's size and the offset in the .text section.
    {
        // NOTE(vlad): We will reuse the emitting code to calculate instructions' size in bytes.
        Arena* scratch_arena = context->scratch_arena;
        Buffer scratch_buffer = {0};

        Size current_text_section_offset = 0;

        for (Index function_index = 0;
             function_index < mir->functions_count;
             ++function_index)
        {
            MIR_Function* function = &mir->functions[function_index];
            function->offset_of_code_in_the_text_section = current_text_section_offset;

            ASSERT(function->blocks_layout_computed);

            const MIR_Blocks_Layout* layout = &function->blocks_layout;

            // NOTE(vlad): Computing blocks' offsets.
            {
                Size current_offset_of_block_in_function = 0;

                for (Index block_index = 0;
                     block_index < layout->blocks_count;
                     ++block_index)
                {
                    MIR_Block* block = layout->blocks[block_index];
                    block->offset_from_the_function_start = current_offset_of_block_in_function;

                    for (MIR_Instruction* instruction = block->first_instruction;
                         instruction != NULL;
                         instruction = instruction->next_instruction)
                    {
                        switch (instruction->opcode)
                        {
                            case MIR_UNDEFINED:
                            case MIR_NOP:
                            case MIR_PHI:
                            case MIR_GET_PARAMETER:
                            {
                                UNREACHABLE();
                            } break;

                            case MIR_ADD:
                            case MIR_SUBTRACT:
                            {
                                x86_64_emit_add_or_subtract(scratch_arena, &scratch_buffer, function, instruction);
                            } break;

                            case MIR_MULTIPLY:
                            {
                                FAIL("[MIR] MULTIPLY instruction is not supported yet.");
                            } break;

                            case MIR_DIVIDE:
                            {
                                FAIL("[MIR] DIVIDE instruction is not supported yet.");
                            } break;

                            case MIR_COMPARE:
                            {
                                x86_64_emit_compare(scratch_arena, &scratch_buffer, function, instruction);
                            } break;

                            case MIR_CAST:
                            {
                                FAIL("[MIR] CAST instruction is not supported yet.");
                            } break;

                            case MIR_LOAD:
                            {
                                FAIL("[MIR] LOAD instruction is not supported yet.");
                            } break;

                            case MIR_STORE:
                            {
                                FAIL("[MIR] STORE instruction is not supported yet.");
                            } break;

                            case MIR_GET_ADDRESS:
                            {
                                FAIL("[MIR] GET_ADDRESS instruction is not supported yet.");
                            } break;

                            case MIR_PUSH_REGISTER_ON_STACK:
                            case MIR_POP_REGISTER_FROM_STACK:
                            {
                                x86_64_emit_push_or_pop_register(scratch_arena,
                                                                 &scratch_buffer,
                                                                 function,
                                                                 instruction);
                            } break;

                            case MIR_GROW_STACK:
                            case MIR_SHRINK_STACK:
                            {
                                x86_64_emit_grow_or_shrink_stack(scratch_arena,
                                                                 &scratch_buffer,
                                                                 function,
                                                                 instruction);
                            } break;

                            case MIR_MOVE:
                            {
                                x86_64_emit_move(scratch_arena, &scratch_buffer, function, instruction);
                            } break;

                            case MIR_JUMP:
                            {
                                x86_64_emit_jump(scratch_arena, &scratch_buffer, function, instruction);
                            } break;

                            case MIR_JUMP_IF_FALSE:
                            {
                                x86_64_emit_jump_if_false(scratch_arena, &scratch_buffer, function, instruction);
                            } break;

                            case MIR_CALL:
                            {
                                MIR_Relocation relocation = {0};

                                x86_64_emit_call(context,
                                                 scratch_arena,
                                                 &scratch_buffer,
                                                 function,
                                                 instruction,
                                                 &relocation);
                            } break;

                            case MIR_RETURN:
                            {
                                x86_64_emit_return(scratch_arena,
                                                   &scratch_buffer,
                                                   function,
                                                   instruction);
                            } break;
                        }

                        instruction->binary_code_size = scratch_buffer.bytes_count;
                        current_offset_of_block_in_function += instruction->binary_code_size;

                        scratch_buffer.bytes_count = 0;
                    }
                }

                current_text_section_offset += current_offset_of_block_in_function;
            }
        }
    }

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        MIR_Function* function = &mir->functions[function_index];

        ASSERT(function->blocks_layout_computed);

        const MIR_Blocks_Layout* layout = &function->blocks_layout;

        for (Index block_index = 0;
             block_index < layout->blocks_count;
             ++block_index)
        {
            const MIR_Block* block = layout->blocks[block_index];

            for (const MIR_Instruction* instruction = block->first_instruction;
                 instruction != NULL;
                 instruction = instruction->next_instruction)
            {
                const Size binary_code_offset_before_instruction = function->binary_code_buffer.bytes_count;

                switch (instruction->opcode)
                {
                    case MIR_UNDEFINED:
                    case MIR_NOP:
                    case MIR_PHI:
                    case MIR_GET_PARAMETER:
                    {
                        UNREACHABLE();
                    } break;

                    case MIR_ADD:
                    case MIR_SUBTRACT:
                    {
                        x86_64_emit_add_or_subtract(context->binary_code_arena,
                                                    &function->binary_code_buffer,
                                                    function,
                                                    instruction);
                    } break;

                    case MIR_MULTIPLY:
                    {
                        FAIL("[MIR] MULTIPLY instruction is not supported yet.");
                    } break;

                    case MIR_DIVIDE:
                    {
                        FAIL("[MIR] DIVIDE instruction is not supported yet.");
                    } break;

                    case MIR_COMPARE:
                    {
                        x86_64_emit_compare(context->binary_code_arena,
                                            &function->binary_code_buffer,
                                            function,
                                            instruction);
                    } break;

                    case MIR_CAST:
                    {
                        FAIL("[MIR] CAST instruction is not supported yet.");
                    } break;

                    case MIR_LOAD:
                    {
                        FAIL("[MIR] LOAD instruction is not supported yet.");
                    } break;

                    case MIR_STORE:
                    {
                        FAIL("[MIR] STORE instruction is not supported yet.");
                    } break;

                    case MIR_GET_ADDRESS:
                    {
                        FAIL("[MIR] GET_ADDRESS instruction is not supported yet.");
                    } break;

                    case MIR_PUSH_REGISTER_ON_STACK:
                    case MIR_POP_REGISTER_FROM_STACK:
                    {
                        x86_64_emit_push_or_pop_register(context->binary_code_arena,
                                                         &function->binary_code_buffer,
                                                         function,
                                                         instruction);
                    } break;

                    case MIR_GROW_STACK:
                    case MIR_SHRINK_STACK:
                    {
                        x86_64_emit_grow_or_shrink_stack(context->binary_code_arena,
                                                         &function->binary_code_buffer,
                                                         function,
                                                         instruction);
                    } break;

                    case MIR_MOVE:
                    {
                        x86_64_emit_move(context->binary_code_arena,
                                         &function->binary_code_buffer,
                                         function,
                                         instruction);
                    } break;

                    case MIR_JUMP:
                    {
                        x86_64_emit_jump(context->binary_code_arena,
                                         &function->binary_code_buffer,
                                         function,
                                         instruction);
                    } break;

                    case MIR_JUMP_IF_FALSE:
                    {
                        x86_64_emit_jump_if_false(context->binary_code_arena,
                                                  &function->binary_code_buffer,
                                                  function,
                                                  instruction);
                    } break;

                    case MIR_CALL:
                    {
                        MIR_Relocation relocation = {0};

                        const Bool should_emit_relocation = x86_64_emit_call(context,
                                                                             context->binary_code_arena,
                                                                             &function->binary_code_buffer,
                                                                             function,
                                                                             instruction,
                                                                             &relocation);

                        if (should_emit_relocation)
                        {
                            append_array(context->relocations_arena,
                                         function->relocations,
                                         MIR_Relocation,
                                         relocation);
                        }
                    } break;

                    case MIR_RETURN:
                    {
                        x86_64_emit_return(context->binary_code_arena,
                                           &function->binary_code_buffer,
                                           function,
                                           instruction);
                    } break;
                }

                // NOTE(vlad): Sanity check: testing that the prepass correctly determined the size of instructions.
                {
                    const Size binary_code_offset_after_instruction = function->binary_code_buffer.bytes_count;
                    const Size emitted_instruction_size = binary_code_offset_after_instruction - binary_code_offset_before_instruction;

                    ASSERT(emitted_instruction_size != 0);
                    ASSERT(emitted_instruction_size == instruction->binary_code_size);
                }
            }
        }
    }
}
