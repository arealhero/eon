#pragma once

#include <eon/types.h>
#include <eon/string.h>

struct Builtin_Type
{
    C_String name;
};
typedef struct Builtin_Type Builtin_Type;

struct Integer_Builtin_Type
{
    C_String name;
    Size width_in_bits;
    Bool is_signed;
};
typedef struct Integer_Builtin_Type Integer_Builtin_Type;

struct Float_Builtin_Type
{
    C_String name;
    Size width_in_bits;
};
typedef struct Float_Builtin_Type Float_Builtin_Type;

static Builtin_Type BUILTIN_TYPES[] = {
    (Builtin_Type) { .name = "void" },
};

static Integer_Builtin_Type INTEGER_BUILTIN_TYPES[] = {
    (Integer_Builtin_Type) { .name = "s8",  .width_in_bits = 8,  .is_signed = true },
    (Integer_Builtin_Type) { .name = "s16", .width_in_bits = 16, .is_signed = true },
    (Integer_Builtin_Type) { .name = "s32", .width_in_bits = 32, .is_signed = true },
    (Integer_Builtin_Type) { .name = "s64", .width_in_bits = 64, .is_signed = true },
    (Integer_Builtin_Type) { .name = "u8",  .width_in_bits = 8,  .is_signed = false },
    (Integer_Builtin_Type) { .name = "u16", .width_in_bits = 16, .is_signed = false },
    (Integer_Builtin_Type) { .name = "u32", .width_in_bits = 32, .is_signed = false },
    (Integer_Builtin_Type) { .name = "u64", .width_in_bits = 64, .is_signed = false },
};

static Float_Builtin_Type FLOAT_BUILTIN_TYPES[] = {
    (Float_Builtin_Type) { .name = "f32",  .width_in_bits = 8 },
    (Float_Builtin_Type) { .name = "f64", .width_in_bits = 16 },
};
