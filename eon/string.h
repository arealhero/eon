#pragma once

#include <eon/build_info.h>
#include <eon/common.h>
#include <eon/memory.h>

struct String_View
{
    const char* data;
    Size length;
};
typedef struct String_View String_View;

struct String
{
    char* data;
    Size length;
};
typedef struct String String;

// NOTE(vlad): We cannot move these functions to 'string.c' because they must be visible
//             from the 'string_view()'s call site.
internal inline String_View INTERNAL_string_view_passthrough(const String_View string_view);
internal inline String_View INTERNAL_c_string_view(const char* const c_string);
internal inline String_View INTERNAL_string_view_from_string(const String string);
#define string_view(arg)                              \
    _Generic(                                         \
        (arg),                                        \
        char*: INTERNAL_c_string_view,                \
        const char*: INTERNAL_c_string_view,          \
        String: INTERNAL_string_view_from_string,     \
        String_View: INTERNAL_string_view_passthrough \
    )((arg))

internal s32 compare_strings(const String_View lhs, const String_View rhs);
internal Bool strings_are_equal(const String_View lhs, const String_View rhs);

internal String copy_string(Arena* const arena, const String_View string_to_copy);

internal void reverse_string(String string);

// TODO(vlad): Use '_Generic' to overload 'Bool' and 'Bool8'?
#define BOOL_TO_STRING(value) string_view(((value) ? "true" : "false"))

enum Number_Base
{
    NUMBER_BASE_BINARY = 2,
    NUMBER_BASE_DECIMAL = 10,
    NUMBER_BASE_HEXADECIMAL = 16,
};
typedef enum Number_Base Number_Base;

#define DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(Integer_Type)         \
    internal void Integer_Type##_to_string_inplace(String* string,      \
                                                   Integer_Type number, \
                                                   const Number_Base base)

DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(s8);
DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(s16);
DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(s32);
DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(s64);

DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(u8);
DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(u16);
DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(u32);
DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(u64);

#undef DECLARE_TO_STRING_INPLACE_FUNCTION_FOR_INTEGER

enum Format_Type_Tag
{
    TYPE_TAG_STRING,
    TYPE_TAG_STRING_VIEW,
    TYPE_TAG_C_STRING,

    TYPE_TAG_char,

    TYPE_TAG_s8,
    TYPE_TAG_s16,
    TYPE_TAG_s32,
    TYPE_TAG_s64,

    TYPE_TAG_u8,
    TYPE_TAG_u16,
    TYPE_TAG_u32,
    TYPE_TAG_u64,
};
typedef enum Format_Type_Tag Format_Type_Tag;

struct Format_Type_Info
{
    Format_Type_Tag tag;
    Size max_size_in_bytes;

    union
    {
        String string;
        String_View string_view;
        const char* c_string;

        char char_value;

        s8  s8_value;
        s16 s16_value;
        s32 s32_value;
        s64 s64_value;

        u8  u8_value;
        u16 u16_value;
        u32 u32_value;
        u64 u64_value;
    } argument;
};
typedef struct Format_Type_Info Format_Type_Info;

// NOTE(vlad): We cannot move these functions to 'string.c' because they must be visible
//             from the 'format_string()'s call site.
internal inline Format_Type_Info INTERNAL_format_tag_string(const String string);
internal inline Format_Type_Info INTERNAL_format_tag_string_view(const String_View string);
internal inline Format_Type_Info INTERNAL_format_tag_c_string(const char* string);

internal inline Format_Type_Info INTERNAL_format_tag_char(const char c);

#define DECLARE_FORMAT_TAG_FOR_INTEGER(Integer_Type)                    \
    internal inline Format_Type_Info INTERNAL_format_tag_##Integer_Type(Integer_Type number)

DECLARE_FORMAT_TAG_FOR_INTEGER(s8);
DECLARE_FORMAT_TAG_FOR_INTEGER(s16);
DECLARE_FORMAT_TAG_FOR_INTEGER(s32);
DECLARE_FORMAT_TAG_FOR_INTEGER(s64);

DECLARE_FORMAT_TAG_FOR_INTEGER(u8);
DECLARE_FORMAT_TAG_FOR_INTEGER(u16);
DECLARE_FORMAT_TAG_FOR_INTEGER(u32);
DECLARE_FORMAT_TAG_FOR_INTEGER(u64);

#undef DECLARE_FORMAT_TAG_FOR_INTEGER

internal String format_string_impl(Arena* const arena,
                                   const String_View format,
                                   const Size number_of_arguments,
                                   ...);

#define DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(Integer_Type)      \
    Integer_Type: INTERNAL_format_tag_##Integer_Type

#if OS_MAC
// NOTE(vlad): These checks are redundant because the width of 'ptrdiff_t'
//             is the same as the width of 'size_t', but just in case I'll leave them here.
#    if defined(__PTRDIFF_WIDTH__)
#        define EON_SIZE_WIDTH __PTRDIFF_WIDTH__
#    else
#        error MacOS: Failed to determine the width of "Size".
#    endif

#    if defined(__SIZE_WIDTH__)
#        define EON_USIZE_WIDTH __SIZE_WIDTH__
#    else
#        error MacOS: Failed to determine the width of "USize".
#    endif

#    if EON_SIZE_WIDTH == 32
#        define OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_SIZE() Size: INTERNAL_format_tag_s32,
#    elif EON_SIZE_WIDTH == 64
#        define OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_SIZE() Size: INTERNAL_format_tag_s64,
#    else
#        error MacOS: Width of "Size" is neither 32 nor 64 bits. I don't know what's going on.
#    endif

#    if EON_USIZE_WIDTH == 32
#        define OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_USIZE() USize: INTERNAL_format_tag_u32,
#    elif EON_USIZE_WIDTH == 64
#        define OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_USIZE() USize: INTERNAL_format_tag_u64,
#    else
#        error MacOS: Width of "USize" is neither 32 nor 64 bits. I don't know what's going on.
#    endif

#else
#    define OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_SIZE()
#    define OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_USIZE()
#endif

#define FORMAT_GET_TYPE_INFO(arg)                       \
    , /* NOTE(vlad): This comma is crucial. */          \
    _Generic(                                           \
        (arg),                                          \
                                                        \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(s8),       \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(s16),      \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(s32),      \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(s64),      \
                                                        \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(u8),       \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(u16),      \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(u32),      \
        DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(u64),      \
                                                        \
        OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_SIZE()  \
        OPTIONALLY_DECLARE_GENERIC_OVERLOAD_FOR_USIZE() \
                                                        \
        char: INTERNAL_format_tag_char,                 \
                                                        \
        char*: INTERNAL_format_tag_c_string,            \
        const char*: INTERNAL_format_tag_c_string,      \
                                                        \
        String: INTERNAL_format_tag_string,             \
        String_View: INTERNAL_format_tag_string_view    \
    )((arg))

// NOTE(vlad): We cannot #undef 'DECLARE_GENERIC_OVERLOAD_FOR_INTEGER' macro here
//             because it will be used upon calling 'format_string'.

#define format_string(arena, format, ...)                               \
    format_string_impl(arena,                                           \
                       string_view(format),                             \
                       NUMBER_OF_ARGS(__VA_ARGS__) /* NOTE(vlad): Comma is intentionally omitted */ \
                       FOR_EACH(FORMAT_GET_TYPE_INFO, __VA_ARGS__))
