#include "eon_lexical_scopes.h"

#include "eon_builtin_types.h"

internal Index
create_new_lexical_scope_with_parent(Compilation_Context* context, const Index parent_scope_index)
{
    append_array(context->lexical_scopes_arena,
                 context->lexical_scopes,
                 Lexical_Scope,
                 (Lexical_Scope){0});

    Lexical_Scope* created_scope = &context->lexical_scopes[context->lexical_scopes_count - 1];
    created_scope->symbol_ids_arena = acquire_arena_from_provider(context->arena_provider,
                                                                  string_view("lexical-scope-symbol-ids"),
                                                                  GiB(1),
                                                                  MiB(1));
    created_scope->parent_lexical_scope_id = parent_scope_index;
    return context->lexical_scopes_count - 1;
}

internal void
add_symbol_id_to_lexical_scope(Compilation_Context* context,
                               const Index lexical_scope_index,
                               const Symbol_Id symbol_id_to_add)
{
    ASSERT(0 <= lexical_scope_index && lexical_scope_index < context->lexical_scopes_count);

    // TODO(vlad): Forbid shadowing.

    Lexical_Scope* lexical_scope = &context->lexical_scopes[lexical_scope_index];

    Symbol* symbol_to_add = get_symbol_by_id(context, symbol_id_to_add);

    for (Index symbol_id_index = 0;
         symbol_id_index < lexical_scope->symbol_ids_count;
         ++symbol_id_index)
    {
        const Symbol_Id existing_symbol_id = lexical_scope->symbol_ids[symbol_id_index];
        const Symbol* existing_symbol = &context->symbols[existing_symbol_id];

        if (strings_are_equal(existing_symbol->name, symbol_to_add->name))
        {
            const String error_message_text = format_string(context->diagnostic_message_texts_arena,
                                                            "Redefinition of '{}'",
                                                            symbol_to_add->name);

            Diagnostic_Message error = {0};
            error.level = MESSAGE_LEVEL_ERROR;
            error.location = symbol_to_add->location;
            error.text = string_view(error_message_text);

            emit_diagnostic_message(context, &error);

            Diagnostic_Message note = {0};
            note.level = MESSAGE_LEVEL_NOTE;
            note.location = existing_symbol->location;
            note.text = string_view("Previously defined here");

            emit_diagnostic_message(context, &note);

            // TODO(vlad): Propagate this error to the callee?
        }
    }

    append_array(lexical_scope->symbol_ids_arena,
                 lexical_scope->symbol_ids,
                 Symbol_Id,
                 symbol_id_to_add);
}

internal void set_symbol_ids_for_identifiers_in_expression(Compilation_Context* context,
                                                           Ast_Expression* expression,
                                                           const Lexical_Scope_Id this_lexical_scope_id);

internal inline void
set_symbol_id_for_identifier(Compilation_Context* context,
                             Ast_Identifier* identifier,
                             const Lexical_Scope_Id this_lexical_scope_id)
{
    const Symbol_Id found_symbol_id = find_symbol_id(context,
                                                     this_lexical_scope_id,
                                                     identifier->token.lexeme);

    if (found_symbol_id == UNDEFINED_SYMBOL_ID)
    {
        const String error_message_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Use of undeclared identifier '{}'",
                                                        identifier->token.lexeme);

        Diagnostic_Message error = {0};
        error.level = MESSAGE_LEVEL_ERROR;
        error.location = identifier->token.location;
        error.text = string_view(error_message_text);

        emit_diagnostic_message(context, &error);

        identifier->symbol_id = INVALID_SYMBOL_ID;
    }
    else
    {
        identifier->symbol_id = found_symbol_id;
    }
}

internal inline void
set_symbol_id_for_named_type(Compilation_Context* context,
                             Ast_Type* type,
                             const Lexical_Scope_Id this_lexical_scope_id)
{
    ASSERT(type->kind == AST_TYPE_NAME);

    const Symbol_Id found_symbol_id = find_symbol_id(context,
                                                     this_lexical_scope_id,
                                                     type->named_type.token.lexeme);
    if (found_symbol_id == UNDEFINED_SYMBOL_ID)
    {
        const String error_message_text = format_string(context->diagnostic_message_texts_arena,
                                                        "Use of undeclared identifier '{}'",
                                                        type->named_type.token.lexeme);

        Diagnostic_Message error = {0};
        error.level = MESSAGE_LEVEL_ERROR;
        error.location = type->named_type.token.location;
        error.text = string_view(error_message_text);

        emit_diagnostic_message(context, &error);

        type->symbol_id = INVALID_SYMBOL_ID;
    }
    else
    {
        type->symbol_id = found_symbol_id;
    }
}

internal inline void
set_symbol_ids_for_identifiers_in_call_expression(Compilation_Context* context,
                                                  Ast_Call* call,
                                                  const Lexical_Scope_Id this_lexical_scope_id)
{
    set_symbol_ids_for_identifiers_in_expression(context, call->called_expression, this_lexical_scope_id);

    for (Index argument_index = 0;
         argument_index < call->arguments_count;
         ++argument_index)
    {
        Ast_Expression* argument = call->arguments[argument_index];
        set_symbol_ids_for_identifiers_in_expression(context, argument, this_lexical_scope_id);
    }
}

internal void
set_symbol_ids_for_identifiers_in_type(Compilation_Context* context,
                                       Ast_Type* type,
                                       const Lexical_Scope_Id this_lexical_scope_id)
{
    switch (type->kind)
    {
        case AST_TYPE_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_TYPE_NAME:
        {
            set_symbol_id_for_named_type(context, type, this_lexical_scope_id);
        } break;

        case AST_TYPE_POINTER:
        {
            set_symbol_ids_for_identifiers_in_type(context,
                                                   type->pointer.pointed_to,
                                                   this_lexical_scope_id);
        } break;

        case AST_TYPE_FUNCTION:
        {
            Ast_Function_Type* function = &type->function;

            for (Index parameter_index = 0;
                 parameter_index < function->parameters_count;
                 ++parameter_index)
            {
                Ast_Function_Parameter* parameter = &function->parameters[parameter_index];

                // TODO(vlad): Should we create symbols here? I mean, it looks like a perfectly reasonable place
                //             to do that, but the function name becomes kind of misleading.
                {
                    const Symbol_Id symbol_id = create_symbol(context);
                    Symbol* symbol = get_symbol_by_id(context, symbol_id);

                    symbol->kind = SYMBOL_VARIABLE;
                    symbol->name = parameter->name.token.lexeme;
                    symbol->location = parameter->name.token.location;
                    symbol->binding_is_mutable = parameter->type->is_mutable;

                    parameter->name.symbol_id = symbol_id;
                }

                set_symbol_ids_for_identifiers_in_type(context,
                                                       parameter->type,
                                                       this_lexical_scope_id);

            }

            set_symbol_ids_for_identifiers_in_type(context,
                                                   function->return_type,
                                                   this_lexical_scope_id);
        } break;

        case AST_TYPE_OMITTED:
        {
        } break;
    }
}

internal void
set_symbol_ids_for_identifiers_in_expression(Compilation_Context* context,
                                             Ast_Expression* expression,
                                             const Lexical_Scope_Id this_lexical_scope_id)
{
    switch (expression->kind)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        case AST_EXPRESSION_STRING_LITERAL:
        {
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            Ast_Identifier* identifier = &expression->identifier;
            set_symbol_id_for_identifier(context, identifier, this_lexical_scope_id);
        } break;

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        case AST_EXPRESSION_EQUAL:
        case AST_EXPRESSION_NOT_EQUAL:
        case AST_EXPRESSION_LESS:
        case AST_EXPRESSION_LESS_OR_EQUAL:
        case AST_EXPRESSION_GREATER:
        case AST_EXPRESSION_GREATER_OR_EQUAL:
        {
            Ast_Binary_Expression* binary_expression = &expression->binary_expression;
            set_symbol_ids_for_identifiers_in_expression(context, binary_expression->lhs, this_lexical_scope_id);
            set_symbol_ids_for_identifiers_in_expression(context, binary_expression->rhs, this_lexical_scope_id);
        } break;

        case AST_EXPRESSION_NEGATE:
        case AST_EXPRESSION_DEREFERENCE:
        case AST_EXPRESSION_ADDRESS_OF:
        {
            Ast_Unary_Expression* unary_expression = &expression->unary_expression;
            set_symbol_ids_for_identifiers_in_expression(context, unary_expression->operand, this_lexical_scope_id);
        } break;

        case AST_EXPRESSION_CALL:
        {
            Ast_Call* call = &expression->call;
            set_symbol_ids_for_identifiers_in_call_expression(context, call, this_lexical_scope_id);
        } break;
    }
}

internal void
create_lexical_scopes_for_code_block(Compilation_Context* context, Ast_Code_Block* code_block)
{
    const Lexical_Scope_Id this_lexical_scope_id = code_block->lexical_scope_id;
    ASSERT(this_lexical_scope_id != INVALID_LEXICAL_SCOPE_ID);

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

                const Symbol_Id symbol_id = create_symbol(context);
                ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

                definition->name.symbol_id = symbol_id;

                {
                    Symbol* symbol = get_symbol_by_id(context, symbol_id);
                    symbol->kind = SYMBOL_VARIABLE;
                    symbol->name = definition->name.token.lexeme;
                    symbol->location = definition->name.token.location;
                    symbol->binding_is_mutable = definition->type->is_mutable;
                }

                add_symbol_id_to_lexical_scope(context, this_lexical_scope_id, symbol_id);
                set_symbol_ids_for_identifiers_in_type(context, definition->type, this_lexical_scope_id);

                if (definition->has_initial_value)
                {
                    set_symbol_ids_for_identifiers_in_expression(context,
                                                                 &definition->initial_value,
                                                                 this_lexical_scope_id);
                }
            } break;

            case AST_STATEMENT_ASSIGNMENT:
            {
                Ast_Assignment* assignment = &statement->assignment;

                set_symbol_ids_for_identifiers_in_expression(context,
                                                             &assignment->lhs,
                                                             this_lexical_scope_id);
                set_symbol_ids_for_identifiers_in_expression(context,
                                                             &assignment->rhs,
                                                             this_lexical_scope_id);
            } break;

            case AST_STATEMENT_RETURN:
            {
                Ast_Return_Statement* return_statement = &statement->return_statement;

                if (!return_statement->is_empty)
                {
                    set_symbol_ids_for_identifiers_in_expression(context,
                                                                 &return_statement->expression,
                                                                 this_lexical_scope_id);
                }
            } break;

            case AST_STATEMENT_WHILE:
            {
                Ast_While_Statement* while_statement = &statement->while_statement;

                set_symbol_ids_for_identifiers_in_expression(context, &while_statement->condition, this_lexical_scope_id);

                Ast_Code_Block* body = &while_statement->body;
                const Lexical_Scope_Id new_scope_id = create_new_lexical_scope_with_parent(context, this_lexical_scope_id);
                body->lexical_scope_id = new_scope_id;

                create_lexical_scopes_for_code_block(context, body);
            } break;

            case AST_STATEMENT_IF:
            {
                Ast_If_Statement* if_statement = &statement->if_statement;

                set_symbol_ids_for_identifiers_in_expression(context, &if_statement->condition, this_lexical_scope_id);

                Ast_Code_Block* then_code_block = &if_statement->if_statements;
                const Lexical_Scope_Id then_scope_id = create_new_lexical_scope_with_parent(context, this_lexical_scope_id);
                then_code_block->lexical_scope_id = then_scope_id;

                create_lexical_scopes_for_code_block(context, then_code_block);

                Ast_Code_Block* else_code_block = &if_statement->else_statements;
                const Lexical_Scope_Id else_scope_id = create_new_lexical_scope_with_parent(context, this_lexical_scope_id);
                else_code_block->lexical_scope_id = else_scope_id;

                create_lexical_scopes_for_code_block(context, else_code_block);
            } break;

            case AST_STATEMENT_CALL:
            {
                Ast_Call_Statement* call_statement = &statement->call_statement;
                set_symbol_ids_for_identifiers_in_call_expression(context,
                                                                  &call_statement->call_expression.call,
                                                                  this_lexical_scope_id);
            } break;

            case AST_STATEMENT_BREAK:
            case AST_STATEMENT_CONTINUE:
            {
            } break;
        }
    }
}

internal inline void
add_builtin_type_symbol(Compilation_Context* context, const C_String name)
{
    const Symbol_Id symbol_id = create_symbol(context);
    ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

    {
        Symbol* symbol = get_symbol_by_id(context, symbol_id);
        symbol->kind = SYMBOL_TYPE;
        symbol->name = string_view(name);
        symbol->binding_is_mutable = false;
        symbol->is_builtin = true;
    }

    add_symbol_id_to_lexical_scope(context, GLOBAL_LEXICAL_SCOPE_ID, symbol_id);
}

internal inline void
add_builtin_variable_symbol(Compilation_Context* context, const C_String name)
{
    const Symbol_Id symbol_id = create_symbol(context);
    ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

    {
        Symbol* symbol = get_symbol_by_id(context, symbol_id);
        symbol->kind = SYMBOL_VARIABLE;
        symbol->name = string_view(name);
        symbol->binding_is_mutable = false;
        symbol->is_builtin = true;
    }

    add_symbol_id_to_lexical_scope(context, GLOBAL_LEXICAL_SCOPE_ID, symbol_id);
}

internal void
create_lexical_scopes(Compilation_Context* context)
{
    const Lexical_Scope_Id sentinel_scope_id = create_new_lexical_scope_with_parent(context, INVALID_LEXICAL_SCOPE_ID);
    ASSERT(sentinel_scope_id == INVALID_LEXICAL_SCOPE_ID);

    const Lexical_Scope_Id global_scope_id = create_new_lexical_scope_with_parent(context, INVALID_LEXICAL_SCOPE_ID);
    ASSERT(global_scope_id == GLOBAL_LEXICAL_SCOPE_ID);

    // NOTE(vlad): Populating global scope with builtin types.
    {
        const Symbol_Id undefined_symbol_id = create_symbol(context);
        ASSERT(undefined_symbol_id == UNDEFINED_SYMBOL_ID);

        const Symbol_Id invalid_symbol_id = create_symbol(context);
        ASSERT(invalid_symbol_id == INVALID_SYMBOL_ID);

        // NOTE(vlad): Creating symbol for a placeholder.
        {
            const Symbol_Id symbol_id = create_symbol(context);
            ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

            {
                Symbol* symbol = get_symbol_by_id(context, symbol_id);
                symbol->kind = SYMBOL_WILDCARD;
                symbol->name = string_view("_");
                symbol->binding_is_mutable = false;
                symbol->is_builtin = true;
            }

            add_symbol_id_to_lexical_scope(context, GLOBAL_LEXICAL_SCOPE_ID, symbol_id);
        }

        const Builtin_Type void_builtin_type = VOID_BUILTIN_TYPE;
        add_builtin_type_symbol(context, void_builtin_type.name);

        const Builtin_Type boolean_builtin_type = BOOLEAN_BUILTIN_TYPE;
        add_builtin_type_symbol(context, boolean_builtin_type.name);

        const Integer_Builtin_Type integer_builtin_types[] = INTEGER_BUILTIN_TYPES;
        for (Index i = 0;
             i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(integer_builtin_types);
             ++i)
        {
            const Integer_Builtin_Type* type = &integer_builtin_types[i];
            add_builtin_type_symbol(context, type->name);
        }

        const Float_Builtin_Type float_builtin_types[] = FLOAT_BUILTIN_TYPES;
        for (Index i = 0;
             i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(float_builtin_types);
             ++i)
        {
            const Float_Builtin_Type* type = &float_builtin_types[i];
            add_builtin_type_symbol(context, type->name);
        }
    }

    // NOTE(vlad): Populating global scope with builtin constants.
    {
        add_builtin_variable_symbol(context, "true");
        add_builtin_variable_symbol(context, "false");
    }

    // NOTE(vlad): Populating global scope so that the order of definition does not matter.
    {
        for (Index function_definition_index = 0;
             function_definition_index < context->ast.function_definitions_count;
             ++function_definition_index)
        {
            Ast_Function_Definition* function_definition = &context->ast.function_definitions[function_definition_index];

            // FIXME(vlad): Remove this code duplication.
            {
                const Symbol_Id symbol_id = create_symbol(context);
                ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

                function_definition->name.symbol_id = symbol_id;

                {
                    Symbol* symbol = get_symbol_by_id(context, symbol_id);
                    symbol->kind = SYMBOL_FUNCTION;
                    symbol->name = function_definition->name.token.lexeme;
                    symbol->location = function_definition->name.token.location;
                    symbol->binding_is_mutable = function_definition->type->is_mutable;
                }

                add_symbol_id_to_lexical_scope(context, global_scope_id, symbol_id);
            }
        }
    }

    for (Index function_definition_index = 0;
         function_definition_index < context->ast.function_definitions_count;
         ++function_definition_index)
    {
        Ast_Function_Definition* function_definition = &context->ast.function_definitions[function_definition_index];
        Ast_Code_Block* function_body = &function_definition->body;

        const Lexical_Scope_Id function_scope_id = create_new_lexical_scope_with_parent(context, global_scope_id);
        function_body->lexical_scope_id = function_scope_id;

        ASSERT(function_definition->type->kind == AST_TYPE_FUNCTION);

        set_symbol_ids_for_identifiers_in_type(context, function_definition->type, function_scope_id);

        Ast_Function_Type* function_type = &function_definition->type->function;

        for (Index parameter_index = 0;
             parameter_index < function_type->parameters_count;
             ++parameter_index)
        {
            Ast_Function_Parameter* parameter = &function_type->parameters[parameter_index];

            const Symbol_Id symbol_id = create_symbol(context);
            ASSERT(symbol_id != UNDEFINED_SYMBOL_ID && symbol_id != INVALID_SYMBOL_ID);

            parameter->name.symbol_id = symbol_id;

            {
                Symbol* symbol = get_symbol_by_id(context, symbol_id);
                symbol->kind = SYMBOL_VARIABLE;
                symbol->name = parameter->name.token.lexeme;
                symbol->location = parameter->name.token.location;
                symbol->binding_is_mutable = parameter->type->is_mutable;
            }

            add_symbol_id_to_lexical_scope(context, function_scope_id, symbol_id);
        }

        create_lexical_scopes_for_code_block(context, function_body);
    }
}
