#pragma once

#include <eon/build_info.h>
#include <eon/common.h>

struct Arena;

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

maybe_unused internal char* to_c_string(struct Arena* arena, const String_View string_view);

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

internal String copy_string(struct Arena* const arena, const String_View string_to_copy);

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

// TODO(vlad): Make these functions internal and use _Generic-based macro instead.
#define DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION(Integer_Type)         \
    internal void Integer_Type##_to_string_inplace(String* string,      \
                                                   Integer_Type number, \
                                                   const Number_Base base);
#define DECLARE_STRING_TO_NUMBER_FUNCTION(Integer_Type)                 \
    maybe_unused internal Bool INTERNAL_parse_##Integer_Type(const String_View string, \
                                                             Integer_Type* out_integer);

FOR_EACH_INTEGER_TYPE(DECLARE_NUMBER_TO_STRING_INPLACE_FUNCTION)
FOR_EACH_INTEGER_TYPE(DECLARE_STRING_TO_NUMBER_FUNCTION)

#undef DECLARE_TO_STRING_INPLACE_FUNCTION_FOR_INTEGER
#undef DECLARE_STRING_TO_NUMBER_FUNCTION

#define DECLARE_PARSE_INTEGER_OVERLOAD(Integer_Type) Integer_Type*: INTERNAL_parse_##Integer_Type,
#define parse_integer(string, out_integer)                              \
    _Generic((out_integer),                                             \
                 FOR_EACH_INTEGER_TYPE(DECLARE_PARSE_INTEGER_OVERLOAD)  \
                 default: "Integer expected"                            \
             )(string_view(string), out_integer)
// NOTE(vlad): We cannot #undef 'DECLARE_PARSE_INTEGER_OVERLOAD' macro here
//             because it will be needed upon calling 'parse_integer'.

enum Format_Type_Tag
{
    TYPE_TAG_STRING_VIEW,

    TYPE_TAG_char,

    TYPE_TAG_f32,
    TYPE_TAG_f64,

#define DECLARE_INTEGER_TYPE_TAG(Integer_Type) TYPE_TAG_##Integer_Type,
    FOR_EACH_INTEGER_TYPE(DECLARE_INTEGER_TYPE_TAG)
#undef DECLARE_INTEGER_TYPE_TAG
};
typedef enum Format_Type_Tag Format_Type_Tag;

struct Format_Type_Info
{
    Format_Type_Tag tag;
    Size max_size_in_bytes;

    union
    {
        String_View string_view;

        char char_value;
        f32 f32_value;
        f64 f64_value;

#define DECLARE_INTEGER_VALUE(Integer_Type) Integer_Type Integer_Type##_value;
        FOR_EACH_INTEGER_TYPE(DECLARE_INTEGER_VALUE)
#undef DECLARE_INTEGER_VALUE
    } argument;
};
typedef struct Format_Type_Info Format_Type_Info;

// NOTE(vlad): We cannot move these functions to 'string.c' because they must be visible
//             from the 'format_string()'s call site.
internal inline Format_Type_Info INTERNAL_format_tag_string(const String string);
internal inline Format_Type_Info INTERNAL_format_tag_string_view(const String_View string);
internal inline Format_Type_Info INTERNAL_format_tag_c_string(const char* string);

internal inline Format_Type_Info INTERNAL_format_tag_char(const char c);

internal inline Format_Type_Info INTERNAL_format_tag_f32(const f32 number);
internal inline Format_Type_Info INTERNAL_format_tag_f64(const f64 number);

#define DECLARE_FORMAT_TAG_FOR_INTEGER(Integer_Type)                    \
    internal inline Format_Type_Info INTERNAL_format_tag_##Integer_Type(Integer_Type number);
FOR_EACH_INTEGER_TYPE(DECLARE_FORMAT_TAG_FOR_INTEGER)
#undef DECLARE_FORMAT_TAG_FOR_INTEGER

internal String format_string_impl(struct Arena* const arena,
                                   const String_View format,
                                   const Size number_of_arguments,
                                   ...);

#define DECLARE_GENERIC_OVERLOAD_FOR_INTEGER(Integer_Type)      \
    Integer_Type: INTERNAL_format_tag_##Integer_Type,

#define FORMAT_GET_TYPE_INFO(arg)                                   \
    , /* NOTE(vlad): This comma is crucial. */                      \
    _Generic(                                                       \
        (arg),                                                      \
                                                                    \
        FOR_EACH_INTEGER_TYPE(DECLARE_GENERIC_OVERLOAD_FOR_INTEGER) \
                                                                    \
        char: INTERNAL_format_tag_char,                             \
                                                                    \
        char*: INTERNAL_format_tag_c_string,                        \
        const char*: INTERNAL_format_tag_c_string,                  \
                                                                    \
        f32: INTERNAL_format_tag_f32,                               \
        f64: INTERNAL_format_tag_f64,                               \
                                                                    \
        String: INTERNAL_format_tag_string,                         \
        String_View: INTERNAL_format_tag_string_view                \
    )((arg))

// NOTE(vlad): We cannot #undef 'DECLARE_GENERIC_OVERLOAD_FOR_INTEGER' macro here
//             because it will be used upon calling 'format_string'.

#define format_string(arena, format, ...)                               \
    format_string_impl(arena,                                           \
                       string_view(format),                             \
                       NUMBER_OF_ARGS(__VA_ARGS__) /* NOTE(vlad): Comma is intentionally omitted */ \
                       FOR_EACH(FORMAT_GET_TYPE_INFO, __VA_ARGS__))
