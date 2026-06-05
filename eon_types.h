#pragma once

#include <eon/types.h>
#include <eon/memory.h>

#include "eon_compilation_context.h"

enum Type_Kind
{
    TYPE_UNDEFINED = 0,

    TYPE_VOID,

    TYPE_VARIABLE, // NOTE(vlad): Think of it as a variable in an equation,
                   //             but the type system's "equation" is a set
                   //             of constraints that must be fulfilled.
    TYPE_NUMBER_VARIABLE,

    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_BOOLEAN,

    TYPE_POINTER,
    TYPE_FUNCTION,

    // TYPE_STRUCT, // TODO(vlad): Rename to something like 'USER_DEFINED_TYPE'?
    // TYPE_ENUMERATION,
};
typedef enum Type_Kind Type_Kind;

struct Integer_Type_Info
{
    Size width_in_bits;
    Bool is_signed;
};
typedef struct Integer_Type_Info Integer_Type_Info;

struct Float_Type_Info
{
    Size width_in_bits;
};
typedef struct Float_Type_Info Float_Type_Info;

struct Pointer_Type_Info
{
    Type_Id points_to_type_id;
    Bool pointee_is_mutable;
};
typedef struct Pointer_Type_Info Pointer_Type_Info;

struct Function_Type_Info
{
    Type_Id* parameter_type_ids;
    Size parameter_type_ids_count;

    Type_Id return_type_id;
};
typedef struct Function_Type_Info Function_Type_Info;

enum Sign_Constraint
{
    SIGN_UNKNOWN = 0,
    SIGN_SIGNED,
    SIGN_UNSIGNED,
};
typedef enum Sign_Constraint Sign_Constraint;

struct Number_Constraints
{
    Bool must_be_a_floating_point_number;
    Sign_Constraint sign;
    Index min_bit_width;
    u64 integer_value;
};
typedef struct Number_Constraints Number_Constraints;

// NOTE(vlad): Type is a node in a disjoint-set.
struct Type
{
    Type_Kind kind;

    Type_Id parent_type_id;
    Bool is_mutable;

    union
    {
        Integer_Type_Info integer_info;
        Float_Type_Info float_info;
        Pointer_Type_Info pointer_info;
        Function_Type_Info function_info;

        Number_Constraints number_constraints;
    };
};
typedef struct Type Type;

maybe_unused internal inline Bool type_id_is_defined(const Type_Id type_id);
maybe_unused internal inline Bool type_id_is_undefined(const Type_Id type_id);

maybe_unused internal inline Bool type_id_is_valid(Compilation_Context* context, const Type_Id type_id);
maybe_unused internal inline Bool type_id_is_invalid(Compilation_Context* context, const Type_Id type_id);

maybe_unused internal Bool resolve_and_validate_types(Compilation_Context* context);
maybe_unused internal String_View convert_type_to_string(Arena* arena,
                                                         Compilation_Context* context,
                                                         const Type_Id type_id);
