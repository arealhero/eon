#include "eon_semantics.h"

internal Index
create_new_lexical_scope_with_parent(Arena* lexical_scopes_arena, Ast* ast, const Index parent_scope_index)
{
    if (ast->lexical_scopes_count == ast->lexical_scopes_capacity)
    {
        // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
        const Size new_capacity = MAX(1, 2 * ast->lexical_scopes_capacity);
        ast->lexical_scopes = reallocate(lexical_scopes_arena,
                                         ast->lexical_scopes,
                                         Lexical_Scope,
                                         ast->lexical_scopes_capacity,
                                         new_capacity);
        ast->lexical_scopes_capacity = new_capacity;
    }

    ast->lexical_scopes[ast->lexical_scopes_count] = (Lexical_Scope){0};
    ast->lexical_scopes_count += 1;

    Lexical_Scope* created_scope = &ast->lexical_scopes[ast->lexical_scopes_count - 1];
    created_scope->parent_scope_index = parent_scope_index;
    created_scope->required_return_type.type = AST_TYPE_UNSPECIFIED;

    return ast->lexical_scopes_count - 1;
}

internal Bool
add_variable_to_lexical_scope(Arena* lexical_scopes_arena, // TODO(vlad): Use separate arena for
                                                           //             every scope's variables?
                              Ast* ast,
                              const Index lexical_scope_index,
                              Ast_Variable_Definition* variable)
{
    Lexical_Scope* lexical_scope = &ast->lexical_scopes[lexical_scope_index];

    for (Index variable_index = 0;
         variable_index < lexical_scope->variables_count;
         ++variable_index)
    {
        const Ast_Variable_Definition* existing_variable = lexical_scope->variables[variable_index];
        if (strings_are_equal(existing_variable->name.token.lexeme,
                              variable->name.token.lexeme))
        {
            FAIL("Variable already exists in this scope");
            return false;
        }
    }

    if (lexical_scope->variables_count == lexical_scope->variables_capacity)
    {
        // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
        const Size new_capacity = MAX(1, 2 * lexical_scope->variables_capacity);
        lexical_scope->variables = reallocate(lexical_scopes_arena,
                                              lexical_scope->variables,
                                              Ast_Variable_Definition*,
                                              lexical_scope->variables_capacity,
                                              new_capacity);
        lexical_scope->variables_capacity = new_capacity;
    }

    lexical_scope->variables[lexical_scope->variables_count] = variable;
    lexical_scope->variables_count += 1;

    if (variable->type->type == AST_TYPE_UNSPECIFIED)
    {
        FAIL("Trying to add unspecified variable to the current scope");
    }

    return true;
}

internal Ast_Variable_Definition*
find_variable(Ast* ast,
              const Index lexical_scope_index,
              const String_View name)
{
    Index next_scope_index = lexical_scope_index;
    while (next_scope_index != LAST_LEXICAL_SCOPE_INDEX)
    {
        Lexical_Scope* scope = &ast->lexical_scopes[next_scope_index];

        for (Index variable_index = 0;
             variable_index < scope->variables_count;
             ++variable_index)
        {
            Ast_Variable_Definition* variable = scope->variables[variable_index];
            if (strings_are_equal(variable->name.token.lexeme, name))
            {
                return variable;
            }
        }

        next_scope_index = scope->parent_scope_index;
    }

    return NULL;
}

internal Ast_Function_Definition*
find_function(Ast* ast, const String_View name)
{
    // TODO(vlad): Use lexical scopes here as well.

    for (Index function_definition_index = 0;
         function_definition_index < ast->function_definitions_count;
         ++function_definition_index)
    {
        Ast_Function_Definition* function_definition = &ast->function_definitions[function_definition_index];

        if (strings_are_equal(function_definition->name.token.lexeme, name))
        {
            return function_definition;
        }
    }

    return NULL;
}

internal Bool
types_are_equal(const Ast_Type* lhs, const Ast_Type* rhs)
{
    if (lhs->type == AST_TYPE_UNSPECIFIED || rhs->type == AST_TYPE_UNSPECIFIED)
    {
        FAIL("Failed to compare types: one of them is unspecified");
        return false;
    }

    if (lhs->type != rhs->type)
    {
        return false;
    }

    switch (lhs->type)
    {
        case AST_TYPE_UNDEFINED:
        case AST_TYPE_UNSPECIFIED:
        {
            UNREACHABLE();
        } break;

        // NOTE(vlad): Trivial types.
        case AST_TYPE_VOID:
        case AST_TYPE_BOOL:
        case AST_TYPE_INT_32:
        case AST_TYPE_FLOAT_32:
        {
            return true;
        } break;

        case AST_TYPE_USER_DEFINED:
        {
            FAIL("User-defined types are not supported yet");
        } break;

        case AST_TYPE_FUNCTION:
        {
            const Ast_Function_Arguments* lhs_arguments = &lhs->arguments;
            const Ast_Function_Arguments* rhs_arguments = &rhs->arguments;

            if (lhs_arguments->arguments_count != rhs_arguments->arguments_count)
            {
                return false;
            }

            for (Index argument_index = 0;
                 argument_index < lhs_arguments->arguments_count;
                 ++argument_index)
            {
                if (!types_are_equal(lhs->return_type, rhs->return_type))
                {
                    return false;
                }
            }

            return types_are_equal(lhs->return_type, rhs->return_type);
        } break;

        case AST_TYPE_POINTER:
        {
            return types_are_equal(lhs->pointed_to, rhs->pointed_to);
        } break;
    }
}

internal Ast_Type*
find_required_return_type(Ast* ast,
                          const Index current_scope_index)
{
    Index next_scope_index = current_scope_index;
    while (next_scope_index != LAST_LEXICAL_SCOPE_INDEX)
    {
        Lexical_Scope* scope = &ast->lexical_scopes[next_scope_index];
        Ast_Type* return_type = &scope->required_return_type;

        if (return_type->type != AST_TYPE_UNSPECIFIED)
        {
            return return_type;
        }

        next_scope_index = scope->parent_scope_index;
    }

    return NULL;
}

// TODO(vlad): Change the return type to 'Ast_Type*' or something. Probably better to use indices as they won't
//             be invalidated after reallocations.
internal Ast_Type
get_inferred_type(Ast* ast,
                  const Index current_scope_index,
                  const Ast_Expression* expression)
{
    // TODO(vlad): Store the pointer to inferred type in the expression?
    Ast_Type result = {0};

    switch (expression->type)
    {
        case AST_EXPRESSION_UNDEFINED:
        {
            UNREACHABLE();
        } break;

        case AST_EXPRESSION_NUMBER:
        {
            // FIXME(vlad): Remove this ad-hoc handling.
            const String_View lexeme = expression->number.token.lexeme;

            Index fraction_start_index = -1;
            for (Index lexeme_index = 0;
                 lexeme_index < lexeme.length;
                 ++lexeme_index)
            {
                if (lexeme.data[lexeme_index] == '.')
                {
                    fraction_start_index = lexeme_index;
                    break;
                }
            }

            if (fraction_start_index == -1)
            {
                result.type = AST_TYPE_INT_32;
            }
            else
            {
                result.type = AST_TYPE_FLOAT_32;
            }
        } break;

        case AST_EXPRESSION_STRING_LITERAL:
        {
            FAIL("String literals are not supported yet");
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            const Ast_Variable_Definition* variable = find_variable(ast,
                                                                    current_scope_index,
                                                                    expression->identifier.token.lexeme);

            if (variable == NULL)
            {
                // TODO(vlad): Remove this ad-hoc handling of the 'print' function.
                if (strings_are_equal(expression->identifier.token.lexeme, string_view("print")))
                {
                    result.type = AST_TYPE_FUNCTION;
                    return result;
                }

                const Ast_Function_Definition* function = find_function(ast, expression->identifier.token.lexeme);
                if (function == NULL)
                {
                    FAIL("Failed to infer expression type: unknown identifier found");
                }

                result = *function->type;
            }
            else
            {
                if (variable->type->type == AST_TYPE_UNSPECIFIED)
                {
                    FAIL("Failed to infer expression type: variable's type was not specified");
                }

                // TODO(vlad): Just return this pointer?
                result = *variable->type;
            }
        } break;

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        {
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;

            const Ast_Type lhs_type = get_inferred_type(ast, current_scope_index, binary_expression->lhs);
            const Ast_Type rhs_type = get_inferred_type(ast, current_scope_index, binary_expression->rhs);

            if (!types_are_equal(&lhs_type, &rhs_type))
            {
                FAIL("LHS and RHS of a binary expression must be of the same type");
            }

            // TODO(vlad): Test that this binary operator is defined for these types.

            result = lhs_type;
        } break;

        case AST_EXPRESSION_EQUAL:
        case AST_EXPRESSION_NOT_EQUAL:
        case AST_EXPRESSION_LESS:
        case AST_EXPRESSION_LESS_OR_EQUAL:
        case AST_EXPRESSION_GREATER:
        case AST_EXPRESSION_GREATER_OR_EQUAL:
        {
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;

            const Ast_Type lhs_type = get_inferred_type(ast, current_scope_index, binary_expression->lhs);
            const Ast_Type rhs_type = get_inferred_type(ast, current_scope_index, binary_expression->rhs);

            if (!types_are_equal(&lhs_type, &rhs_type))
            {
                FAIL("LHS and RHS of a comparision must be of the same type");
            }

            // TODO(vlad): Test that this comparison is defined for these types.

            result.type = AST_TYPE_BOOL;
        } break;

        case AST_EXPRESSION_CALL:
        {
            // TODO(vlad): Remove code duplication (see 'AST_STATEMENT_CALL' below).
            const Ast_Call* call = &expression->call;

            const Ast_Type called_expression_type = get_inferred_type(ast,
                                                                      current_scope_index,
                                                                      call->called_expression);

            if (called_expression_type.type != AST_TYPE_FUNCTION)
            {
                FAIL("Failed to call expression: its type is not callable");
            }

            const Ast_Function_Arguments* expected_arguments = &called_expression_type.arguments;
            if (expected_arguments->arguments_count != call->arguments_count)
            {
                FAIL("Failed to call expression: wrong number of arguments provided");
            }

            for (Index argument_index = 0;
                 argument_index < expected_arguments->arguments_count;
                 ++argument_index)
            {
                const Ast_Variable_Definition* expected_argument = &expected_arguments->arguments[argument_index];
                const Ast_Expression* actual_argument = call->arguments[argument_index];

                const Ast_Type actual_argument_type = get_inferred_type(ast,
                                                                        current_scope_index,
                                                                        actual_argument);

                if (!types_are_equal(expected_argument->type, &actual_argument_type))
                {
                    FAIL("Failed to call expression: argument of a wrong type provided");
                }
            }

            result = *called_expression_type.return_type;
        } break;
    }

    return result;
}

internal Bool
analyse_lexical_scope_and_infer_types_in_statements(Arena* lexical_scopes_arena,
                                                    Ast* ast,
                                                    Ast_Statements* statements,
                                                    const Index current_scope_index)
{
    statements->lexical_scope_index = current_scope_index;

    for (Index statement_index = 0;
         statement_index < statements->statements_count;
         ++statement_index)
    {
        Ast_Statement* statement = &statements->statements[statement_index];

        switch (statement->type)
        {
            case AST_UNDEFINED:
            {
                UNREACHABLE();
            } break;

            case AST_STATEMENT_VARIABLE_DEFINITION:
            {
                Ast_Variable_Definition* definition = &statement->variable_definition;
                if (definition->type->type == AST_TYPE_UNSPECIFIED)
                {
                    ASSERT(definition->initialisation_type == AST_INITIALISATION_WITH_VALUE);

                    const Ast_Qualifiers qualifiers = definition->type->qualifiers;
                    *definition->type = get_inferred_type(ast, current_scope_index, &definition->initial_value);
                    definition->type->qualifiers = qualifiers;
                }
                else
                {
                    if (definition->initialisation_type == AST_INITIALISATION_WITH_VALUE)
                    {
                        const Ast_Type initial_value_type = get_inferred_type(ast,
                                                                              current_scope_index,
                                                                              &definition->initial_value);

                        if (!types_are_equal(definition->type, &initial_value_type))
                        {
                            FAIL("Failed to initialise variable: wrong initial value type");
                        }
                    }

                    // NOTE(vlad): All types are considered default-initialisable.
                }

                if (!add_variable_to_lexical_scope(lexical_scopes_arena,
                                                   ast,
                                                   current_scope_index,
                                                   definition))
                {
                    return false;
                }
            } break;

            case AST_STATEMENT_ASSIGNMENT:
            {
                Ast_Assignment* assignment = &statement->assignment;

                Ast_Variable_Definition* variable = find_variable(ast,
                                                                  current_scope_index,
                                                                  assignment->name.token.lexeme);

                Ast_Type expression_type = get_inferred_type(ast,
                                                             current_scope_index,
                                                             &assignment->expression);

                if (!types_are_equal(variable->type, &expression_type))
                {
                    FAIL("Failed to assign variable: expression type does not match the variable one");
                }
            } break;

            case AST_STATEMENT_RETURN:
            {
                Ast_Return_Statement* return_statement = &statement->return_statement;

                const Ast_Type* required_return_type = find_required_return_type(ast, current_scope_index);
                if (required_return_type == NULL)
                {
                    FAIL("Return statement found, but required return type was not");
                }

                Ast_Type return_expression_type = {0};
                if (return_statement->is_empty)
                {
                    return_expression_type.type = AST_TYPE_VOID;
                }
                else
                {
                    return_expression_type = get_inferred_type(ast, current_scope_index, &return_statement->expression);
                }

                if (!types_are_equal(required_return_type, &return_expression_type))
                {
                    FAIL("Return statement expression type does not match the required return type");
                }
            } break;

            case AST_STATEMENT_WHILE:
            {
                Ast_While_Statement* while_statement = &statement->while_statement;

                const Ast_Expression* condition = &while_statement->condition;
                const Ast_Type condition_type = get_inferred_type(ast, current_scope_index, condition);

                if (condition_type.type != AST_TYPE_BOOL)
                {
                    FAIL("Condition type of the while statement must be boolean");
                }

                Ast_Statements* body = &while_statement->statements;
                if (body->statements_count != 0)
                {
                    const Index body_scope_index = create_new_lexical_scope_with_parent(lexical_scopes_arena,
                                                                                        ast,
                                                                                        current_scope_index);

                    if (!analyse_lexical_scope_and_infer_types_in_statements(lexical_scopes_arena,
                                                                             ast,
                                                                             body,
                                                                             body_scope_index))
                    {
                        return false;
                    }
                }
            } break;

            case AST_STATEMENT_IF:
            {
                Ast_If_Statement* if_statement = &statement->if_statement;

                const Ast_Expression* condition = &if_statement->condition;
                const Ast_Type condition_type = get_inferred_type(ast, current_scope_index, condition);
                if (condition_type.type != AST_TYPE_BOOL)
                {
                    FAIL("Condition type of the if statement must be boolean");
                }

                {
                    Ast_Statements* if_statements = &if_statement->if_statements;

                    if (if_statements->statements_count != 0)
                    {
                        const Index if_statements_scope_index
                            = create_new_lexical_scope_with_parent(lexical_scopes_arena,
                                                                   ast,
                                                                   current_scope_index);

                        if (!analyse_lexical_scope_and_infer_types_in_statements(lexical_scopes_arena,
                                                                                 ast,
                                                                                 if_statements,
                                                                                 if_statements_scope_index))
                        {
                            return false;
                        }
                    }
                }

                {
                    Ast_Statements* else_statements = &if_statement->else_statements;

                    if (else_statements->statements_count != 0)
                    {
                        const Index else_statements_scope_index
                            = create_new_lexical_scope_with_parent(lexical_scopes_arena,
                                                                   ast,
                                                                   current_scope_index);

                        if (!analyse_lexical_scope_and_infer_types_in_statements(lexical_scopes_arena,
                                                                                 ast,
                                                                                 else_statements,
                                                                                 else_statements_scope_index))
                        {
                            return false;
                        }
                    }
                }
            } break;

            case AST_STATEMENT_CALL:
            {
                Ast_Call_Statement* call_statement = &statement->call_statement;
                Ast_Call* call = &call_statement->call;

                const Ast_Type called_expression_type = get_inferred_type(ast,
                                                                          current_scope_index,
                                                                          call->called_expression);

                if (called_expression_type.type != AST_TYPE_FUNCTION)
                {
                    FAIL("Failed to call expression: its type is not callable");
                }

                if (call->called_expression->type == AST_EXPRESSION_IDENTIFIER
                    && strings_are_equal(call->called_expression->identifier.token.lexeme,
                                         string_view("print")))
                {
                    // NOTE(vlad): Ignore type checks for 'print' function.
                    // TODO(vlad): Add these checks when we will support function overloading.
                }
                else
                {
                    const Ast_Function_Arguments* expected_arguments = &called_expression_type.arguments;
                    if (expected_arguments->arguments_count != call->arguments_count)
                    {
                        FAIL("Failed to call expression: wrong number of arguments provided");
                    }

                    for (Index argument_index = 0;
                         argument_index < expected_arguments->arguments_count;
                         ++argument_index)
                    {
                        const Ast_Variable_Definition* expected_argument = &expected_arguments->arguments[argument_index];
                        const Ast_Expression* actual_argument = call->arguments[argument_index];

                        const Ast_Type actual_argument_type = get_inferred_type(ast,
                                                                                current_scope_index,
                                                                                actual_argument);

                        if (!types_are_equal(expected_argument->type, &actual_argument_type))
                        {
                            FAIL("Failed to call expression: argument of a wrong type provided");
                        }
                    }
                }
            } break;
        }
    }

    // TODO(vlad): Ensure that the non-void functions have a return value.

    return true;
}

internal Bool
create_lexical_scopes_and_infer_types(Arena* lexical_scopes_arena, Ast* ast)
{
    const Index sentinel_scope_index = create_new_lexical_scope_with_parent(lexical_scopes_arena,
                                                                            ast,
                                                                            LAST_LEXICAL_SCOPE_INDEX);
    ASSERT(sentinel_scope_index == LAST_LEXICAL_SCOPE_INDEX);

    const Index global_scope_index = create_new_lexical_scope_with_parent(lexical_scopes_arena,
                                                                          ast,
                                                                          sentinel_scope_index);

    for (Index function_definition_index = 0;
         function_definition_index < ast->function_definitions_count;
         ++function_definition_index)
    {
        Ast_Function_Definition* function_definition = &ast->function_definitions[function_definition_index];

        // TODO(vlad): Don't create a lexical scope if there are no statements in the function?
        //             Ensure that this function returns void, otherwise report an error.
        const Index function_scope_index = create_new_lexical_scope_with_parent(lexical_scopes_arena,
                                                                                ast,
                                                                                global_scope_index);

        ASSERT(function_definition->type->type == AST_TYPE_FUNCTION);
        {
            Ast_Function_Arguments* arguments = &function_definition->type->arguments;

            for (Index argument_index = 0;
                 argument_index < arguments->arguments_count;
                 ++argument_index)
            {
                Ast_Variable_Definition* argument = &arguments->arguments[argument_index];

                // XXX(vlad): Should we support unspecified argument types? Like 'foo: (n := 10) -> void = {}'.
                // STUDY(vlad): This looks like a generic function if we would defer the type inference until usage.
                ASSERT(argument->type->type != AST_TYPE_UNSPECIFIED);

                if (!add_variable_to_lexical_scope(lexical_scopes_arena,
                                                   ast,
                                                   function_scope_index,
                                                   argument))
                {
                    return false;
                }
            }
        }

        {
            Lexical_Scope* scope = &ast->lexical_scopes[function_scope_index];
            scope->required_return_type = *function_definition->type->return_type;
        }

        Ast_Statements* statements = &function_definition->statements;
        if (!analyse_lexical_scope_and_infer_types_in_statements(lexical_scopes_arena,
                                                                 ast,
                                                                 statements,
                                                                 function_scope_index))
        {
            return false;
        }
    }

    return true;
}
