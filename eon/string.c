#include "string.h"

#include <eon/io.h>

#include <stdarg.h>

internal char*
to_c_string(Arena* arena, const String_View string_view)
{
    char* c_string = allocate_uninitialized_array(arena, string_view.length + 1, char);
    copy_memory(as_bytes(c_string),
                as_bytes(string_view.data),
                string_view.length);
    c_string[string_view.length] = '\0';
    return c_string;
}

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

// TODO(vlad): Change 'NUMBER' to 'INTEGER' here.
#define NUMBER_TO_STRING_ALPHABET "fedcba9876543210123456789abcdef"
#define DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION(Integer_Type)          \
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
        }                                                               \
        while (number);                                                 \
                                                                        \
        /* NOTE(vlad): Print minus sign in decimal base only (like MSVC's `itoa') */ \
        if (TYPE_IS_SIGNED(Integer_Type)                                \
            && base == NUMBER_BASE_DECIMAL                              \
            && previous_value < 0)                                      \
        {                                                               \
            *ptr = '-';                                                 \
            ptr += 1;                                                   \
        }                                                               \
                                                                        \
        /* XXX(vlad): do not overallocate? */                           \
        string->length = ptr - string->data;                            \
                                                                        \
        reverse_string(*string);                                        \
    }

FOR_EACH_INTEGER_TYPE(DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION)
#undef DEFINE_NUMBER_TO_STRING_INPLACE_FUNCTION

#define DEFINE_PARSE_INTEGER_FUNCTION(Integer_Type)                     \
    internal Bool                                                       \
    INTERNAL_parse_##Integer_Type(const String_View string,             \
                                  Integer_Type* out_integer)            \
    {                                                                   \
        Integer_Type result = 0;                                        \
        Integer_Type sign = 1;                                          \
        Index i = 0;                                                    \
                                                                        \
        if (i < string.length)                                          \
        {                                                               \
            const char c = string.data[i];                              \
            if (c == '+')                                               \
            {                                                           \
                sign = 1;                                               \
                i += 1;                                                 \
            }                                                           \
            else if (TYPE_IS_SIGNED(Integer_Type) && c == '-')          \
            {                                                           \
                sign = (Integer_Type)(-1);                              \
                i += 1;                                                 \
            }                                                           \
        }                                                               \
                                                                        \
        if (i >= string.length)                                         \
        {                                                               \
            return false;                                               \
        }                                                               \
                                                                        \
        for (;                                                          \
             i < string.length;                                         \
             ++i)                                                       \
        {                                                               \
            const char c = string.data[i];                              \
            const Bool is_digit = '0' <= c && c <= '9';                 \
            if (!is_digit)                                              \
            {                                                           \
                return false;                                           \
            }                                                           \
                                                                        \
            const Integer_Type next_digit = (Integer_Type)(c - '0');    \
                                                                        \
            if (sign == (Integer_Type)1)                                \
            {                                                           \
                if (result > MAX_VALUE(Integer_Type) / 10)              \
                {                                                       \
                    return false;                                       \
                }                                                       \
                                                                        \
                if (result == MAX_VALUE(Integer_Type) / 10              \
                    && next_digit > MAX_VALUE(Integer_Type) % 10)       \
                {                                                       \
                    return false;                                       \
                }                                                       \
            }                                                           \
            else if (TYPE_IS_SIGNED(Integer_Type) && sign == (Integer_Type)(-1)) \
            {                                                           \
                if (result > ABS(MIN_VALUE(Integer_Type) / 10))         \
                {                                                       \
                    return false;                                       \
                }                                                       \
                                                                        \
                if (result == ABS(MIN_VALUE(Integer_Type) / 10)         \
                    && next_digit > ABS(MIN_VALUE(Integer_Type) % 10))  \
                {                                                       \
                    return false;                                       \
                }                                                       \
            }                                                           \
            else                                                        \
            {                                                           \
                UNREACHABLE();                                          \
            }                                                           \
                                                                        \
            result = 10 * result + next_digit;                          \
        }                                                               \
        *out_integer = sign * result;                                   \
        return true;                                                    \
    }

FOR_EACH_INTEGER_TYPE(DEFINE_PARSE_INTEGER_FUNCTION)
#undef DEFINE_PARSE_INTEGER_FUNCTION

#define DEFINE_FLOAT_TO_STRING_INPLACE_FUNCTION(Float_Type)             \
    internal void                                                       \
    Float_Type##_to_string_inplace(String* string, const Float_Type number, Size precision) \
    {                                                                   \
        u64 integer_part = (u64) (ABS(number));                         \
                                                                        \
        Float_Type fraction_part_float = ABS(number) - (Float_Type)integer_part; \
        for (u32 i = 0;                                                 \
             i < precision;                                             \
             ++i)                                                       \
        {                                                               \
            fraction_part_float *= 10;                                  \
        }                                                               \
                                                                        \
        u64 fraction_part = (u64) (fraction_part_float);                \
                                                                        \
        /* NOTE(vlad): Poor man's rounding. */                          \
        {                                                               \
            const u64 digit_after_fraction_part = (u64) ((fraction_part_float - (Float_Type)fraction_part) * 10); \
            if (digit_after_fraction_part >= 5)                         \
            {                                                           \
                fraction_part += 1;                                     \
            }                                                           \
        }                                                               \
                                                                        \
        const Bool number_is_negative = number < 0.0f;                  \
                                                                        \
        Index index = 0;                                                \
        if (number_is_negative)                                         \
        {                                                               \
            string->data[index++] = '-';                                \
        }                                                               \
                                                                        \
        if (integer_part == 0)                                          \
        {                                                               \
            string->data[index++] = '0';                                \
        }                                                               \
        else                                                            \
        {                                                               \
            String string_to_reverse = {0};                             \
            Index start_index = index;                                  \
                                                                        \
            while (integer_part != 0)                                   \
            {                                                           \
                string->data[index++] = (integer_part % 10) + '0';      \
                integer_part /= 10;                                     \
            }                                                           \
                                                                        \
            string_to_reverse.data = string->data + start_index;        \
            string_to_reverse.length = index - start_index;             \
                                                                        \
            reverse_string(string_to_reverse);                          \
        }                                                               \
                                                                        \
        string->data[index++] = '.';                                    \
                                                                        \
        if (fraction_part == 0)                                         \
        {                                                               \
            while (precision > 0)                                       \
            {                                                           \
                string->data[index++] = '0';                            \
                precision -= 1;                                         \
            }                                                           \
        }                                                               \
        else                                                            \
        {                                                               \
            String string_to_reverse = {0};                             \
            Index start_index = index;                                  \
                                                                        \
            while (fraction_part != 0)                                  \
            {                                                           \
                string->data[index++] = (fraction_part % 10) + '0';     \
                fraction_part /= 10;                                    \
            }                                                           \
                                                                        \
            string_to_reverse.data = string->data + start_index;        \
            string_to_reverse.length = index - start_index;             \
                                                                        \
            reverse_string(string_to_reverse);                          \
        }                                                               \
                                                                        \
        string->length = index;                                         \
    }
FOR_EACH_FLOAT_TYPE(DEFINE_FLOAT_TO_STRING_INPLACE_FUNCTION)
#undef DEFINE_FLOAT_TO_STRING_INPLACE_FUNCTION

#define DEFINE_PARSE_FLOAT_FUNCTION(Float_Type)                         \
    maybe_unused internal Bool                                          \
    INTERNAL_parse_##Float_Type(const String_View string, Float_Type* out_float) \
    {                                                                   \
        Index dot_index = -1;                                           \
        for (Index i = 0;                                               \
             i < string.length;                                         \
             ++i)                                                       \
        {                                                               \
            if (string.data[i] == '.')                                  \
            {                                                           \
                dot_index = i;                                          \
                break;                                                  \
            }                                                           \
        }                                                               \
                                                                        \
        if (dot_index == -1)                                            \
        {                                                               \
            s64 integer;                                                \
            if (!parse_integer(string, &integer))                       \
            {                                                           \
                return false;                                           \
            }                                                           \
                                                                        \
            *out_float = (Float_Type) integer;                          \
            return true;                                                \
        }                                                               \
                                                                        \
        String_View integer_part = {0};                                 \
        integer_part.data = string.data;                                \
        integer_part.length = dot_index;                                \
                                                                        \
        s64 integer;                                                    \
        if (!parse_integer(integer_part, &integer))                     \
        {                                                               \
            return false;                                               \
        }                                                               \
                                                                        \
        String_View fraction_part = {0};                                \
        fraction_part.data = string.data + dot_index + 1;               \
        fraction_part.length = string.length - dot_index - 1;           \
                                                                        \
        u64 fraction;                                                   \
        if (!parse_integer(fraction_part, &fraction))                   \
        {                                                               \
            return false;                                               \
        }                                                               \
                                                                        \
        u64 fraction_divisor = 1;                                       \
        for (Index i = 0;                                               \
             i < fraction_part.length;                                  \
             ++i)                                                       \
        {                                                               \
            fraction_divisor *= 10;                                     \
        }                                                               \
                                                                        \
        if (integer > 0)                                                \
        {                                                               \
            *out_float = (Float_Type)(integer) + (Float_Type)(fraction) / (Float_Type)(fraction_divisor); \
        }                                                               \
        else                                                            \
        {                                                               \
            *out_float = (Float_Type)(integer) - (Float_Type)(fraction) / (Float_Type)(fraction_divisor); \
        }                                                               \
                                                                        \
        return true;                                                    \
    }
FOR_EACH_FLOAT_TYPE(DEFINE_PARSE_FLOAT_FUNCTION)
#undef DEFINE_PARSE_FLOAT_FUNCTION

#define DEFINE_GET_NUMBER_LENGTH_AS_A_STRING_FUNCTION(Integer_Type)     \
    internal inline Size                                                \
    get_##Integer_Type##_length_as_a_string(Integer_Type number,        \
                                            const Number_Base base)     \
    {                                                                   \
        if (number == 0)                                                \
        {                                                               \
            return 1;                                                   \
        }                                                               \
                                                                        \
        Size result = 0;                                                \
        if (TYPE_IS_SIGNED(Integer_Type)                                \
            && number < (Integer_Type)0                                 \
            && base == NUMBER_BASE_DECIMAL)                             \
        {                                                               \
            result += 1;                                                \
        }                                                               \
                                                                        \
        while (number != 0)                                             \
        {                                                               \
            result += 1;                                                \
            number /= (Integer_Type)base;                               \
        }                                                               \
                                                                        \
        return result;                                                  \
    }

FOR_EACH_INTEGER_TYPE(DEFINE_GET_NUMBER_LENGTH_AS_A_STRING_FUNCTION)
#undef DEFINE_GET_NUMBER_LENGTH_AS_A_STRING_FUNCTION

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

internal inline Format_Type_Info
INTERNAL_format_tag_f32(const f32 number)
{
    return (Format_Type_Info){
        .tag = TYPE_TAG_f32,
        .max_size_in_bytes = 100, // FIXME(vlad): Remove this hardcoded value.
        .argument = {
            .f32_value = number,
        },
    };
}

internal inline Format_Type_Info
INTERNAL_format_tag_f64(const f64 number)
{
    return (Format_Type_Info){
        .tag = TYPE_TAG_f64,
        .max_size_in_bytes = 100, // FIXME(vlad): Remove this hardcoded value.
        .argument = {
            .f64_value = number,
        },
    };
}

#define DEFINE_FORMAT_TAG_FOR_INTEGER(Integer_Type)                     \
    internal inline Format_Type_Info                                    \
    INTERNAL_format_tag_##Integer_Type(const Integer_Type number)       \
    {                                                                   \
        return (Format_Type_Info){                                      \
            .tag = TYPE_TAG_##Integer_Type,                             \
            .max_size_in_bytes = WIDTH_IN_BITS(number) + IS_SIGNED(number), \
            .argument = {                                               \
                .Integer_Type##_value = number,                         \
            },                                                          \
        };                                                              \
    }

FOR_EACH_INTEGER_TYPE(DEFINE_FORMAT_TAG_FOR_INTEGER)
#undef DEFINE_FORMAT_TAG_FOR_INTEGER

struct Integer_Format_Settings
{
    Number_Base base;
    Size left_pad_count;
    char left_pad_char;
};
typedef struct Integer_Format_Settings Integer_Format_Settings;

internal Integer_Format_Settings
parse_integer_format_settings(const String_View format)
{
    enum Integer_Format_Keyword_Type
    {
        INTEGER_FORMAT_KEYWORD_INVALID = 0,

        INTEGER_FORMAT_KEYWORD_BASE,
        INTEGER_FORMAT_KEYWORD_LEFT_PAD_COUNT,
        INTEGER_FORMAT_KEYWORD_LEFT_PAD_CHAR,
    };

    struct Keyword
    {
        enum Integer_Format_Keyword_Type type;
        String_View lexeme;
    };

    const struct Keyword keywords[] = {
        {
            .type = INTEGER_FORMAT_KEYWORD_BASE,
            .lexeme = string_view("base"),
        },
        {
            .type = INTEGER_FORMAT_KEYWORD_LEFT_PAD_COUNT,
            .lexeme = string_view("left-pad-count"),
        },
        {
            .type = INTEGER_FORMAT_KEYWORD_LEFT_PAD_CHAR,
            .lexeme = string_view("left-pad-char"),
        },
    };
    const Size keywords_count = size_of(keywords) / size_of(keywords[0]);

    Integer_Format_Settings settings = {0};
    settings.base = NUMBER_BASE_DECIMAL;
    settings.left_pad_count = 0;
    settings.left_pad_char = ' ';

    Index index = 0;
    while (index < format.length)
    {
        Index lexeme_start_index = index;

        const char c = format.data[index];

        if (c == ' ')
        {
            index += 1;
            continue;
        }

        // TODO(vlad): Use 'is_letter'.
        if ('a' <= c && c <= 'z')
        {
            index += 1;
            while (index < format.length)
            {
                const char this_char = format.data[index];

                const Bool is_valid_lexeme_symbol =
                    ('a' <= this_char && this_char <= 'z')
                    || (this_char == '-');

                if (!is_valid_lexeme_symbol)
                {
                    break;
                }

                index += 1;
            }

            String_View lexeme = {0};
            lexeme.data = format.data + lexeme_start_index;
            lexeme.length = index - lexeme_start_index;

            enum Integer_Format_Keyword_Type type = INTEGER_FORMAT_KEYWORD_INVALID;
            for (Index keyword_index = 0;
                 keyword_index < keywords_count;
                 ++keyword_index)
            {
                const struct Keyword* this_keyword = &keywords[keyword_index];
                if (strings_are_equal(lexeme, this_keyword->lexeme))
                {
                    type = this_keyword->type;
                    break;
                }
            }

            if (type == INTEGER_FORMAT_KEYWORD_INVALID)
            {
                FAIL("Invalid integer format specifier found");
            }

            if (index >= format.length)
            {
                FAIL("Unexpected end of format settings found");
            }

            if (format.data[index] != ':')
            {
                FAIL("Unexpected char found (expected ':')");
            }

            index += 1;

            while (index < format.length)
            {
                if (format.data[index] == ' ')
                {
                    index += 1;
                }
                else
                {
                    break;
                }
            }

            if (index >= format.length)
            {
                FAIL("Unexpected end of format settings found");
            }

            switch (type)
            {
                case INTEGER_FORMAT_KEYWORD_INVALID:
                {
                    UNREACHABLE();
                } break;

                case INTEGER_FORMAT_KEYWORD_BASE:
                {
                    Size base = 0;
                    while (index < format.length)
                    {
                        const char this_char = format.data[index];

                        if ('0' <= this_char && this_char <= '9')
                        {
                            base = 10 * base + (this_char - '0');
                            index += 1;
                        }
                        else if (this_char == ',')
                        {
                            break;
                        }
                        else
                        {
                            FAIL("Unexpected char found");
                        }
                    }

                    switch (base)
                    {
                        case NUMBER_BASE_BINARY:
                        case NUMBER_BASE_DECIMAL:
                        case NUMBER_BASE_HEXADECIMAL:
                        {
                            settings.base = (Number_Base) base;
                        } break;

                        default:
                        {
                            FAIL("Unsupported base encountered");
                        } break;
                    }
                } break;

                case INTEGER_FORMAT_KEYWORD_LEFT_PAD_COUNT:
                {
                    Size left_pad_count = 0;
                    while (index < format.length)
                    {
                        const char this_char = format.data[index];

                        if ('0' <= this_char && this_char <= '9')
                        {
                            left_pad_count = 10 * left_pad_count + (this_char - '0');
                            index += 1;
                        }
                        else if (this_char == ',')
                        {
                            break;
                        }
                        else
                        {
                            FAIL("Unexpected char found");
                        }
                    }

                    settings.left_pad_count = left_pad_count;
                } break;

                case INTEGER_FORMAT_KEYWORD_LEFT_PAD_CHAR:
                {
                    settings.left_pad_char = format.data[index];
                    index += 1;
                } break;
            }

            while (index < format.length)
            {
                if (format.data[index] == ' ')
                {
                    index += 1;
                }
                else
                {
                    break;
                }
            }

            if (index < format.length && format.data[index] == ',')
            {
                index += 1;
            }
        }
        else
        {
            FAIL("Unexpected char encountered while parsing format specifier");
        }
    }

    return settings;
}

struct Float_Format_Settings
{
    Size left_pad_count;
    Size precision;
    char left_pad_char;
};
typedef struct Float_Format_Settings Float_Format_Settings;

internal Float_Format_Settings
parse_float_format_settings(const String_View format)
{
    enum Float_Format_Keyword_Type
    {
        FLOAT_FORMAT_KEYWORD_INVALID = 0,

        FLOAT_FORMAT_KEYWORD_LEFT_PAD_COUNT,
        FLOAT_FORMAT_KEYWORD_LEFT_PAD_CHAR,
        FLOAT_FORMAT_KEYWORD_PRECISION,
    };

    struct Keyword
    {
        enum Float_Format_Keyword_Type type;
        String_View lexeme;
    };

    const struct Keyword keywords[] = {
        {
            .type = FLOAT_FORMAT_KEYWORD_LEFT_PAD_COUNT,
            .lexeme = string_view("left-pad-count"),
        },
        {
            .type = FLOAT_FORMAT_KEYWORD_LEFT_PAD_CHAR,
            .lexeme = string_view("left-pad-char"),
        },
        {
            .type = FLOAT_FORMAT_KEYWORD_PRECISION,
            .lexeme = string_view("precision"),
        },
    };
    const Size keywords_count = size_of(keywords) / size_of(keywords[0]);

    Float_Format_Settings settings = {0};
    settings.left_pad_count = 0;
    settings.left_pad_char = ' ';
    settings.precision = 2;

    Index index = 0;
    while (index < format.length)
    {
        Index lexeme_start_index = index;

        const char c = format.data[index];

        if (c == ' ')
        {
            index += 1;
            continue;
        }

        // TODO(vlad): Use 'is_letter'.
        if ('a' <= c && c <= 'z')
        {
            index += 1;
            while (index < format.length)
            {
                const char this_char = format.data[index];

                const Bool is_valid_lexeme_symbol =
                    ('a' <= this_char && this_char <= 'z')
                    || (this_char == '-');

                if (!is_valid_lexeme_symbol)
                {
                    break;
                }

                index += 1;
            }

            String_View lexeme = {0};
            lexeme.data = format.data + lexeme_start_index;
            lexeme.length = index - lexeme_start_index;

            enum Float_Format_Keyword_Type type = FLOAT_FORMAT_KEYWORD_INVALID;
            for (Index keyword_index = 0;
                 keyword_index < keywords_count;
                 ++keyword_index)
            {
                const struct Keyword* this_keyword = &keywords[keyword_index];
                if (strings_are_equal(lexeme, this_keyword->lexeme))
                {
                    type = this_keyword->type;
                    break;
                }
            }

            if (type == FLOAT_FORMAT_KEYWORD_INVALID)
            {
                FAIL("Invalid integer format specifier found");
            }

            if (index >= format.length)
            {
                FAIL("Unexpected end of format settings found");
            }

            if (format.data[index] != ':')
            {
                FAIL("Unexpected char found (expected ':')");
            }

            index += 1;

            while (index < format.length)
            {
                if (format.data[index] == ' ')
                {
                    index += 1;
                }
                else
                {
                    break;
                }
            }

            if (index >= format.length)
            {
                FAIL("Unexpected end of format settings found");
            }

            switch (type)
            {
                case FLOAT_FORMAT_KEYWORD_INVALID:
                {
                    UNREACHABLE();
                } break;

                case FLOAT_FORMAT_KEYWORD_LEFT_PAD_COUNT:
                {
                    Size left_pad_count = 0;
                    while (index < format.length)
                    {
                        const char this_char = format.data[index];

                        if ('0' <= this_char && this_char <= '9')
                        {
                            left_pad_count = 10 * left_pad_count + (this_char - '0');
                            index += 1;
                        }
                        else if (this_char == ',')
                        {
                            break;
                        }
                        else
                        {
                            FAIL("Unexpected char found");
                        }
                    }

                    settings.left_pad_count = left_pad_count;
                } break;

                case FLOAT_FORMAT_KEYWORD_LEFT_PAD_CHAR:
                {
                    settings.left_pad_char = format.data[index];
                    index += 1;
                } break;

                case FLOAT_FORMAT_KEYWORD_PRECISION:
                {
                    Size precision = 0;
                    while (index < format.length)
                    {
                        const char this_char = format.data[index];

                        if ('0' <= this_char && this_char <= '9')
                        {
                            precision = 10 * precision + (this_char - '0');
                            index += 1;
                        }
                        else if (this_char == ',')
                        {
                            break;
                        }
                        else
                        {
                            FAIL("Unexpected char found");
                        }
                    }

                    settings.precision = precision;
                } break;
            }

            while (index < format.length)
            {
                if (format.data[index] == ' ')
                {
                    index += 1;
                }
                else
                {
                    break;
                }
            }

            if (index < format.length && format.data[index] == ',')
            {
                index += 1;
            }
        }
        else
        {
            FAIL("Unexpected char encountered while parsing format specifier");
        }
    }

    return settings;
}

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
    // FIXME(vlad): Grow the buffer dynamically.
    char* buffer = allocate_uninitialized_array(arena, 2 * formatted_string_length, char);
    Index buffer_index = 0;

    Bool brace_was_opened = false;
    Index format_specification_start_index = 0;
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
                format_specification_start_index = i + 1;
            } break;

            case '}':
            {
                if (!brace_was_opened)
                {
                    buffer[buffer_index++] = c;
                    break;
                }

                String_View format_specification = {0};
                format_specification.data = format.data + format_specification_start_index;
                format_specification.length = i - format_specification_start_index;

                Format_Type_Info info = va_arg(args, Format_Type_Info);

                switch (info.tag)
                {
                    case TYPE_TAG_STRING_VIEW:
                    {
                        if (format_specification.length != 0)
                        {
                            FAIL("Strings format specification is not yet supported");
                        }

                        copy_memory(as_bytes(buffer + buffer_index),
                                    as_bytes(info.argument.string_view.data),
                                    info.argument.string_view.length);
                        buffer_index += info.argument.string_view.length;
                    } break;

                    case TYPE_TAG_char:
                    {
                        if (format_specification.length != 0)
                        {
                            FAIL("Char format specification is not yet supported");
                        }

                        buffer[buffer_index] = info.argument.char_value;
                        buffer_index += 1;
                    } break;

#define HANDLE_INTEGER_CASE(Integer_Type)                               \
                    case TYPE_TAG_##Integer_Type:                       \
                    {                                                   \
                        const Integer_Format_Settings settings = parse_integer_format_settings(format_specification); \
                                                                        \
                        const Size target_length = info.max_size_in_bytes; \
                        String target = {0};                            \
                        target.data = allocate_array(arena, target_length, char); \
                        target.length = target_length;                  \
                                                                        \
                        Integer_Type##_to_string_inplace(&target,       \
                                                         info.argument.Integer_Type##_value, \
                                                         settings.base); \
                                                                        \
                        if (target.length < settings.left_pad_count)    \
                        {                                               \
                            for (Index pad_index = 0;                   \
                                 pad_index < settings.left_pad_count - target.length; \
                                 ++pad_index)                           \
                            {                                           \
                                buffer[buffer_index] = settings.left_pad_char; \
                                buffer_index += 1;                      \
                            }                                           \
                        }                                               \
                        copy_memory(as_bytes(buffer + buffer_index),    \
                                    as_bytes(target.data),              \
                                    target.length);                     \
                        buffer_index += target.length;                  \
                    } break;                                            \

                    FOR_EACH_INTEGER_TYPE(HANDLE_INTEGER_CASE);
#undef HANDLE_INTEGER_CASE

#define HANDLE_FLOAT_CASE(Float_Type)                                   \
                    case TYPE_TAG_##Float_Type:                         \
                    {                                                   \
                        const Float_Format_Settings settings = parse_float_format_settings(format_specification); \
                                                                        \
                        const Size target_length = info.max_size_in_bytes; \
                        String target = {0};                            \
                        target.data = allocate_array(arena, target_length, char); \
                        target.length = target_length;                  \
                                                                        \
                        Float_Type##_to_string_inplace(&target,         \
                                                       info.argument.Float_Type##_value, \
                                                       settings.precision); \
                                                                        \
                        if (target.length < settings.left_pad_count)    \
                        {                                               \
                            for (Index pad_index = 0;                   \
                                 pad_index < settings.left_pad_count - target.length; \
                                 ++pad_index)                           \
                            {                                           \
                                buffer[buffer_index] = settings.left_pad_char; \
                                buffer_index += 1;                      \
                            }                                           \
                        }                                               \
                        copy_memory(as_bytes(buffer + buffer_index),    \
                                    as_bytes(target.data),              \
                                    target.length);                     \
                        buffer_index += target.length;                  \
                    } break;                                            \

                    FOR_EACH_FLOAT_TYPE(HANDLE_FLOAT_CASE);
#undef HANDLE_FLOAT_CASE
                }

                brace_was_opened = false;
            } break;

            default:
            {
                if (!brace_was_opened)
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

        const Size format_specification_length = format.length - format_specification_start_index;
        copy_memory(as_bytes(buffer + buffer_index),
                    as_bytes(buffer + format_specification_start_index),
                    format_specification_length);
        buffer_index += format_specification_length;
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
