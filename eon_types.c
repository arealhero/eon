#include "eon_types.h"

#include "eon_builtin_types.h"
#include "eon_lexical_scopes.h"

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

internal Type_Id
create_new_type_variable(Compilation_Context* context)
{
    const Type_Id type_variable_id = create_type(context);

    Type* type_variable = get_type_by_id(context, type_variable_id);
    type_variable->kind = TYPE_VARIABLE;
    type_variable->parent_type_id = UNDEFINED_TYPE_ID;

    return type_variable_id;
}

internal void
bind_type_id_to_a_symbol(Compilation_Context* context,
                         const Symbol_Id symbol_id,
                         const Type_Id type_id)
{
    ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);
    ASSERT(type_id != UNDEFINED_TYPE_ID && type_id != INVALID_TYPE_ID);

    Symbol* symbol = &context->symbols[symbol_id];
    symbol->type_id = type_id;
}

internal void
bind_type_id_to_a_builtin_symbol(Compilation_Context* context,
                                 const C_String type_name,
                                 const Type_Id type_id)
{
    ASSERT(type_id != UNDEFINED_TYPE_ID && type_id != INVALID_TYPE_ID);

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
    const Type_Id undefined_type_id = create_type(context);
    ASSERT(undefined_type_id == UNDEFINED_TYPE_ID);

    const Type_Id invalid_type_id = create_type(context);
    ASSERT(invalid_type_id == INVALID_TYPE_ID);

    {
        const Type_Id void_type_id = create_type(context);
        ASSERT(void_type_id != UNDEFINED_TYPE_ID && void_type_id != INVALID_TYPE_ID);

        Type* void_type = get_type_by_id(context, void_type_id);
        void_type->kind = TYPE_VOID;

        const Builtin_Type void_builtin_type = VOID_BUILTIN_TYPE;
        bind_type_id_to_a_builtin_symbol(context, void_builtin_type.name, void_type_id);
    }

    {
        const Type_Id bool_type_id = create_type(context);
        ASSERT(bool_type_id != UNDEFINED_TYPE_ID && bool_type_id != INVALID_TYPE_ID);

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
        ASSERT(integer_type_id != UNDEFINED_TYPE_ID && integer_type_id != INVALID_TYPE_ID);

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
        ASSERT(float_type_id != UNDEFINED_TYPE_ID && float_type_id != INVALID_TYPE_ID);

        Type* float_type = get_type_by_id(context, float_type_id);
        float_type->kind = TYPE_FLOAT;
        float_type->float_info.width_in_bits = builtin_type->width_in_bits;

        bind_type_id_to_a_builtin_symbol(context, builtin_type->name, float_type_id);
    }
}

// FIXME(vlad): Add Source_Location that triggered this unification.
internal Bool
try_to_unify_types(Compilation_Context* context,
                   const Type_Id lhs_type_id,
                   const Type_Id rhs_type_id)
{
    const Type_Id lhs_root_type_id = find_root_type_id(context, lhs_type_id);
    const Type_Id rhs_root_type_id = find_root_type_id(context, rhs_type_id);

    if (lhs_root_type_id == rhs_root_type_id)
    {
        return true;
    }

    Type* lhs_root_type = get_type_by_id(context, lhs_root_type_id);
    Type* rhs_root_type = get_type_by_id(context, rhs_root_type_id);

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

    // NOTE(vlad): Both types are concrete so their kinds should match.
    if (lhs_root_type->kind != rhs_root_type->kind)
    {
        FAIL("Failed to unify types");
    }

    switch (lhs_root_type->kind)
    {
        case TYPE_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case TYPE_VOID:
        {
            return true;
        } break;

        case TYPE_VARIABLE:
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
            return try_to_unify_types(context,
                                      lhs_root_type->pointer_info.points_to_type_id,
                                      rhs_root_type->pointer_info.points_to_type_id);
        } break;

        case TYPE_FUNCTION:
        {
            FAIL("[TYPE] Function types unification is not supported yet");
        } break;
    }

    return true;
}

internal Type_Id
resolve_type_by_ast_type(Compilation_Context* context,
                         Ast_Type* type)
{
    if (type->is_mutable)
    {
        FAIL("[TYPE] mutable types are not supported yet");
    }

    switch (type->kind)
    {
        case AST_TYPE_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_TYPE_NAME:
        {
            ASSERT(type->symbol_id != UNDEFINED_SYMBOL_ID && type->symbol_id != INVALID_SYMBOL_ID);

            Symbol* symbol = get_symbol_by_id(context, type->symbol_id);
            ASSERT(symbol->kind == SYMBOL_TYPE);
            ASSERT(symbol->type_id != UNDEFINED_TYPE_ID && symbol->type_id != INVALID_TYPE_ID);

            type->type_id = symbol->type_id;
            return type->type_id;
        } break;

        case AST_TYPE_POINTER:
        {
            FAIL("[TYPE] Pointers are not yet supported");
        } break;

        case AST_TYPE_FUNCTION:
        {
            // XXX(vlad): Lexical scopes are not available here,
            //            thus we cannot set type ids to the parameter symbols.

            Ast_Function_Type* ast_type = &type->function;

            const Type_Id function_type_id = create_type(context);
            Type* function_type = get_type_by_id(context, function_type_id);
            function_type->kind = TYPE_FUNCTION;

            Function_Type_Info* type_info = &function_type->function_info;

            type_info->parameter_type_ids_count = ast_type->parameters_count;

            if (ast_type->parameters_count != 0)
            {
                type_info->parameter_type_ids = allocate_array(context->parameter_type_ids_arena,
                                                               ast_type->parameters_count,
                                                               Type_Id);
            }

            for (Index parameter_index = 0;
                 parameter_index < ast_type->parameters_count;
                 ++parameter_index)
            {
                Ast_Function_Parameter* parameter = &ast_type->parameters[parameter_index];
                const Type_Id parameter_type_id = resolve_type_by_ast_type(context,
                                                                           parameter->type);

                type_info->parameter_type_ids[parameter_index] = parameter_type_id;
                bind_type_id_to_a_symbol(context, parameter->name.symbol_id, parameter_type_id);
            }

            type_info->return_type_id = resolve_type_by_ast_type(context, ast_type->return_type);

            return function_type_id;
        } break;

        case AST_TYPE_OMITTED:
        {
            const Type_Id type_id = create_new_type_variable(context);
            type->type_id = type_id;
            return type_id;
        } break;
    }

    UNREACHABLE();
}

internal Type_Id
resolve_types_in_expression(Compilation_Context* context, Ast_Expression* expression)
{
    switch (expression->kind)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        {
            FAIL("[TYPE] Numbers are not supported yet");
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("[TYPE] String literals are not supported yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            Ast_Identifier* identifier = &expression->identifier;
            Symbol* symbol = get_symbol_for_identifier(context, identifier);
            ASSERT(symbol->type_id != UNDEFINED_TYPE_ID);
            ASSERT(symbol->type_id != INVALID_TYPE_ID);

            expression->type_id = symbol->type_id;
            return symbol->type_id;
        } break;

        case AST_EXPRESSION_ADD:
        {
            FAIL("[TYPE] Additions are not supported yet");
        } break;

        case AST_EXPRESSION_SUBTRACT:
        {
            FAIL("[TYPE] Subtractions are not supported yet");
        } break;

        case AST_EXPRESSION_MULTIPLY:
        {
            FAIL("[TYPE] Multiplications are not supported yet");
        } break;

        case AST_EXPRESSION_DIVIDE:
        {
            FAIL("[TYPE] Divisions are not supported yet");
        } break;

        case AST_EXPRESSION_EQUAL:
        case AST_EXPRESSION_NOT_EQUAL:
        case AST_EXPRESSION_LESS:
        case AST_EXPRESSION_LESS_OR_EQUAL:
        case AST_EXPRESSION_GREATER:
        case AST_EXPRESSION_GREATER_OR_EQUAL:
        {
            FAIL("[TYPE] Comparisons are not supported yet");
        } break;

        case AST_EXPRESSION_NEGATE:
        {
            FAIL("[TYPE] Negations are not supported yet");
        } break;

        case AST_EXPRESSION_DEREFERENCE:
        {
            FAIL("[TYPE] Dereferences are not supported yet");
        } break;

        case AST_EXPRESSION_ADDRESS_OF:
        {
            FAIL("[TYPE] Address-of expressions are not supported yet");
        } break;

        case AST_EXPRESSION_CALL:
        {
            FAIL("[TYPE] Calls are not supported yet");
        } break;
    }

    UNREACHABLE();
}

internal void
resolve_types_in_code_block(Compilation_Context* context, Ast_Code_Block* code_block)
{
    for (Index statement_index = 0;
         statement_index < code_block->statements_count;
         ++statement_index)
    {
        Ast_Statement* statement = &code_block->statements[statement_index];

        switch (statement->type)
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
                    const Type_Id initial_value_type_id = resolve_types_in_expression(context,
                                                                                      &definition->initial_value);
                    ASSERT(try_to_unify_types(context, variable_symbol->type_id, initial_value_type_id));
                }
                else
                {
                    Type* type = get_type_by_id(context, variable_symbol->type_id);
                    ASSERT(type->kind != TYPE_VARIABLE);
                }
            } break;

            case AST_STATEMENT_ASSIGNMENT:
            {
                FAIL("[TYPE] Assignments are not yet supported");
            } break;

            case AST_STATEMENT_RETURN:
            {
                Ast_Return_Statement* return_statement = &statement->return_statement;

                Type_Id return_type_id = UNDEFINED_TYPE_ID;
                if (return_statement->is_empty)
                {
                    return_type_id = get_void_type_id(context);
                }
                else
                {
                    return_type_id = resolve_types_in_expression(context, &return_statement->expression);
                }

                if (code_block->return_type_id == UNDEFINED_TYPE_ID)
                {
                    code_block->return_type_id = return_type_id;
                }
                else
                {
                    ASSERT(try_to_unify_types(context, code_block->return_type_id, return_type_id));
                }

                code_block->every_path_returns = true;
            } break;

            case AST_STATEMENT_WHILE:
            {
                FAIL("[TYPE] While loops are not yet supported");
            } break;

            case AST_STATEMENT_IF:
            {
                FAIL("[TYPE] If statements are not yet supported");
            } break;

            case AST_STATEMENT_CALL:
            {
                FAIL("[TYPE] Call statements are not yet supported");
            } break;
        }
    }
}

internal Bool
resolve_and_validate_types(Compilation_Context* context)
{
    create_builtin_types(context);

    Ast* ast = &context->ast;

    for (Index function_definition_index = 0;
         function_definition_index < ast->function_definitions_count;
         ++function_definition_index)
    {
        Ast_Function_Definition* function_definition = &ast->function_definitions[function_definition_index];

        ASSERT(function_definition->type->kind == AST_TYPE_FUNCTION);

        const Type_Id function_type_id = resolve_type_by_ast_type(context, function_definition->type);

        function_definition->type->type_id = function_type_id;
        bind_type_id_to_a_symbol(context, function_definition->name.symbol_id, function_type_id);

        Ast_Code_Block* body = &function_definition->body;
        resolve_types_in_code_block(context, body);

        const Type_Id void_type_id = get_void_type_id(context);
        const Type_Id expected_return_type_id = function_definition->type->function.return_type->type_id;

        if (body->return_type_id == UNDEFINED_TYPE_ID)
        {
            body->return_type_id = void_type_id;
            body->every_path_returns = true;
        }

        if (!try_to_unify_types(context,
                                body->return_type_id,
                                expected_return_type_id))
        {
            FAIL("[TYPE] Return types mismatch");
        }

        if (expected_return_type_id != void_type_id && !body->every_path_returns)
        {
            FAIL("[TYPE] Non-void function does not return a value in all control paths");
        }
    }

    return true;
}
