#include "eon_lexical_scopes.h"

#include "eon_builtin_types.h"

internal Index
create_new_lexical_scope_with_parent(Compilation_Context* context, const Index parent_scope_index)
{
    grow_array_if_needed(context->lexical_scopes_arena, context->lexical_scopes, Lexical_Scope);

    context->lexical_scopes[context->lexical_scopes_count] = (Lexical_Scope){0};
    context->lexical_scopes_count += 1;

    Lexical_Scope* created_scope = &context->lexical_scopes[context->lexical_scopes_count - 1];
    created_scope->symbol_ids_arena = acquire_arena_from_provider(context->arena_provider,
                                                                  string_view("lexical-scope-symbol-ids"),
                                                                  GiB(1),
                                                                  MiB(1));
    created_scope->parent_lexical_scope_id = parent_scope_index;
    return context->lexical_scopes_count - 1;
}

internal Lexical_Scope_Id
add_symbol_to_lexical_scope(Compilation_Context* context,
                            const Index lexical_scope_index,
                            const Symbol* symbol_to_add)
{
    ASSERT(0 <= lexical_scope_index && lexical_scope_index < context->lexical_scopes_count);

    Lexical_Scope* lexical_scope = &context->lexical_scopes[lexical_scope_index];

    for (Index symbol_id_index = 0;
         symbol_id_index < lexical_scope->symbol_ids_count;
         ++symbol_id_index)
    {
        const Symbol_Id symbol_id = lexical_scope->symbol_ids[symbol_id_index];
        const Symbol* symbol = &context->symbols[symbol_id];

        if (strings_are_equal(symbol->name, symbol_to_add->name))
        {
            FAIL("Symbol already exists in this scope");
            return INVALID_SYMBOL_ID;
        }
    }

    const Symbol_Id this_symbol_id = create_symbol(context);
    context->symbols[this_symbol_id] = *symbol_to_add;

    grow_array_if_needed(lexical_scope->symbol_ids_arena, lexical_scope->symbol_ids, Symbol_Id);

    lexical_scope->symbol_ids[lexical_scope->symbol_ids_count++] = this_symbol_id;

    return this_symbol_id;
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

    if (found_symbol_id == INVALID_SYMBOL_ID)
    {
        const Source_Location error_location = identifier->token.location;
        const String error_message = format_string(context->error_messages_arena,
                                                   "Use of undeclared identifier '{}'",
                                                   identifier->token.lexeme);

        emit_error(context, error_location, string_view(error_message));

        const Symbol_Id unknown_symbol_id = create_symbol(context);
        Symbol* unknown_symbol = get_symbol_by_id(context, unknown_symbol_id);

        unknown_symbol->kind = SYMBOL_UNKNOWN;

        identifier->symbol_id = unknown_symbol_id;
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
    if (found_symbol_id == INVALID_SYMBOL_ID)
    {
        const Source_Location error_location = type->named_type.token.location;
        const String error_message = format_string(context->error_messages_arena,
                                                   "Use of undeclared identifier '{}'",
                                                   type->named_type.token.lexeme);

        emit_error(context, error_location, string_view(error_message));

        const Symbol_Id unknown_symbol_id = create_symbol(context);
        Symbol* unknown_symbol = get_symbol_by_id(context, unknown_symbol_id);

        unknown_symbol->kind = SYMBOL_UNKNOWN;

        type->symbol_id = unknown_symbol_id;
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

        switch (statement->type)
        {
            case AST_STATEMENT_UNDEFINED:
            {
                UNREACHABLE();
            } break;

            case AST_STATEMENT_VARIABLE_DEFINITION:
            {
                Ast_Variable_Definition* definition = &statement->variable_definition;

                Symbol symbol = {0};
                symbol.kind = SYMBOL_VARIABLE;
                symbol.name = definition->name.token.lexeme;
                symbol.type_id = INVALID_TYPE_ID;
                symbol.is_mutable = definition->type->is_mutable;

                const Symbol_Id symbol_id = add_symbol_to_lexical_scope(context,
                                                                        this_lexical_scope_id,
                                                                        &symbol);
                ASSERT(symbol_id != INVALID_SYMBOL_ID);

                definition->name.symbol_id = symbol_id;

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

                set_symbol_id_for_identifier(context,
                                             &assignment->name,
                                             this_lexical_scope_id);

                set_symbol_ids_for_identifiers_in_expression(context,
                                                             &assignment->expression,
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
                set_symbol_ids_for_identifiers_in_call_expression(context, &call_statement->call, this_lexical_scope_id);
            } break;
        }
    }
}

internal inline void
add_builtin_type_symbol(Compilation_Context* context, const C_String name)
{
    Symbol symbol = {0};
    symbol.kind = SYMBOL_TYPE;
    symbol.name = string_view(name);
    symbol.type_id = INVALID_TYPE_ID;
    symbol.is_mutable = false;

    const Symbol_Id symbol_id = add_symbol_to_lexical_scope(context,
                                                            GLOBAL_LEXICAL_SCOPE_ID,
                                                            &symbol);
    ASSERT(symbol_id != INVALID_SYMBOL_ID);
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
        const Symbol_Id sentinel_symbol_id = create_symbol(context);
        ASSERT(sentinel_symbol_id == INVALID_SYMBOL_ID);

        for (Index i = 0;
             i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(BUILTIN_TYPES);
             ++i)
        {
            const Builtin_Type* type = &BUILTIN_TYPES[i];
            add_builtin_type_symbol(context, type->name);
        }

        for (Index i = 0;
             i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(INTEGER_BUILTIN_TYPES);
             ++i)
        {
            const Integer_Builtin_Type* type = &INTEGER_BUILTIN_TYPES[i];
            add_builtin_type_symbol(context, type->name);
        }

        for (Index i = 0;
             i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(FLOAT_BUILTIN_TYPES);
             ++i)
        {
            const Float_Builtin_Type* type = &FLOAT_BUILTIN_TYPES[i];
            add_builtin_type_symbol(context, type->name);
        }
    }

    // NOTE(vlad): Populating global scope so that the order of definition does not matter.
    {
        for (Index function_definition_index = 0;
             function_definition_index < context->ast.function_definitions_count;
             ++function_definition_index)
        {
            Ast_Function_Definition* function_definition = &context->ast.function_definitions[function_definition_index];

            {
                Symbol symbol = {0};
                symbol.kind = SYMBOL_FUNCTION;
                symbol.name = function_definition->name.token.lexeme;
                symbol.type_id = INVALID_TYPE_ID;
                symbol.is_mutable = function_definition->type->is_mutable;

                const Symbol_Id symbol_id = add_symbol_to_lexical_scope(context,
                                                                        global_scope_id,
                                                                        &symbol);

                ASSERT(symbol_id != INVALID_SYMBOL_ID);

                function_definition->name.symbol_id = symbol_id;
            }
        }
    }

    for (Index function_definition_index = 0;
         function_definition_index < context->ast.function_definitions_count;
         ++function_definition_index)
    {
        Ast_Function_Definition* function_definition = &context->ast.function_definitions[function_definition_index];
        Ast_Code_Block* function_body = &function_definition->body;

        const Index function_scope_index = create_new_lexical_scope_with_parent(context, global_scope_id);
        function_body->lexical_scope_id = function_scope_index;

        ASSERT(function_definition->type->kind == AST_TYPE_FUNCTION);

        set_symbol_ids_for_identifiers_in_type(context, function_definition->type, function_scope_index);

        Ast_Function_Type* function_type = &function_definition->type->function;

        for (Index parameter_index = 0;
             parameter_index < function_type->parameters_count;
             ++parameter_index)
        {
            Ast_Function_Parameter* parameter = &function_type->parameters[parameter_index];

            Symbol symbol = {0};
            symbol.kind = SYMBOL_VARIABLE;
            symbol.name = parameter->name.token.lexeme;
            symbol.type_id = INVALID_TYPE_ID;
            symbol.is_mutable = parameter->type->is_mutable;

            const Symbol_Id symbol_id = add_symbol_to_lexical_scope(context,
                                                                    function_scope_index,
                                                                    &symbol);
            ASSERT(symbol_id != INVALID_SYMBOL_ID);

            parameter->name.symbol_id = symbol_id;
        }

        create_lexical_scopes_for_code_block(context, function_body);
    }
}
