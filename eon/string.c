#include "string.h"

#include <eon/io.h>

#include <stdarg.h>

internal Size
c_string_length(const char* c_string)
{
    if (c_string == NULL) return 0;

    Size length = 0;
    while (c_string[length] != '\0')
    {
        ++length;
    }

    return length;
}

// NOTE(vlad): No-op for 'string_view' generic.
internal inline String_View
INTERNAL_string_view_passthrough(const String_View string_view)
{
    return string_view;
}

internal inline String_View
INTERNAL_c_string_view(const char* const c_string)
{
    return (String_View) {
        .data   = c_string,
        .length = c_string_length(c_string),
    };
}

internal inline String_View
INTERNAL_string_view_from_string(const String string)
{
    return (String_View) {
        .data   = string.data,
        .length = string.length,
    };
}

maybe_unused internal s32
compare_strings(const String_View lhs, const String_View rhs)
{
    // TODO(vlad): Change to 'comptime_constant' if we will decide to use C11+.
    enum { LESS = -1, EQUAL = 0, GREATER = 1 };

    const Size min_length = MIN(lhs.length, rhs.length);

    for (Index i = 0;
         i < min_length;
         ++i)
    {
        if (lhs.data[i] < rhs.data[i])
        {
            return LESS;
        }
        else if (lhs.data[i] > rhs.data[i])
        {
            return GREATER;
        }
    }

    if (lhs.length < rhs.length)
    {
        return LESS;
    }
    else if (lhs.length > rhs.length)
    {
        return GREATER;
    }

    return EQUAL;
}

maybe_unused internal Bool
strings_are_equal(const String_View lhs, const String_View rhs)
{
    if (lhs.length != rhs.length) return false;

    for (Index i = 0;
         i < lhs.length;
         ++i)
    {
        if (lhs.data[i] != rhs.data[i])
        {
            return false;
        }
    }

    return true;
}

internal String
copy_string(Arena* const arena, const String_View string_to_copy)
{
    String result = {0};
    result.data = allocate_uninitialized_array(arena, string_to_copy.length, char);
    result.length = string_to_copy.length;

    copy_memory(as_bytes(result.data),
                as_bytes(string_to_copy.data),
                string_to_copy.length);

    return result;
}

internal void
reverse_string(String string)
{
    char* start = string.data;
    char* end = string.data + string.length - 1;

    while (start < end) {
        const char c = *end;
        *end = *start;
        *start = c;

        start += 1;
        end -= 1;
    }
}

#define NUMBER_TO_STRING_ALPHABET "fedcba9876543210123456789abcdef"
#define DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(Integer_Type, is_signed, width_in_bits) \
    internal void                                                       \
    Integer_Type##_to_string_inplace(String* string,                    \
                                     Integer_Type number,               \
                                     const Number_Base base)            \
    {                                                                   \
        char* ptr = string->data;                                       \
                                                                        \
        Integer_Type previous_value;                                    \
        do {                                                            \
            previous_value = number;                                    \
                                                                        \
            number /= (Integer_Type)base;                               \
            *ptr = NUMBER_TO_STRING_ALPHABET[15 + (previous_value - number * (Integer_Type)base)]; \
                                                                        \
            ptr += 1;                                                   \
        } while (number);                                               \
                                                                        \
        /* NOTE(vlad): Print minus sign in decimal base only (like MSVC's `itoa') */ \
        if (is_signed && base == NUMBER_BASE_DECIMAL && previous_value < 0) { \
            *ptr = '-';                                                 \
            ptr += 1;                                                   \
        }                                                               \
                                                                        \
        /* XXX(vlad): do not overallocate? */                           \
        string->length = ptr - string->data;                            \
                                                                        \
        reverse_string(*string);                                        \
    }                                                                   \
    REQUIRE_SEMICOLON

DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(s8, true, 8);
DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(s16, true, 16);
DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(s32, true, 32);
DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(s64, true, 64);

DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(u8, true, 8);
DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(u16, true, 16);
DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(u32, true, 32);
DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(u64, true, 64);

internal inline Format_Type_Info
INTERNAL_format_tag_string(const String string)
{
    return (Format_Type_Info){
        .tag = TYPE_TAG_STRING_VIEW,
        .max_size_in_bytes = string.length,
        .argument = {
            .string_view = (String_View){ .data = string.data, .length = string.length, },
        },
    };
}

internal inline Format_Type_Info
INTERNAL_format_tag_string_view(const String_View string)
{
    return (Format_Type_Info){
        .tag = TYPE_TAG_STRING_VIEW,
        .max_size_in_bytes = string.length,
        .argument = {
            .string_view = string,
        },
    };
}

internal inline Format_Type_Info
INTERNAL_format_tag_c_string(const char* string)
{
    const Size length = c_string_length(string);
    return (Format_Type_Info){
        .tag = TYPE_TAG_STRING_VIEW,
        .max_size_in_bytes = length,
        .argument = {
            .string_view = (String_View){ .data = string, .length = length, },
        },
    };
}

internal inline Format_Type_Info
INTERNAL_format_tag_char(const char c)
{
    return (Format_Type_Info){
        .tag = TYPE_TAG_char,
        .max_size_in_bytes = 1,
        .argument = {
            .char_value = c,
        },
    };
}

#define DEFINE_FORMAT_TAG_FOR_INTEGER(Integer_Type, is_sized, width_in_bits) \
    internal inline Format_Type_Info                                    \
    INTERNAL_format_tag_##Integer_Type(const Integer_Type number)       \
    {                                                                   \
        return (Format_Type_Info){                                      \
            .tag = TYPE_TAG_##Integer_Type,                             \
            .max_size_in_bytes = width_in_bits + is_sized,              \
            .argument = {                                               \
                .Integer_Type##_value = number,                         \
            },                                                          \
        };                                                              \
    }                                                                   \
    REQUIRE_SEMICOLON

DEFINE_FORMAT_TAG_FOR_INTEGER(s8,  true, 8);
DEFINE_FORMAT_TAG_FOR_INTEGER(s16, true, 16);
DEFINE_FORMAT_TAG_FOR_INTEGER(s32, true, 32);
DEFINE_FORMAT_TAG_FOR_INTEGER(s64, true, 64);

DEFINE_FORMAT_TAG_FOR_INTEGER(u8,  true, 8);
DEFINE_FORMAT_TAG_FOR_INTEGER(u16, true, 16);
DEFINE_FORMAT_TAG_FOR_INTEGER(u32, true, 32);
DEFINE_FORMAT_TAG_FOR_INTEGER(u64, true, 64);

internal Size
validate_arguments_and_calculate_total_size_in_bytes(String_View format,
                                                     Size number_of_arguments,
                                                     va_list args)
{
    Size total_size = 0;
    Bool brace_was_opened = false;

    va_list copy_of_args;
    va_copy(copy_of_args, args);

    for (Index i = 0;
         i < format.length;
         ++i)
    {
        const char c = format.data[i];

        switch (c)
        {
            case '{':
            {
                if (brace_was_opened)
                {
                    FAIL("format_string: nested braces are not allowed.");
                }

                brace_was_opened = true;
            } break;

            case '}':
            {
                if (!brace_was_opened)
                {
                    total_size += 1;
                    break;
                }

                if (number_of_arguments == 0)
                {
                    FAIL("format_string: not enough parameters provided.");
                }

                number_of_arguments -= 1;

                Format_Type_Info info = va_arg(copy_of_args, Format_Type_Info);
                total_size += info.max_size_in_bytes;

                brace_was_opened = false;
            } break;

            default:
            {
                // FIXME(vlad): support format settings in braces like std::format or fmt:format
                if (!brace_was_opened)
                {
                    total_size += 1;
                }

                // if (brace_was_opened)
                // {
                //     brace_was_opened = 0;
                //     total_size += 1;
                // }

                total_size += 1;
            } break;
        }
    }

    if (brace_was_opened)
    {
        // NOTE(vlad): Trailing brace
        total_size += 1;
    }

    if (number_of_arguments != 0)
    {
        // NOTE(vlad): Although the 'std::format' documentation explicitly states that
        //             it's not an error to provide more arguments than required
        //             in the format string, we will treat this situation as an error.
        //             @ref: https://en.cppreference.com/w/cpp/utility/format/format.html#Notes
        FAIL("format_string: excessive parameters provided.");
    }

    va_end(copy_of_args);

    return total_size;
}

maybe_unused internal String
vformat_string_impl(Arena* const arena,
                    const String_View format,
                    const Size number_of_arguments,
                    va_list args)
{
    const Size formatted_string_length
        = validate_arguments_and_calculate_total_size_in_bytes(format,
                                                               number_of_arguments,
                                                               args);

    // FIXME(vlad): Ensure that buffer has enough space before every call to 'copy_memory' in this function.
    char* buffer = allocate_uninitialized_array(arena, formatted_string_length, char);
    Index buffer_index = 0;

    Number_Base number_base = NUMBER_BASE_DECIMAL;
    Bool brace_was_opened = false;
    for (Index i = 0;
         i < format.length;
         ++i)
    {
        const char c = format.data[i];

        switch (c)
        {
            case '{':
            {
                brace_was_opened = true;
                number_base = NUMBER_BASE_DECIMAL;
            } break;

            case '}':
            {
                if (!brace_was_opened)
                {
                    buffer[buffer_index++] = c;
                    break;
                }

                Format_Type_Info info = va_arg(args, Format_Type_Info);

                switch (info.tag)
                {
                    case TYPE_TAG_STRING_VIEW:
                    {
                        copy_memory(as_bytes(buffer + buffer_index),
                                    as_bytes(info.argument.string_view.data),
                                    info.argument.string_view.length);
                        buffer_index += info.argument.string_view.length;
                    } break;

                    case TYPE_TAG_char:
                    {
                        buffer[buffer_index] = info.argument.char_value;
                        buffer_index += 1;
                    } break;

                    // TODO(vlad): Support other bases.
#define HANDLE_INTEGER_CASE(Integer_Type)                               \
                    case TYPE_TAG_##Integer_Type:                       \
                    {                                                   \
                        String target = {0};                            \
                        target.data = buffer + buffer_index;            \
                        target.length = info.max_size_in_bytes;         \
                        Integer_Type##_to_string_inplace(&target,       \
                                                         info.argument.Integer_Type##_value, \
                                                         number_base);  \
                        buffer_index += target.length;                  \
                    } break;                                            \
                    REQUIRE_SEMICOLON

                    HANDLE_INTEGER_CASE(s8);
                    HANDLE_INTEGER_CASE(s16);
                    HANDLE_INTEGER_CASE(s32);
                    HANDLE_INTEGER_CASE(s64);

                    HANDLE_INTEGER_CASE(u8);
                    HANDLE_INTEGER_CASE(u16);
                    HANDLE_INTEGER_CASE(u32);
                    HANDLE_INTEGER_CASE(u64);
#undef HANDLE_INTEGER_CASE

                }

                brace_was_opened = false;
            } break;

            default:
            {
                if (brace_was_opened)
                {
                    if (c == 'h')
                    {
                        number_base = NUMBER_BASE_HEXADECIMAL;
                    }
                    else
                    {
                        println("Unknown character encountered: {}", c);
                        FAIL("Unknown character encountered");
                    }
                }
                else
                {
                    buffer[buffer_index++] = c;
                }

                // if (brace_was_opened)
                // {
                //     buffer[buffer_index++] = '{';
                //     brace_was_opened = false;
                // }
            } break;
        }
    }

    if (brace_was_opened)
    {
        // NOTE(vlad): Trailing brace.
        // FIXME(vlad): Restore optional format-like string that followed '{'.
        buffer[buffer_index++] = '{';
    }

    String formatted_string = {0};
    formatted_string.data = buffer;
    formatted_string.length = buffer_index;
    return formatted_string;
}

maybe_unused internal String
format_string_impl(Arena* const arena,
                   const String_View format,
                   const Size number_of_arguments,
                   ...)
{
    if (number_of_arguments == 0)
    {
        return copy_string(arena, format);
    }

    va_list args;
    va_start(args, number_of_arguments);
    const String formatted_string = vformat_string_impl(arena, format, number_of_arguments, args);
    va_end(args);

    return formatted_string;
}
