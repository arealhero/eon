#include "eon_types.h"

#include "eon_builtin_types.h"
#include "eon_lexical_scopes.h"

#include <eon/string.h>

enum
{
    UNDEFINED_TYPE_INDEX = 0,
    INVALID_TYPE_INDEX = 1,
};

internal inline Bool
type_id_is_defined(const Type_Id type_id)
{
    return !type_id_is_undefined(type_id);
}

internal inline Bool
type_id_is_undefined(const Type_Id type_id)
{
    return type_id.index == UNDEFINED_TYPE_INDEX;
}

internal inline Bool
type_id_is_valid(Compilation_Context* context, const Type_Id type_id)
{
    return type_id_is_defined(type_id) && !type_id_is_invalid(context, type_id);
}

internal inline Bool
type_id_is_invalid(Compilation_Context* context, const Type_Id type_id)
{
    const Type_Id root_type_id = find_root_type_id(context, type_id);
    return root_type_id.index == INVALID_TYPE_INDEX;
}

internal inline Type_Id
get_void_type_id(Compilation_Context* context)
{
    const Builtin_Type void_builtin_type = VOID_BUILTIN_TYPE;
    const Symbol_Id void_symbol_id = find_symbol_id(context,
                                                    GLOBAL_LEXICAL_SCOPE_ID,
                                                    string_view(void_builtin_type.name));
    const Symbol* void_symbol = get_symbol_by_id(context, void_symbol_id);
    return void_symbol->type_id;
}

internal inline Type_Id
get_boolean_type_id(Compilation_Context* context)
{
    const Builtin_Type boolean_builtin_type = BOOLEAN_BUILTIN_TYPE;
    const Symbol_Id boolean_symbol_id = find_symbol_id(context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       string_view(boolean_builtin_type.name));
    const Symbol* boolean_symbol = get_symbol_by_id(context, boolean_symbol_id);
    return boolean_symbol->type_id;
}

internal Type_Id
create_new_type_variable(Compilation_Context* context)
{
    const Type_Id type_variable_id = create_type(context);

    Type* type_variable = get_type_by_id(context, type_variable_id);
    type_variable->kind = TYPE_VARIABLE;

    return type_variable_id;
}

internal Bool variable_type_is_valid(Compilation_Context* context,
                                     const Type_Id variable_type_id,
                                     const Source_Location* variable_location);

internal void
bind_type_id_to_a_symbol(Compilation_Context* context,
                         const Symbol_Id symbol_id,
                         const Type_Id type_id)
{
    ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

    Symbol* symbol = get_symbol_by_id(context, symbol_id);

    if (!variable_type_is_valid(context, type_id, &symbol->location))
    {
        symbol->type_id.index = INVALID_TYPE_INDEX;
    }
    else
    {
        symbol->type_id = type_id;
    }
}

internal void
bind_type_id_to_a_builtin_symbol(Compilation_Context* context,
                                 const C_String type_name,
                                 const Type_Id type_id)
{
    ASSERT(type_id_is_valid(context, type_id));

    const Symbol_Id symbol_id = find_symbol_id(context,
                                               GLOBAL_LEXICAL_SCOPE_ID,
                                               string_view(type_name));
    ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

    Symbol* symbol = &context->symbols[symbol_id];
    ASSERT(symbol->kind == SYMBOL_TYPE && symbol->is_builtin);
    symbol->type_id = type_id;
}

internal void
create_builtin_types(Compilation_Context* context)
{
    {
        const Type_Id undefined_type_id = create_type(context);
        ASSERT(undefined_type_id.index == UNDEFINED_TYPE_INDEX);

        Type* undefined_type = get_type_by_id(context, undefined_type_id);
        undefined_type->kind = TYPE_UNDEFINED;
    }

    {
        const Type_Id invalid_type_id = create_type(context);
        ASSERT(invalid_type_id.index == INVALID_TYPE_INDEX);

        Type* invalid_type = get_type_by_id(context, invalid_type_id);
        invalid_type->kind = TYPE_INVALID;
    }

    {
        const Type_Id void_type_id = create_type(context);
        ASSERT(type_id_is_valid(context, void_type_id));

        Type* void_type = get_type_by_id(context, void_type_id);
        void_type->kind = TYPE_VOID;

        const Builtin_Type void_builtin_type = VOID_BUILTIN_TYPE;
        bind_type_id_to_a_builtin_symbol(context, void_builtin_type.name, void_type_id);
    }

    {
        const Type_Id bool_type_id = create_type(context);
        ASSERT(type_id_is_valid(context, bool_type_id));

        Type* bool_type = get_type_by_id(context, bool_type_id);
        bool_type->kind = TYPE_BOOLEAN;

        const Builtin_Type boolean_builtin_type = BOOLEAN_BUILTIN_TYPE;
        bind_type_id_to_a_builtin_symbol(context, boolean_builtin_type.name, bool_type_id);
    }

    const Integer_Builtin_Type integer_builtin_types[] = INTEGER_BUILTIN_TYPES;
    for (Index i = 0;
         i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(integer_builtin_types);
         ++i)
    {
        const Integer_Builtin_Type* builtin_type = &integer_builtin_types[i];

        const Type_Id integer_type_id = create_type(context);
        ASSERT(type_id_is_valid(context, integer_type_id));

        Type* integer_type = get_type_by_id(context, integer_type_id);
        integer_type->kind = TYPE_INTEGER;
        integer_type->integer_info.width_in_bits = builtin_type->width_in_bits;
        integer_type->integer_info.is_signed = builtin_type->is_signed;

        bind_type_id_to_a_builtin_symbol(context, builtin_type->name, integer_type_id);
    }

    const Float_Builtin_Type float_builtin_types[] = FLOAT_BUILTIN_TYPES;
    for (Index i = 0;
         i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(float_builtin_types);
         ++i)
    {
        const Float_Builtin_Type* builtin_type = &float_builtin_types[i];

        const Type_Id float_type_id = create_type(context);
        ASSERT(type_id_is_valid(context, float_type_id));

        Type* float_type = get_type_by_id(context, float_type_id);
        float_type->kind = TYPE_FLOAT;
        float_type->float_info.width_in_bits = builtin_type->width_in_bits;

        bind_type_id_to_a_builtin_symbol(context, builtin_type->name, float_type_id);
    }
}

internal u64
get_max_integer_value(const Integer_Type_Info* type_info)
{
    if (type_info->is_signed)
    {
        switch (type_info->width_in_bits)
        {
            case 8:  return MAX_VALUE(s8);
            case 16: return MAX_VALUE(s16);
            case 32: return MAX_VALUE(s32);
            case 64: return MAX_VALUE(s64);

            default:
            {
                FAIL("[TYPE] Unexpected integer width encountered.");
            } break;
        }
    }
    else
    {
        switch (type_info->width_in_bits)
        {
            case 8:  return MAX_VALUE(s8);
            case 16: return MAX_VALUE(s16);
            case 32: return MAX_VALUE(s32);
            case 64: return MAX_VALUE(s64);

            default:
            {
                FAIL("[TYPE] Unexpected integer width encountered.");
            } break;
        }
    }
}

// FIXME(vlad): Add Source_Location that triggered this unification.
internal Bool
try_to_unify_types(Compilation_Context* context,
                   const Type_Id lhs_type_id,
                   const Type_Id rhs_type_id)
{
    ASSERT(type_id_is_defined(lhs_type_id));
    ASSERT(type_id_is_defined(rhs_type_id));

    const Type_Id lhs_root_type_id = find_root_type_id(context, lhs_type_id);
    const Type_Id rhs_root_type_id = find_root_type_id(context, rhs_type_id);

    if (lhs_root_type_id.index == rhs_root_type_id.index)
    {
        return true;
    }

    Type* lhs_root_type = get_type_by_id(context, lhs_root_type_id);
    Type* rhs_root_type = get_type_by_id(context, rhs_root_type_id);

    if (type_id_is_invalid(context, lhs_root_type_id) || type_id_is_invalid(context, rhs_root_type_id))
    {
        return true;
    }

    if (lhs_root_type->kind == TYPE_VARIABLE)
    {
        lhs_root_type->parent_type_id = rhs_root_type_id;
        return true;
    }

    if (rhs_root_type->kind == TYPE_VARIABLE)
    {
        rhs_root_type->parent_type_id = lhs_root_type_id;
        return true;
    }

    if (lhs_root_type->kind == TYPE_NUMBER_VARIABLE && rhs_root_type->kind == TYPE_NUMBER_VARIABLE)
    {
        Number_Constraints* lhs_constraints = &lhs_root_type->number_constraints;
        Number_Constraints* rhs_constraints = &rhs_root_type->number_constraints;

        Type unified_type = {0};
        unified_type.kind = TYPE_NUMBER_VARIABLE;

        Number_Constraints* unified_constraints = &unified_type.number_constraints;

        if (lhs_constraints->sign == SIGN_UNKNOWN)
        {
            unified_constraints->sign = rhs_constraints->sign;
        }
        else if (rhs_constraints->sign == SIGN_UNKNOWN)
        {
            unified_constraints->sign = lhs_constraints->sign;
        }
        else if (lhs_constraints->sign == rhs_constraints->sign)
        {
            unified_constraints->sign = lhs_constraints->sign;
        }
        else
        {
            return false;
        }

        unified_constraints->must_be_a_floating_point_number = lhs_constraints->must_be_a_floating_point_number
                                                               || rhs_constraints->must_be_a_floating_point_number;

        unified_constraints->min_bit_width = MAX(lhs_constraints->min_bit_width, rhs_constraints->min_bit_width);

        const Type_Id unified_type_id = create_type(context);
        Type* created_type = get_type_by_id(context, unified_type_id);
        *created_type = unified_type;

        // NOTE(vlad): Fetching root types again because they can be invalidated after the 'create_type()' function call.
        lhs_root_type = get_type_by_id(context, lhs_root_type_id);
        rhs_root_type = get_type_by_id(context, rhs_root_type_id);
        lhs_root_type->parent_type_id = unified_type_id;
        rhs_root_type->parent_type_id = unified_type_id;

        return true;
    }
    else if (lhs_root_type->kind == TYPE_NUMBER_VARIABLE || rhs_root_type->kind == TYPE_NUMBER_VARIABLE)
    {
        const Type_Id other_type_id = lhs_root_type->kind != TYPE_NUMBER_VARIABLE ? lhs_type_id : rhs_type_id;

        Type* number_variable_type = lhs_root_type->kind == TYPE_NUMBER_VARIABLE ? lhs_root_type : rhs_root_type;
        Type* other_type           = lhs_root_type->kind != TYPE_NUMBER_VARIABLE ? lhs_root_type : rhs_root_type;

        if (other_type->kind == TYPE_INTEGER)
        {
            if (number_variable_type->number_constraints.must_be_a_floating_point_number)
            {
                return false;
            }

            const u64 max_value = get_max_integer_value(&other_type->integer_info);
            if (number_variable_type->number_constraints.integer_value > max_value)
            {
                return false;
            }

            // TODO(vlad): Should we support negative numbers here? I guess not because number lexemes are unsigned
            //             due to the unary minus is being treated as a separate unary operation and not as a
            //             part of a number literal.
        }
        else if (other_type->kind == TYPE_FLOAT)
        {
            // TODO(vlad): Test 'min_bit_width' here? But we don't fill this field in for floats.
        }
        else
        {
            return false;
        }

        number_variable_type->parent_type_id = other_type_id;
        return true;
    }

    // NOTE(vlad): Both types are concrete so their kinds should match.
    if (lhs_root_type->kind != rhs_root_type->kind)
    {
        // XXX(vlad): Pass some information to the callee?
        return false;
    }

    switch (lhs_root_type->kind)
    {
        case TYPE_UNDEFINED:
        case TYPE_INVALID:
        {
            UNREACHABLE();
        } break;

        case TYPE_VOID:
        {
            return true;
        } break;

        case TYPE_VARIABLE:
        case TYPE_NUMBER_VARIABLE:
        {
            // NOTE(vlad): Type variables were handled earlier in this function.
            UNREACHABLE();
        } break;

        case TYPE_INTEGER:
        {
            return lhs_root_type->integer_info.is_signed == rhs_root_type->integer_info.is_signed
                && lhs_root_type->integer_info.width_in_bits == rhs_root_type->integer_info.width_in_bits;
        } break;

        case TYPE_FLOAT:
        {
            return lhs_root_type->float_info.width_in_bits == rhs_root_type->float_info.width_in_bits;
        } break;

        case TYPE_BOOLEAN:
        {
            return true;
        } break;

        case TYPE_POINTER:
        {
            const Pointer_Type_Info* lhs_pointer_info = &lhs_root_type->pointer_info;
            const Pointer_Type_Info* rhs_pointer_info = &rhs_root_type->pointer_info;

            if (lhs_pointer_info->pointee_is_mutable && !rhs_pointer_info->pointee_is_mutable)
            {
                return false;
            }

            return try_to_unify_types(context,
                                      lhs_pointer_info->points_to_type_id,
                                      rhs_pointer_info->points_to_type_id);
        } break;

        case TYPE_FUNCTION:
        {
            FAIL("[TYPE] Function types unification is not supported yet");
        } break;
    }

    return true;
}

internal Bool
types_are_equal(Compilation_Context* context,
                const Type_Id lhs_type_id,
                const Type_Id rhs_type_id)
{
    ASSERT(type_id_is_defined(lhs_type_id));
    ASSERT(type_id_is_defined(rhs_type_id));

    const Type_Id lhs_root_type_id = find_root_type_id(context, lhs_type_id);
    const Type_Id rhs_root_type_id = find_root_type_id(context, rhs_type_id);

    if (lhs_root_type_id.index == rhs_root_type_id.index)
    {
        return true;
    }

    if (type_id_is_invalid(context, lhs_root_type_id) || type_id_is_invalid(context, rhs_root_type_id))
    {
        // TODO(vlad): Change to 'true'?
        return false;
    }

    const Type* lhs_root_type = get_type_by_id(context, lhs_root_type_id);
    const Type* rhs_root_type = get_type_by_id(context, rhs_root_type_id);

    if (lhs_root_type->kind != rhs_root_type->kind)
    {
        return false;
    }

    switch (lhs_root_type->kind)
    {
        case TYPE_UNDEFINED:
        case TYPE_INVALID:
        {
            UNREACHABLE();
        } break;

        case TYPE_VOID:
        {
            return true;
        } break;

        case TYPE_VARIABLE:
        {
            return true;
        } break;

        case TYPE_NUMBER_VARIABLE:
        {
            const Number_Constraints* lhs_constraints = &lhs_root_type->number_constraints;
            const Number_Constraints* rhs_constraints = &rhs_root_type->number_constraints;

            return lhs_constraints->must_be_a_floating_point_number == rhs_constraints->must_be_a_floating_point_number
                && lhs_constraints->min_bit_width == rhs_constraints->min_bit_width
                && lhs_constraints->sign == rhs_constraints->sign;
        } break;

        case TYPE_INTEGER:
        {
            return lhs_root_type->integer_info.is_signed == rhs_root_type->integer_info.is_signed
                && lhs_root_type->integer_info.width_in_bits == rhs_root_type->integer_info.width_in_bits;
        } break;

        case TYPE_FLOAT:
        {
            return lhs_root_type->float_info.width_in_bits == rhs_root_type->float_info.width_in_bits;
        } break;

        case TYPE_BOOLEAN:
        {
            return true;
        } break;

        case TYPE_POINTER:
        {
            const Pointer_Type_Info* lhs_pointer_info = &lhs_root_type->pointer_info;
            const Pointer_Type_Info* rhs_pointer_info = &rhs_root_type->pointer_info;

            if (lhs_pointer_info->pointee_is_mutable && !rhs_pointer_info->pointee_is_mutable)
            {
                return false;
            }

            return types_are_equal(context,
                                   lhs_pointer_info->points_to_type_id,
                                   rhs_pointer_info->points_to_type_id);
        } break;

        case TYPE_FUNCTION:
        {
            FAIL("[TYPE] Function types equality test is not supported yet");
        } break;
    }

    return true;
}

internal Bool
variable_type_is_valid(Compilation_Context* context,
                       const Type_Id variable_type_id,
                       const Source_Location* variable_location)
{
    ASSERT(type_id_is_defined(variable_type_id));

    if (type_id_is_invalid(context, variable_type_id))
    {
        return false;
    }

    const Type_Id void_type_id = get_void_type_id(context);

    if (types_are_equal(context, variable_type_id, void_type_id))
    {
        Diagnostic_Message error = {0};
        error.level = MESSAGE_LEVEL_ERROR;
        error.location = *variable_location;
        error.text = string_view("Variable cannot have a 'void' type");
        emit_diagnostic_message(context, &error);

        return false;
    }

    const Type* variable_type = get_type_by_id(context, variable_type_id);
    if (variable_type->kind == TYPE_POINTER
        && types_are_equal(context, variable_type->pointer_info.points_to_type_id, void_type_id))
    {
        Diagnostic_Message error = {0};
        error.level = MESSAGE_LEVEL_ERROR;
        error.location = *variable_location;
        error.text = string_view("Variable cannot have a '* void' type, use '* byte' instead");
        emit_diagnostic_message(context, &error);

        return false;
    }

    return true;
}

internal Type_Id
resolve_type_by_ast_type(Compilation_Context* context,
                         Ast_Type* ast_type)
{
    switch (ast_type->kind)
    {
        case AST_TYPE_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_TYPE_NAME:
        {
            ASSERT(ast_type->symbol_id != UNDEFINED_SYMBOL_ID && ast_type->symbol_id != INVALID_SYMBOL_ID);

            Symbol* symbol = get_symbol_by_id(context, ast_type->symbol_id);
            if (symbol->kind == SYMBOL_WILDCARD)
            {
                const Type_Id type_id = create_new_type_variable(context);
                ast_type->type_id = type_id;
            }
            else if (symbol->kind == SYMBOL_TYPE)
            {
                ASSERT(type_id_is_valid(context, symbol->type_id));
                ast_type->type_id = symbol->type_id;
            }
            else
            {
                UNREACHABLE();
            }
        } break;

        case AST_TYPE_POINTER:
        {
            Ast_Pointer_Type* ast_pointer_type = &ast_type->pointer;

            const Type_Id points_to_type_id = resolve_type_by_ast_type(context,
                                                                       ast_pointer_type->pointed_to);

            const Type_Id pointer_type_id = create_type(context);

            Type* pointer_type = get_type_by_id(context, pointer_type_id);
            pointer_type->kind = TYPE_POINTER;
            pointer_type->pointer_info.points_to_type_id = points_to_type_id;
            pointer_type->pointer_info.pointee_is_mutable = ast_pointer_type->pointed_to->is_mutable;

            ast_type->type_id = pointer_type_id;
        } break;

        case AST_TYPE_FUNCTION:
        {
            Ast_Function_Type* ast_function_type = &ast_type->function;

            Type_Id* parameter_type_ids = NULL;

            if (ast_function_type->parameters_count != 0)
            {
                parameter_type_ids = allocate_array(context->parameter_type_ids_arena,
                                                    ast_function_type->parameters_count,
                                                    Type_Id);

                for (Index parameter_index = 0;
                     parameter_index < ast_function_type->parameters_count;
                     ++parameter_index)
                {
                    Ast_Function_Parameter* parameter = &ast_function_type->parameters[parameter_index];

                    const Type_Id parameter_type_id = resolve_type_by_ast_type(context, parameter->type);
                    bind_type_id_to_a_symbol(context, parameter->name.symbol_id, parameter_type_id);

                    parameter_type_ids[parameter_index] = parameter_type_id;
                }
            }

            const Type_Id return_type_id = resolve_type_by_ast_type(context,
                                                                    ast_function_type->return_type);

            const Type_Id function_type_id = create_type(context);
            Type* function_type = get_type_by_id(context, function_type_id);
            function_type->kind = TYPE_FUNCTION;

            Function_Type_Info* type_info = &function_type->function_info;
            type_info->parameter_type_ids = parameter_type_ids;
            type_info->parameter_type_ids_count = ast_function_type->parameters_count;
            type_info->return_type_id = return_type_id;

            ast_type->type_id = function_type_id;
        } break;

        case AST_TYPE_OMITTED:
        {
            const Type_Id type_id = create_new_type_variable(context);
            ast_type->type_id = type_id;
        } break;
    }

    ASSERT(type_id_is_defined(ast_type->type_id));

    {
        Type* type = get_type_by_id(context, ast_type->type_id);
        switch (type->kind)
        {
            case TYPE_UNDEFINED:
            case TYPE_INVALID:
            {
                UNREACHABLE();
            } break;

            case TYPE_VARIABLE:
            case TYPE_NUMBER_VARIABLE:
            case TYPE_INTEGER:
            case TYPE_FLOAT:
            case TYPE_BOOLEAN:
            case TYPE_POINTER:
            {
                // NOTE(vlad): These types can be marked as mutable.
            } break;

            case TYPE_VOID:
            case TYPE_FUNCTION:
            {
                if (ast_type->is_mutable)
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = ast_type->location;

                    String_View type_name = {0};
                    switch (type->kind)
                    {
                        case TYPE_VOID:
                        {
                            type_name = string_view("void");
                        } break;

                        case TYPE_FUNCTION:
                        {
                            type_name = string_view("function");
                        } break;

                        default:
                        {
                            UNREACHABLE();
                        }
                    }

                    const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                            "Type '{}' cannot be mutable.",
                                                            type_name);

                    error.text = string_view(error_text);
                    emit_diagnostic_message(context, &error);

                    ast_type->type_id.index = INVALID_TYPE_INDEX;
                }
            } break;
        }
    }

    return ast_type->type_id;
}

struct Expression_Result
{
    Type_Id type_id;
    Bool is_lvalue;
    Bool is_mutable;
};
typedef struct Expression_Result Expression_Result;

internal Expression_Result
resolve_types_in_expression(Compilation_Context* context, Ast_Expression* expression)
{
    Expression_Result result = {0};

    switch (expression->kind)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        {
            Ast_Number* number = &expression->number;

            const Type_Id number_type_id = create_type(context);

            Type* expression_type = get_type_by_id(context, number_type_id);
            expression_type->kind = TYPE_NUMBER_VARIABLE;

            Number_Constraints* constraints = &expression_type->number_constraints;

            if (number->is_a_floating_point_number)
            {
                constraints->must_be_a_floating_point_number = true;
            }
            else
            {
                constraints->must_be_a_floating_point_number = false;
                constraints->sign = SIGN_UNKNOWN;

                u64 value;
                if (!parse_integer(number->token.lexeme, &value))
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = expression->location;

                    const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                            "Number '{}' requires more than 64 bits to store it.",
                                                            number->token.lexeme);

                    error.text = string_view(error_text);
                    emit_diagnostic_message(context, &error);

                    expression->type_id.index = INVALID_TYPE_INDEX;
                    result.type_id.index = INVALID_TYPE_INDEX;
                    return result;
                }

                constraints->integer_value = value;

                if (value > MAX_VALUE(s64))
                {
                    // NOTE(vlad): We know that the only u64 can fit this value, but it might as well be a
                    //             floating-point number thus we are not inferring the concrete type here.
                    constraints->min_bit_width = 64;
                }
                else if (value > MAX_VALUE(u32))
                {
                    constraints->min_bit_width = 64;
                }
                else if (value > MAX_VALUE(u16))
                {
                    constraints->min_bit_width = 32;
                }
                else if (value > MAX_VALUE(u8))
                {
                    constraints->min_bit_width = 16;
                }
            }

            expression->type_id = number_type_id;
            result.type_id = number_type_id;
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("[TYPE] String literals are not supported yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            Ast_Identifier* identifier = &expression->identifier;
            Symbol* symbol = get_symbol_for_identifier(context, identifier);
            ASSERT(type_id_is_defined(symbol->type_id));

            expression->type_id = symbol->type_id;
            result.type_id = symbol->type_id;
            result.is_lvalue = true;
            result.is_mutable = symbol->binding_is_mutable;
        } break;

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        {
            Ast_Binary_Expression* comparison = &expression->binary_expression;

            const Expression_Result lhs_result = resolve_types_in_expression(context, comparison->lhs);
            const Expression_Result rhs_result = resolve_types_in_expression(context, comparison->rhs);

            String_View operation = {0};
            switch (expression->kind)
            {
                case AST_EXPRESSION_ADD:
                {
                    operation = string_view("add");
                } break;

                case AST_EXPRESSION_SUBTRACT:
                {
                    operation = string_view("subtract");
                } break;

                case AST_EXPRESSION_MULTIPLY:
                {
                    operation = string_view("multiply");
                } break;

                case AST_EXPRESSION_DIVIDE:
                {
                    operation = string_view("divide");
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            if (!try_to_unify_types(context, lhs_result.type_id, rhs_result.type_id))
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = expression->location;

                const String_View lhs_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                           context,
                                                                           lhs_result.type_id);

                const String_View rhs_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                           context,
                                                                           rhs_result.type_id);

                const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Cannot {} expressions of different types '{}' and '{}'",
                                                        operation,
                                                        lhs_type_string,
                                                        rhs_type_string);

                error.text = string_view(error_text);
                emit_diagnostic_message(context, &error);

                expression->type_id.index = INVALID_TYPE_INDEX;
                result.type_id.index = INVALID_TYPE_INDEX;
                return result;
            }

            const Type* expression_type = get_type_by_id(context, lhs_result.type_id);

            switch (expression_type->kind)
            {
                case TYPE_UNDEFINED:
                case TYPE_INVALID:
                {
                    UNREACHABLE();
                } break;

                case TYPE_INTEGER:
                case TYPE_FLOAT:
                case TYPE_BOOLEAN:
                case TYPE_NUMBER_VARIABLE:
                {
                    // NOTE(vlad): These types can be added/subtracted/multiplied/divided.
                    // TODO(vlad): Think about number variables: maybe we can add constraints here?
                } break;

                case TYPE_VOID:
                case TYPE_VARIABLE:
                case TYPE_POINTER:
                case TYPE_FUNCTION:
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = expression->location;

                    const String_View lhs_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                               context,
                                                                               lhs_result.type_id);

                    const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                            "Cannot {} expressions of type '{}'",
                                                            operation,
                                                            lhs_type_string);

                    error.text = string_view(error_text);
                    emit_diagnostic_message(context, &error);

                    expression->type_id.index = INVALID_TYPE_INDEX;
                    result.type_id.index = INVALID_TYPE_INDEX;
                    return result;
                } break;
            }

            expression->type_id = lhs_result.type_id;
            result.type_id = lhs_result.type_id;
        } break;

        case AST_EXPRESSION_EQUAL:
        case AST_EXPRESSION_NOT_EQUAL:
        case AST_EXPRESSION_LESS:
        case AST_EXPRESSION_LESS_OR_EQUAL:
        case AST_EXPRESSION_GREATER:
        case AST_EXPRESSION_GREATER_OR_EQUAL:
        {
            Ast_Binary_Expression* comparison = &expression->binary_expression;

            const Expression_Result lhs_result = resolve_types_in_expression(context, comparison->lhs);
            const Expression_Result rhs_result = resolve_types_in_expression(context, comparison->rhs);

            if (!try_to_unify_types(context, lhs_result.type_id, rhs_result.type_id))
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = expression->location;

                const String_View lhs_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                           context,
                                                                           lhs_result.type_id);

                const String_View rhs_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                           context,
                                                                           rhs_result.type_id);

                const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Cannot compare expressions of different types '{}' and '{}'",
                                                        lhs_type_string,
                                                        rhs_type_string);

                error.text = string_view(error_text);
                emit_diagnostic_message(context, &error);

                expression->type_id.index = INVALID_TYPE_INDEX;
                result.type_id.index = INVALID_TYPE_INDEX;
                return result;
            }

            const Type* expression_type = get_type_by_id(context, lhs_result.type_id);

            switch (expression_type->kind)
            {
                case TYPE_UNDEFINED:
                case TYPE_INVALID:
                {
                    UNREACHABLE();
                } break;

                case TYPE_INTEGER:
                case TYPE_FLOAT:
                case TYPE_BOOLEAN:
                case TYPE_NUMBER_VARIABLE:
                {
                    // NOTE(vlad): These types can be compared.
                    // TODO(vlad): Forbid 'less/greater than' comparisons for booleans.
                    // TODO(vlad): Think about number variables: maybe we can add constraints here?
                } break;

                case TYPE_VOID:
                case TYPE_VARIABLE:
                case TYPE_POINTER:
                case TYPE_FUNCTION:
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = expression->location;

                    const String_View lhs_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                               context,
                                                                               lhs_result.type_id);

                    const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                            "Expressions of type '{}' cannot be compared.",
                                                            lhs_type_string);

                    error.text = string_view(error_text);
                    emit_diagnostic_message(context, &error);

                    expression->type_id.index = INVALID_TYPE_INDEX;
                    result.type_id.index = INVALID_TYPE_INDEX;
                    return result;
                } break;
            }

            const Type_Id boolean_type_id = get_boolean_type_id(context);
            expression->type_id = boolean_type_id;
            result.type_id = boolean_type_id;
        } break;

        case AST_EXPRESSION_NEGATE:
        {
            FAIL("[TYPE] Negations are not supported yet");
        } break;

        case AST_EXPRESSION_DEREFERENCE:
        {
            Ast_Unary_Expression* dereference = &expression->unary_expression;
            ASSERT(strings_are_equal(dereference->operator.lexeme, string_view("*")));

            const Expression_Result expression_result = resolve_types_in_expression(context, dereference->operand);
            ASSERT(type_id_is_defined(expression_result.type_id));

            if (type_id_is_invalid(context, expression_result.type_id))
            {
                expression->type_id.index = INVALID_TYPE_INDEX;
                result.type_id.index = INVALID_TYPE_INDEX;
                return result;
            }

            const Type* expression_type = get_type_by_id(context, expression_result.type_id);
            if (expression_type->kind != TYPE_POINTER)
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = dereference->operand->location;

                const String_View expression_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                  context,
                                                                                  expression_result.type_id);

                const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Cannot dereference a non-pointer type '{}'",
                                                        expression_type_string);

                error.text = string_view(error_text);
                emit_diagnostic_message(context, &error);

                expression->type_id.index = INVALID_TYPE_INDEX;
                result.type_id.index = INVALID_TYPE_INDEX;
                return result;
            }

            const Type_Id dereferenced_type_id = expression_type->pointer_info.points_to_type_id;

            expression->type_id = dereferenced_type_id;
            result.type_id = dereferenced_type_id;
            result.is_lvalue = true;
            result.is_mutable = expression_type->pointer_info.pointee_is_mutable;
        } break;

        case AST_EXPRESSION_ADDRESS_OF:
        {
            Ast_Unary_Expression* address_of = &expression->unary_expression;

            const Expression_Result points_to_result = resolve_types_in_expression(context, address_of->operand);
            if (!points_to_result.is_lvalue)
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = address_of->operand->location;
                error.text = string_view("Cannot take address of a non-lvalue expression");
                emit_diagnostic_message(context, &error);

                expression->type_id.index = INVALID_TYPE_INDEX;
                result.type_id.index = INVALID_TYPE_INDEX;
                return result;
            }

            const Type_Id pointer_type_id = create_type(context);
            Type* pointer_type = get_type_by_id(context, pointer_type_id);
            pointer_type->kind = TYPE_POINTER;
            pointer_type->pointer_info.points_to_type_id = points_to_result.type_id;
            pointer_type->pointer_info.pointee_is_mutable = points_to_result.is_mutable;

            expression->type_id = pointer_type_id;
            result.type_id = pointer_type_id;
        } break;

        case AST_EXPRESSION_CALL:
        {
            Ast_Call* call = &expression->call;

            const Expression_Result called_result = resolve_types_in_expression(context, call->called_expression);
            ASSERT(type_id_is_valid(context, called_result.type_id));

            const Type* called_type = get_type_by_id(context, called_result.type_id);

            if (called_type->kind != TYPE_FUNCTION)
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = call->called_expression->location;

                const String_View expression_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                  context,
                                                                                  called_result.type_id);

                const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Expression of type '{}' is not callable",
                                                        expression_type_string);

                error.text = string_view(error_text);
                emit_diagnostic_message(context, &error);

                expression->type_id.index = INVALID_TYPE_INDEX;
                result.type_id.index = INVALID_TYPE_INDEX;
                return result;
            }

            const Function_Type_Info* function_info = &called_type->function_info;

            if (call->arguments_count != function_info->parameter_type_ids_count)
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = call->called_expression->location;

                const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Too many arguments provided to a function call, expected {}, got {}",
                                                        function_info->parameter_type_ids_count,
                                                        call->arguments_count);

                error.text = string_view(error_text);
                emit_diagnostic_message(context, &error);

                expression->type_id.index = INVALID_TYPE_INDEX;
                result.type_id.index = INVALID_TYPE_INDEX;
                return result;
            }

            const Type_Id return_type_id = function_info->return_type_id;
            const Type_Id* parameter_type_ids = function_info->parameter_type_ids;

            // NOTE(vlad): These pointers will be invalidated during the following for-loop execution.
            function_info = NULL;
            called_type = NULL;

            for (Index parameter_index = 0;
                 parameter_index < call->arguments_count;
                 ++parameter_index)
            {
                Ast_Expression* parameter = call->arguments[parameter_index];

                const Expression_Result parameter_result = resolve_types_in_expression(context, parameter);
                const Type_Id expected_type_id = parameter_type_ids[parameter_index];

                if (!try_to_unify_types(context, expected_type_id, parameter_result.type_id))
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = parameter->location;

                    const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                  context,
                                                                                  parameter_result.type_id);
                    const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                    context,
                                                                                    expected_type_id);

                    const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                            "Passing expression of type '{}' to parameter of type '{}'",
                                                            actual_type_string,
                                                            expected_type_string);

                    error.text = string_view(error_text);
                    emit_diagnostic_message(context, &error);

                    expression->type_id.index = INVALID_TYPE_INDEX;
                    result.type_id.index = INVALID_TYPE_INDEX;
                    return result;
                }
            }

            expression->type_id = return_type_id;
            result.type_id = return_type_id;
        } break;
    }

    ASSERT(type_id_is_defined(expression->type_id));
    ASSERT(type_id_is_defined(result.type_id));
    return result;
}

internal Bool
resolve_types_in_code_block(Compilation_Context* context,
                            Ast_Code_Block* code_block,
                            const Type_Id expected_return_type_id)
{
    for (Index statement_index = 0;
         statement_index < code_block->statements_count;
         ++statement_index)
    {
        Ast_Statement* statement = &code_block->statements[statement_index];

        switch (statement->kind)
        {
            case AST_STATEMENT_UNDEFINED:
            {
                UNREACHABLE();
            } break;

            case AST_STATEMENT_VARIABLE_DEFINITION:
            {
                Ast_Variable_Definition* definition = &statement->variable_definition;
                Symbol* variable_symbol = get_symbol_for_identifier(context, &definition->name);
                variable_symbol->type_id = resolve_type_by_ast_type(context, definition->type);

                if (definition->has_initial_value)
                {
                    const Expression_Result initial_value = resolve_types_in_expression(context,
                                                                                        &definition->initial_value);

                    if (!try_to_unify_types(context, variable_symbol->type_id, initial_value.type_id))
                    {
                        Diagnostic_Message error = {0};
                        error.level = MESSAGE_LEVEL_ERROR;
                        error.location = definition->initial_value.location;

                        const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                        context,
                                                                                        variable_symbol->type_id);
                        const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                      context,
                                                                                      initial_value.type_id);

                        const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                                "Cannot initialise variable of type '{}' with expression of type '{}'",
                                                                expected_type_string,
                                                                actual_type_string);

                        error.text = string_view(error_text);
                        emit_diagnostic_message(context, &error);
                        return false;
                    }
                }
                else
                {
                    Type* type = get_type_by_id(context, variable_symbol->type_id);
                    ASSERT(type->kind != TYPE_VARIABLE);
                }

                bind_type_id_to_a_symbol(context, definition->name.symbol_id, variable_symbol->type_id);
            } break;

            case AST_STATEMENT_ASSIGNMENT:
            {
                Ast_Assignment* assignment = &statement->assignment;

                const Expression_Result lhs = resolve_types_in_expression(context, &assignment->lhs);
                const Expression_Result rhs = resolve_types_in_expression(context, &assignment->rhs);

                if (!try_to_unify_types(context, lhs.type_id, rhs.type_id))
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = assignment->rhs.location;

                    const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                    context,
                                                                                    lhs.type_id);
                    const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                  context,
                                                                                  rhs.type_id);

                    const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                            "Type mismatch: expected '{}', got '{}'",
                                                            expected_type_string,
                                                            actual_type_string);

                    error.text = string_view(error_text);
                    emit_diagnostic_message(context, &error);
                    return false;
                }

                if (!lhs.is_lvalue)
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = assignment->lhs.location;
                    error.text = string_view("lvalue is required as a LHS of assignment");
                    emit_diagnostic_message(context, &error);
                    return false;
                }

                if (!lhs.is_mutable)
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = assignment->lhs.location;
                    error.text = string_view("Read-only location is not assignable");
                    emit_diagnostic_message(context, &error);
                    return false;
                }
            } break;

            case AST_STATEMENT_RETURN:
            {
                Ast_Return_Statement* return_statement = &statement->return_statement;

                Expression_Result return_result = {0};
                if (return_statement->is_empty)
                {
                    return_result.type_id = get_void_type_id(context);
                }
                else
                {
                    return_result = resolve_types_in_expression(context, &return_statement->expression);
                }

                if (type_id_is_valid(context, return_result.type_id))
                {
                    if (!try_to_unify_types(context, return_result.type_id, expected_return_type_id))
                    {
                        Diagnostic_Message error = {0};
                        error.level = MESSAGE_LEVEL_ERROR;

                        if (return_statement->is_empty)
                        {
                            error.location = return_statement->empty_expression_location;
                        }
                        else
                        {
                            error.location = return_statement->expression.location;
                        }

                        const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                        context,
                                                                                        expected_return_type_id);
                        const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                      context,
                                                                                      return_result.type_id);

                        const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                                "Return type mismatch: expected '{}', got '{}'",
                                                                expected_type_string,
                                                                actual_type_string);

                        error.text = string_view(error_text);
                        emit_diagnostic_message(context, &error);

                        return false;
                    }
                }

                if (type_id_is_undefined(code_block->return_type_id))
                {
                    code_block->return_type_id = return_result.type_id;
                }
                else
                {
                    ASSERT(try_to_unify_types(context, code_block->return_type_id, return_result.type_id));
                }

                code_block->every_path_returns = true;
            } break;

            case AST_STATEMENT_WHILE:
            {
                Ast_While_Statement* while_statement = &statement->while_statement;

                {
                    Ast_Expression* condition = &while_statement->condition;

                    const Expression_Result condition_result = resolve_types_in_expression(context, condition);
                    const Type_Id boolean_type_id = get_boolean_type_id(context);

                    if (!try_to_unify_types(context, condition_result.type_id, boolean_type_id))
                    {
                        Diagnostic_Message error = {0};
                        error.level = MESSAGE_LEVEL_ERROR;

                        error.location = condition->location;

                        const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                        context,
                                                                                        boolean_type_id);
                        const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                      context,
                                                                                      condition_result.type_id);

                        const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                                "Implicit conversion from '{}' to '{}' is forbidden.",
                                                                actual_type_string,
                                                                expected_type_string);

                        error.text = string_view(error_text);
                        emit_diagnostic_message(context, &error);

                        return false;
                    }
                }

                Ast_Code_Block* body = &while_statement->body;
                if (!resolve_types_in_code_block(context, body, expected_return_type_id))
                {
                    return false;
                }

                if (body->every_path_returns)
                {
                    ASSERT(try_to_unify_types(context, body->return_type_id, expected_return_type_id));
                    code_block->every_path_returns = true;
                    code_block->return_type_id = expected_return_type_id;
                }
            } break;

            case AST_STATEMENT_IF:
            {
                Ast_If_Statement* if_statement = &statement->if_statement;

                {
                    Ast_Expression* condition = &if_statement->condition;
                    const Expression_Result condition_result = resolve_types_in_expression(context, condition);
                    const Type_Id boolean_type_id = get_boolean_type_id(context);

                    if (!try_to_unify_types(context, condition_result.type_id, boolean_type_id))
                    {
                        Diagnostic_Message error = {0};
                        error.level = MESSAGE_LEVEL_ERROR;

                        error.location = condition->location;

                        const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                        context,
                                                                                        boolean_type_id);
                        const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                      context,
                                                                                      condition_result.type_id);

                        const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                                "Implicit conversion from '{}' to '{}' is forbidden.",
                                                                actual_type_string,
                                                                expected_type_string);

                        error.text = string_view(error_text);
                        emit_diagnostic_message(context, &error);

                        return false;
                    }
                }

                Ast_Code_Block* then_code_block = &if_statement->if_statements;
                Ast_Code_Block* else_code_block = &if_statement->else_statements;

                if (!resolve_types_in_code_block(context, then_code_block, expected_return_type_id))
                {
                    return false;
                }

                if (!resolve_types_in_code_block(context, else_code_block, expected_return_type_id))
                {
                    return false;
                }

                if (then_code_block->every_path_returns)
                {
                    ASSERT(try_to_unify_types(context, then_code_block->return_type_id, expected_return_type_id));
                }

                if (else_code_block->every_path_returns)
                {
                    ASSERT(try_to_unify_types(context, else_code_block->return_type_id, expected_return_type_id));
                }

                if (then_code_block->every_path_returns && else_code_block->every_path_returns)
                {
                    code_block->every_path_returns = true;
                    code_block->return_type_id = expected_return_type_id;
                }
            } break;

            case AST_STATEMENT_CALL:
            {
                Ast_Call_Statement* call_statement = &statement->call_statement;

                const Expression_Result return_result = resolve_types_in_expression(context, &call_statement->call_expression);
                ASSERT(type_id_is_defined(return_result.type_id));

                if (type_id_is_invalid(context, return_result.type_id))
                {
                    return false;
                }

                const Type_Id void_type_id = get_void_type_id(context);
                if (!try_to_unify_types(context, return_result.type_id, void_type_id))
                {
                    // FIXME(vlad): Test if returned value is discardable.
                }
            } break;
        }
    }

    return true;
}

internal Bool
resolve_and_validate_types(Compilation_Context* context)
{
    create_builtin_types(context);

    Ast* ast = &context->ast;

    // NOTE(vlad): Resolving global symbols' types.
    {
        for (Index function_definition_index = 0;
             function_definition_index < context->ast.function_definitions_count;
             ++function_definition_index)
        {
            Ast_Function_Definition* function_definition = &context->ast.function_definitions[function_definition_index];

            ASSERT(function_definition->type->kind == AST_TYPE_FUNCTION);

            const Type_Id function_type_id = resolve_type_by_ast_type(context, function_definition->type);

            function_definition->type->type_id = function_type_id;
            bind_type_id_to_a_symbol(context, function_definition->name.symbol_id, function_type_id);
        }
    }

    for (Index function_definition_index = 0;
         function_definition_index < ast->function_definitions_count;
         ++function_definition_index)
    {
        Ast_Function_Definition* function_definition = &ast->function_definitions[function_definition_index];

        ASSERT(function_definition->type->kind == AST_TYPE_FUNCTION);
        ASSERT(type_id_is_defined(function_definition->type->type_id));

        const Type_Id expected_return_type_id = function_definition->type->function.return_type->type_id;

        Ast_Code_Block* body = &function_definition->body;
        if (!resolve_types_in_code_block(context, body, expected_return_type_id))
        {
            return false;
        }

        if (type_id_is_invalid(context, body->return_type_id))
        {
            return false;
        }

        const Type_Id void_type_id = get_void_type_id(context);

        /* NOTE(vlad): Logic is based on these rules:
         *
         *     | expected | body               | every path returns | result                        |
         *     |----------+--------------------+--------------------+-------------------------------|
         *     | void     | undefined          | -                  | ok                            |
         *     | void     | void               | -                  | ok                            |
         *     | void     | non-void           | -                  | mismatch                      |
         *     |----------+--------------------+--------------------+-------------------------------|
         *     | non-void | undefined          | -                  | non-void should have a return |
         *     | non-void | void               | -                  | mismatch                      |
         *     | non-void | same non-void      | true               | ok                            |
         *     | non-void | same non-void      | false              | non-void should have a return |
         *     | non-void | different non-void | -                  | mismatch                      |
         **/

        // FIXME(vlad): Remove code duplication in diagnostic messages emitting.

        if (types_are_equal(context, expected_return_type_id, void_type_id))
        {
            // NOTE(vlad): Expected return type is void.

            if (type_id_is_undefined(body->return_type_id))
            {
                body->return_type_id = void_type_id;
                body->every_path_returns = true;
            }
            else if (!try_to_unify_types(context, body->return_type_id, void_type_id))
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = body->end_location;

                const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                context,
                                                                                expected_return_type_id);
                const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                              context,
                                                                              body->return_type_id);

                const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Return type mismatch: expected '{}', got '{}'",
                                                        expected_type_string,
                                                        actual_type_string);

                error.text = string_view(error_text);
                emit_diagnostic_message(context, &error);
                return false;
            }
            else
            {
                body->every_path_returns = true;
            }
        }
        else
        {
            // NOTE(vlad): Expected return type is non-void.

            if (type_id_is_undefined(body->return_type_id))
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = body->end_location;
                error.text = string_view("Non-void function does not return a value in all control paths");

                emit_diagnostic_message(context, &error);
                return false;
            }

            if (!try_to_unify_types(context, expected_return_type_id, body->return_type_id))
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = body->end_location;

                const String_View expected_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                                context,
                                                                                expected_return_type_id);
                const String_View actual_type_string = convert_type_to_string(context->diagnostic_message_texts_arena,
                                                                              context,
                                                                              body->return_type_id);

                const String error_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Return type mismatch: expected '{}', got '{}'",
                                                        expected_type_string,
                                                        actual_type_string);

                error.text = string_view(error_text);
                emit_diagnostic_message(context, &error);
                return false;
            }

            if (!body->every_path_returns)
            {
                Diagnostic_Message error = {0};
                error.level = MESSAGE_LEVEL_ERROR;
                error.location = body->end_location;
                error.text = string_view("Non-void function does not return a value in all control paths");

                emit_diagnostic_message(context, &error);
                return false;
            }
        }
    }

    return true;
}

internal String_View
convert_type_to_string(Arena* arena,
                       Compilation_Context* context,
                       const Type_Id type_id)
{
    // FIXME(vlad): Remove unnecessary allocations and copies.

    const Type_Id root_type_id = find_root_type_id(context, type_id);
    const Type* type = get_type_by_id(context, root_type_id);

    String_Builder builder = {0};
    create_string_builder(&builder, arena);

    switch (type->kind)
    {
        case TYPE_UNDEFINED:
        case TYPE_INVALID:
        {
            UNREACHABLE();
        } break;

        case TYPE_VOID:
        {
            append_string(&builder, string_view("void"));
        } break;

        case TYPE_VARIABLE:
        {
            append_string(&builder, string_view("_"));
        } break;

        case TYPE_NUMBER_VARIABLE:
        {
            // TODO(vlad): Create a function that would accept 'Number_Constraints' and return a most suitable type.

            const Number_Constraints* constraints = &type->number_constraints;

            if (constraints->must_be_a_floating_point_number)
            {
                switch (constraints->min_bit_width)
                {
                    case 0:
                    case 32:
                    {
                        append_string(&builder, string_view("f32"));
                    } break;

                    case 64:
                    {
                        append_string(&builder, string_view("f64"));
                    } break;

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }
            }
            else
            {
                if (constraints->sign == SIGN_UNSIGNED)
                {
                    append_string(&builder, string_view("u"));
                }
                else
                {
                    append_string(&builder, string_view("s"));
                }

                switch (constraints->min_bit_width)
                {
                    case 8:
                    {
                        append_string(&builder, string_view("8"));
                    } break;

                    case 16:
                    {
                        append_string(&builder, string_view("16"));
                    } break;

                    case 0:
                    case 32:
                    {
                        append_string(&builder, string_view("32"));
                    } break;

                    case 64:
                    {
                        append_string(&builder, string_view("64"));
                    } break;

                    default:
                    {
                        UNREACHABLE();
                    } break;
                }
            }
        } break;

        case TYPE_INTEGER:
        {
            String result = {0};
            result.data = allocate_uninitialized_array(arena, 3, char);
            result.length = 3;

            const Integer_Type_Info* type_info = &type->integer_info;
            result.data[0] = type_info->is_signed ? 's' : 'u';

            switch (type_info->width_in_bits)
            {
                case 8:
                {
                    result.data[1] = '8';
                    result.length -= 1;
                } break;

                case 16:
                {
                    result.data[1] = '1';
                    result.data[2] = '6';
                } break;

                case 32:
                {
                    result.data[1] = '3';
                    result.data[2] = '2';
                } break;

                case 64:
                {
                    result.data[1] = '6';
                    result.data[2] = '4';
                } break;

                default:
                {
                    FAIL("[TYPE] Unsupported integer width encountered.");
                } break;
            }

            append_string(&builder, string_view(result));
        } break;

        case TYPE_FLOAT:
        {
            const Float_Type_Info* type_info = &type->float_info;

            String result = {0};
            result.data = allocate_uninitialized_array(arena, 3, char);
            result.length = 3;

            result.data[0] = 'f';
            switch (type_info->width_in_bits)
            {
                case 32:
                {
                    result.data[1] = '3';
                    result.data[2] = '2';
                } break;

                case 64:
                {
                    result.data[1] = '6';
                    result.data[2] = '4';
                } break;

                default:
                {
                    FAIL("[TYPE] Unsupported floating-point number width encountered.");
                } break;
            }

            append_string(&builder, string_view(result));
        } break;

        case TYPE_BOOLEAN:
        {
            append_string(&builder, string_view("bool"));
        } break;

        case TYPE_POINTER:
        {
            const Pointer_Type_Info* type_info = &type->pointer_info;

            append_string(&builder, string_view("* "));
            if (type_info->pointee_is_mutable)
            {
                append_string(&builder, string_view("mutable "));
            }

            const String_View points_to_type = convert_type_to_string(arena, context, type_info->points_to_type_id);
            append_string(&builder, points_to_type);
        } break;

        case TYPE_FUNCTION:
        {
            String result = {0};

            const Function_Type_Info* type_info = &type->function_info;

            // NOTE(vlad): Length of '() -> ' is 6.
            Size total_result_length = 6;

            String_View* parameter_types = NULL;

            if (type_info->parameter_type_ids_count != 0)
            {
                total_result_length += 2 * (type_info->parameter_type_ids_count - 1);

                parameter_types = allocate_array(arena,
                                                 type_info->parameter_type_ids_count,
                                                 String_View);
                for (Index i = 0;
                     i < type_info->parameter_type_ids_count;
                     ++i)
                {
                    const Type_Id parameter_type_id = type_info->parameter_type_ids[i];
                    parameter_types[i] = convert_type_to_string(arena,
                                                                context,
                                                                parameter_type_id);

                    total_result_length += parameter_types[i].length;
                }
            }

            const String_View result_type = convert_type_to_string(arena,
                                                                   context,
                                                                   type_info->return_type_id);
            total_result_length += result_type.length;

            result.data = allocate_uninitialized_array(arena, total_result_length, char);
            result.length = total_result_length;

            Index result_index = 0;

            result.data[result_index++] = '(';
            for (Index i = 0;
                 i < type_info->parameter_type_ids_count;
                 ++i)
            {
                copy_memory(as_bytes(result.data) + result_index,
                            as_bytes(parameter_types[i].data),
                            parameter_types[i].length);

                result_index += parameter_types[i].length;

                if (i != type_info->parameter_type_ids_count - 1)
                {
                    result.data[result_index++] = ',';
                    result.data[result_index++] = ' ';
                }
            }

            result.data[result_index++] = ')';
            result.data[result_index++] = ' ';
            result.data[result_index++] = '-';
            result.data[result_index++] = '>';
            result.data[result_index++] = ' ';

            copy_memory(as_bytes(result.data) + result_index,
                        as_bytes(result_type.data),
                        result_type.length);

            append_string(&builder, string_view(result));
        } break;
    }

    return string_builder_to_string(&builder);
}
