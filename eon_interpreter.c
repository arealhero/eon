#include "eon_interpreter.h"

#include <eon/string.h>

internal void
push_empty_lexical_scope(Interpreter* interpreter,
                         const Index parent_scope_index)
{
    if (interpreter->lexical_scopes_count == interpreter->lexical_scopes_capacity)
    {
        // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
        const Size new_capacity = MAX(1, 2 * interpreter->lexical_scopes_capacity);
        interpreter->lexical_scopes = reallocate(interpreter->lexical_scopes_arena,
                                                 interpreter->lexical_scopes,
                                                 Interpreter_Lexical_Scope,
                                                 interpreter->lexical_scopes_capacity,
                                                 new_capacity);
        interpreter->lexical_scopes_capacity = new_capacity;
    }

    Interpreter_Lexical_Scope* lexical_scope = &interpreter->lexical_scopes[interpreter->lexical_scopes_count++];
    *lexical_scope = (Interpreter_Lexical_Scope) {0};
    lexical_scope->parent_scope_index = parent_scope_index;
}

internal void
pop_lexical_scope(Interpreter* interpreter)
{
    interpreter->lexical_scopes_count -= 1;
}

internal Index
get_current_lexical_scope_index(Interpreter* interpreter)
{
    if (interpreter->lexical_scopes_count == 2)
    {
        return INTERPRETER_GLOBAL_LEXICAL_SCOPE_INDEX;
    }
    else
    {
        return interpreter->lexical_scopes_count - 1;
    }
}

internal Interpreter_Lexical_Scope*
get_current_lexical_scope(Interpreter* interpreter)
{
    return &interpreter->lexical_scopes[get_current_lexical_scope_index(interpreter)];
}

internal Interpreter_Variable*
find_variable(Interpreter* interpreter, const String_View variable_name)
{
    for (const Interpreter_Lexical_Scope* scope = get_current_lexical_scope(interpreter);
         scope->parent_scope_index != -1;
         scope = &interpreter->lexical_scopes[scope->parent_scope_index])
    {
        for (Index variable_index = 0;
             variable_index < scope->variables_count;
             ++variable_index)
        {
            Interpreter_Variable* variable = &scope->variables[variable_index];

            if (strings_are_equal(variable->name.token.lexeme, variable_name))
            {
                return variable;
            }
        }
    }

    return NULL;
}

internal Bool
execute_expression(Arena* runtime_arena,
                   Interpreter* interpreter,
                   const Ast* ast,
                   const Ast_Expression* expression,
                   Interpreter_Expression_Result* result)
{
    switch (expression->type)
    {
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
                result->type = AST_TYPE_INT_32;
                if (!parse_integer(lexeme, &result->s32_value))
                {
                    FAIL("Failed to parse integer");
                }
            }
            else
            {
                result->type = AST_TYPE_FLOAT_32;
                if (!parse_float(lexeme, &result->f32_value))
                {
                    FAIL("Failed to parse float");
                }
            }

            return true;
        } break;

        case AST_EXPRESSION_IDENTIFIER:
        {
            const Ast_Identifier* identifier = &expression->identifier;

            // TODO(vlad): Support other entities like functions, types (?) and so on.
            const Interpreter_Variable* variable = find_variable(interpreter,
                                                                 identifier->token.lexeme);

            if (variable == NULL)
            {
                FAIL("Variable was not found");
            }

            result->type = variable->type->type;
            switch (variable->type->type)
            {
                case AST_TYPE_INT_32:
                {
                    result->s32_value = variable->s32_value;
                } break;

                case AST_TYPE_FLOAT_32:
                {
                    result->f32_value = variable->f32_value;
                } break;

                default:
                {
                    FAIL("Variable with type other that Int32 or Float32 "
                         "are not yet supported");
                } break;
            }

            return true;
        } break;

        case AST_EXPRESSION_CALL:
        {
            const Ast_Call* call = &expression->call;
            const Ast_Expression* called_expression = call->called_expression;
            if (called_expression->type != AST_EXPRESSION_IDENTIFIER)
            {
                FAIL("Expression calling is not yet supported");
            }

            if (strings_are_equal(called_expression->identifier.token.lexeme, string_view("print")))
            {
                // TODO(vlad): Remove this special handling of a 'print' function.
                if (call->arguments_count != 1)
                {
                    FAIL("Invalid number of arguments provided while calling function 'print'");
                }

                Interpreter_Expression_Result argument = {0};
                if (!execute_expression(runtime_arena,
                                        interpreter,
                                        ast,
                                        call->arguments[0],
                                        &argument))
                {
                    FAIL("Failed to execute expression");
                }

                switch (argument.type)
                {
                    case AST_TYPE_INT_32:
                    {
                        println("{}", argument.s32_value);
                    } break;

                    case AST_TYPE_FLOAT_32:
                    {
                        println("{}", argument.f32_value);
                    } break;

                    default:
                    {
                        FAIL("[interpreter] Unsupported type encountered");
                    } break;
                }

                return true;
            }

            Call_Info call_info = {0};
            call_info.arguments_count = call->arguments_count;
            call_info.arguments_capacity = call->arguments_capacity;
            call_info.arguments = allocate_array(runtime_arena,
                                                 call_info.arguments_count,
                                                 Interpreter_Expression_Result);

            for (Index argument_index = 0;
                 argument_index < call_info.arguments_count;
                 ++argument_index)
            {
                Interpreter_Expression_Result argument = {0};
                if (!execute_expression(runtime_arena,
                                        interpreter,
                                        ast,
                                        call->arguments[argument_index],
                                        &argument))
                {
                    FAIL("Failed to execute expression");
                }

                call_info.arguments[argument_index] = argument;
            }

            push_empty_lexical_scope(interpreter, INTERPRETER_GLOBAL_LEXICAL_SCOPE_INDEX);

            const Run_Result call_result = interpreter_execute_function(runtime_arena,
                                                                        runtime_arena,
                                                                        interpreter,
                                                                        ast,
                                                                        called_expression->identifier.token.lexeme,
                                                                        &call_info);

            pop_lexical_scope(interpreter);

            if (call_result.status != INTERPRETER_RUN_OK)
            {
                println("Function call failed: {}", call_result.error);
                FAIL("Failed to call function");
            }

            *result = call_result.result;
            return true;
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
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;

            Interpreter_Expression_Result lhs;
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    binary_expression->lhs,
                                    &lhs))
            {
                return false;
            }

            Interpreter_Expression_Result rhs;
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    binary_expression->rhs,
                                    &rhs))
            {
                return false;
            }

            if (lhs.type != rhs.type)
            {
                FAIL("Left and right hand sides of the expression must be the same");
            }

            switch (expression->type)
            {
                case AST_EXPRESSION_ADD:
                {
                    result->type = lhs.type;

                    if (result->type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value + rhs.s32_value;
                    }
                    else if (result->type == AST_TYPE_FLOAT_32)
                    {
                        result->f32_value = lhs.f32_value + rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_SUBTRACT:
                {
                    result->type = lhs.type;

                    if (result->type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value - rhs.s32_value;
                    }
                    else if (result->type == AST_TYPE_FLOAT_32)
                    {
                        result->f32_value = lhs.f32_value - rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_MULTIPLY:
                {
                    result->type = lhs.type;

                    if (result->type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value * rhs.s32_value;
                    }
                    else if (result->type == AST_TYPE_FLOAT_32)
                    {
                        result->f32_value = lhs.f32_value * rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_DIVIDE:
                {
                    result->type = lhs.type;

                    if (result->type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value / rhs.s32_value;
                    }
                    else if (result->type == AST_TYPE_FLOAT_32)
                    {
                        result->f32_value = lhs.f32_value / rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                // FIXME(vlad): Use Bool as a resulting type for comparison expressions.
                case AST_EXPRESSION_EQUAL:
                {
                    result->type = AST_TYPE_INT_32;

                    if (lhs.type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value == rhs.s32_value;
                    }
                    else if (lhs.type == AST_TYPE_FLOAT_32)
                    {
                        FAIL("You really should not compare floats via '=='");
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_NOT_EQUAL:
                {
                    result->type = AST_TYPE_INT_32;

                    if (lhs.type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value != rhs.s32_value;
                    }
                    else if (lhs.type == AST_TYPE_FLOAT_32)
                    {
                        FAIL("You really should not compare floats via '!='");
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_LESS:
                {
                    result->type = AST_TYPE_INT_32;

                    if (lhs.type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value < rhs.s32_value;
                    }
                    else if (lhs.type == AST_TYPE_FLOAT_32)
                    {
                        result->s32_value = lhs.f32_value < rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_LESS_OR_EQUAL:
                {
                    result->type = AST_TYPE_INT_32;

                    if (lhs.type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value <= rhs.s32_value;
                    }
                    else if (lhs.type == AST_TYPE_FLOAT_32)
                    {
                        result->s32_value = lhs.f32_value <= rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_GREATER:
                {
                    result->type = AST_TYPE_INT_32;

                    if (lhs.type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value > rhs.s32_value;
                    }
                    else if (lhs.type == AST_TYPE_FLOAT_32)
                    {
                        result->s32_value = lhs.f32_value > rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                case AST_EXPRESSION_GREATER_OR_EQUAL:
                {
                    result->type = AST_TYPE_INT_32;

                    if (lhs.type == AST_TYPE_INT_32)
                    {
                        result->s32_value = lhs.s32_value >= rhs.s32_value;
                    }
                    else if (lhs.type == AST_TYPE_FLOAT_32)
                    {
                        result->s32_value = lhs.f32_value >= rhs.f32_value;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            return true;
        } break;

        default:
        {
            return false;
        } break;
    }
}

internal Interpreter_Variable*
interpreter_add_variable_to_current_lexical_scope(Arena* runtime_arena,
                                                  Interpreter* interpreter,
                                                  const Ast* ast,
                                                  const Ast_Variable_Definition* variable_definition)
{
    Interpreter_Lexical_Scope* scope = get_current_lexical_scope(interpreter);

    if (scope->variables_count == scope->variables_capacity)
    {
        // XXX(vlad): We can change 'MAX(1, 2 * capacity)' to '(2 * capacity) | 1'.
        const Size new_capacity = MAX(1, 2 * scope->variables_capacity);
        scope->variables = reallocate(runtime_arena,
                                      scope->variables,
                                      Interpreter_Variable,
                                      scope->variables_capacity,
                                      new_capacity);
        scope->variables_capacity = new_capacity;
    }

    Interpreter_Variable* variable = &scope->variables[scope->variables_count++];
    variable->name = variable_definition->name;
    variable->type = variable_definition->type;

    switch (variable_definition->initialisation_type)
    {
        case AST_INITIALISATION_DEFAULT:
        {
            switch (variable->type->type)
            {
                case AST_TYPE_INT_32:
                {
                    variable->s32_value = (s32) 0;
                } break;

                case AST_TYPE_FLOAT_32:
                {
                    variable->f32_value = (f32) 0.0f;
                } break;

                case AST_TYPE_UNSPECIFIED:
                {
                    // NOTE(vlad): Wait until the assignment.
                } break;

                default:
                {
                    FAIL("Types other than Int32 and Float32 are not supported yet");
                } break;
            }
        } break;

        case AST_INITIALISATION_WITH_VALUE:
        {
            Interpreter_Expression_Result result = {0};
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &variable_definition->initial_value,
                                    &result))
            {
                FAIL("Failed to initialise variable");
            }

            if (variable->type->type == AST_TYPE_UNSPECIFIED)
            {
                variable->type->type = result.type;
            }
            else if (result.type != variable->type->type)
            {
                FAIL("Failed to initialise variable: wrong expression type");
            }

            switch (result.type)
            {
                case AST_TYPE_INT_32:
                {
                    variable->s32_value = result.s32_value;
                } break;

                case AST_TYPE_FLOAT_32:
                {
                    variable->f32_value = result.f32_value;
                } break;

                default:
                {
                    FAIL("Types other than Int32 and Float32 are not supported yet");
                } break;
            }
        } break;

        default:
        {
            FAIL("Unknown initialisation type");
        } break;
    }

    return variable;
}

internal void
set_value_for_the_variable(Interpreter_Variable* variable,
                           const Interpreter_Expression_Result* value)
{
    if (variable->type->type == AST_TYPE_UNSPECIFIED)
    {
        variable->type->type = value->type;
    }
    else if (variable->type->type != value->type)
    {
        FAIL("Failed to set value for variable: wrong expression type");
    }

    switch (variable->type->type)
    {
        case AST_TYPE_INT_32:
        {
            variable->s32_value = value->s32_value;
        } break;

        case AST_TYPE_FLOAT_32:
        {
            variable->f32_value = value->f32_value;
        } break;

        default:
        {
            FAIL("Types other than Int32 and Float32 are not supported yet");
        } break;
    }
}

internal void
set_new_value_for_the_variable(Interpreter* interpreter,
                               const Ast_Identifier* identifier,
                               const Interpreter_Expression_Result* new_value)
{
    Interpreter_Variable* variable = find_variable(interpreter, identifier->token.lexeme);

    if (variable == NULL)
    {
        FAIL("Variable was not found");
    }

    // TODO(vlad): Change this to '!(qualifiers & AST_QUALIFIER_MUTABLE)'.
    if (variable->type->qualifiers != AST_QUALIFIER_MUTABLE)
    {
        FAIL("Variable is constant");
    }

    set_value_for_the_variable(variable, new_value);
}

internal Run_Result
execute_statement(Arena* runtime_arena,
                  Arena* result_arena,
                  Interpreter* interpreter,
                  const Ast* ast,
                  const Ast_Statement* statement)
{
    switch (statement->type)
    {
        case AST_STATEMENT_RETURN:
        {
            const Ast_Return_Statement* return_statement = &statement->return_statement;

            if (return_statement->is_empty)
            {
                Run_Result result = {0};
                result.status = INTERPRETER_RUN_COMPILE_ERROR;
                result.error = format_string(result_arena,
                                             "Empty return statements are not supported yet");
                return result;
            }

            Run_Result result = {0};
            result.status = INTERPRETER_RUN_OK;
            result.should_exit = true;

            Interpreter_Expression_Result expression_result = {0};
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &return_statement->expression,
                                    &expression_result))
            {
                FAIL("Failed to execute return statement's expression");
            }

            result.result = expression_result;
            return result;
        } break;

        case AST_STATEMENT_VARIABLE_DEFINITION:
        {
            const Ast_Variable_Definition* definition = &statement->variable_definition;
            interpreter_add_variable_to_current_lexical_scope(runtime_arena,
                                                              interpreter,
                                                              ast,
                                                              definition);
        } break;

        case AST_STATEMENT_ASSIGNMENT:
        {
            const Ast_Assignment* assignment = &statement->assignment;

            Interpreter_Expression_Result expression_result = {0};
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &assignment->expression,
                                    &expression_result))
            {
                FAIL("Failed to execute assignment statement's expression");
            }

            set_new_value_for_the_variable(interpreter,
                                           &assignment->name,
                                           &expression_result);
        } break;

        case AST_STATEMENT_WHILE:
        {
            const Ast_While_Statement* while_statement = &statement->while_statement;

            while (true)
            {
                Interpreter_Expression_Result condition_result = {0};
                if (!execute_expression(runtime_arena,
                                        interpreter,
                                        ast,
                                        &while_statement->condition,
                                        &condition_result))
                {
                    FAIL("Failed to execute while loop's condition expression");
                }

                // TODO(vlad): Test that the condition is convertible to bool.
                //             (Or rather IS a bool.)
                const s32 condition_value = condition_result.s32_value;
                if (!condition_value)
                {
                    break;
                }

                push_empty_lexical_scope(interpreter, get_current_lexical_scope_index(interpreter));

                for (Index statement_index = 0;
                     statement_index < while_statement->statements.statements_count;
                     ++statement_index)
                {
                    const Ast_Statement* this_statement = &while_statement->statements.statements[statement_index];
                    const Run_Result this_result = execute_statement(runtime_arena,
                                                                     result_arena,
                                                                     interpreter,
                                                                     ast,
                                                                     this_statement);
                    if (this_result.status != INTERPRETER_RUN_OK
                        || this_result.should_exit)
                    {
                        pop_lexical_scope(interpreter);
                        return this_result;
                    }
                }

                pop_lexical_scope(interpreter);
            }
        } break;

        case AST_STATEMENT_IF:
        {
            const Ast_If_Statement* if_statement = &statement->if_statement;

            Interpreter_Expression_Result condition_result = {0};
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &if_statement->condition,
                                    &condition_result))
            {
                FAIL("Failed to execute if statement's expression");
            }

            const Ast_Statements* statements = condition_result.s32_value
                ? &if_statement->if_statements
                : &if_statement->else_statements;

            push_empty_lexical_scope(interpreter, get_current_lexical_scope_index(interpreter));

            for (Index statement_index = 0;
                 statement_index < statements->statements_count;
                 ++statement_index)
            {
                const Ast_Statement* this_statement = &statements->statements[statement_index];
                const Run_Result this_result = execute_statement(runtime_arena,
                                                                 result_arena,
                                                                 interpreter,
                                                                 ast,
                                                                 this_statement);
                if (this_result.status != INTERPRETER_RUN_OK
                    || this_result.should_exit)
                {
                    pop_lexical_scope(interpreter);
                    return this_result;
                }
            }

            pop_lexical_scope(interpreter);
        } break;

        case AST_STATEMENT_CALL:
        {
            const Ast_Call_Statement* call_statement = &statement->call_statement;
            const Ast_Call* call_expression = &call_statement->call;

            Ast_Expression expression = {0};
            expression.type = AST_EXPRESSION_CALL;
            expression.call = *call_expression;

            Interpreter_Expression_Result call_expression_result = {0};
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &expression,
                                    &call_expression_result))
            {
                FAIL("Failed to execute call expression");
            }

            // TODO(vlad): Emit a warning about ignored result of a non-void functions?
            UNUSED(call_expression_result);
        } break;

        default:
        {
            Run_Result result = {0};
            result.status = INTERPRETER_RUN_COMPILE_ERROR;
            result.error = format_string(result_arena,
                                         "Unsupported statement encountered");
            FAIL("Unsupported statement encountered");
            return result;
        } break;
    }

    Run_Result result = {0};
    return result;
}

internal void
interpreter_create(Interpreter* interpreter, Arena* lexical_scopes_arena)
{
    interpreter->lexical_scopes_arena = lexical_scopes_arena;
    interpreter->lexical_scopes = allocate_array(interpreter->lexical_scopes_arena, 2, Interpreter_Lexical_Scope);
    interpreter->lexical_scopes_count = 2;
    interpreter->lexical_scopes_capacity = 2;
    interpreter->lexical_scopes[0].parent_scope_index = -1;
}

internal Run_Result
interpreter_execute_function(Arena* runtime_arena,
                             Arena* result_arena,
                             Interpreter* interpreter,
                             const Ast* ast,
                             const String_View name_of_the_function_to_run,
                             const Call_Info* call_info)
{
    const Ast_Function_Definition* function_definition = NULL;
    for (Index function_definition_index = 0;
         function_definition_index < ast->function_definitions_count;
         ++function_definition_index)
    {
        const Ast_Function_Definition* current_function_definition
            = &ast->function_definitions[function_definition_index];

        if (strings_are_equal(current_function_definition->name.token.lexeme,
                              name_of_the_function_to_run))
        {
            function_definition = current_function_definition;
            break;
        }
    }

    if (function_definition == NULL)
    {
        Run_Result result = {0};
        result.status = INTERPRETER_RUN_COMPILE_ERROR;
        result.error = format_string(result_arena,
                                     "Function '{}' is not defined",
                                     name_of_the_function_to_run);
        return result;
    }

    const Size arguments_count = function_definition->type->arguments.arguments_count;
    if (arguments_count != call_info->arguments_count)
    {
        Run_Result result = {0};
        result.status = INTERPRETER_RUN_COMPILE_ERROR;
        result.error = format_string(result_arena,
                                     "Invalid number of arguments provided while calling function '{}'"
                                     " (expected '{}', got '{}')",
                                     name_of_the_function_to_run,
                                     arguments_count,
                                     call_info->arguments_count);
        return result;
    }

    for (Index argument_index = 0;
         argument_index < arguments_count;
         ++argument_index)
    {
        Ast_Variable_Definition argument = function_definition->type->arguments.arguments[argument_index];

        // TODO(vlad): Remove this and pass argument directly.
        argument.initialisation_type = AST_INITIALISATION_DEFAULT;
        Interpreter_Variable* variable = interpreter_add_variable_to_current_lexical_scope(runtime_arena,
                                                                                           interpreter,
                                                                                           ast,
                                                                                           &argument);
        set_value_for_the_variable(variable, &call_info->arguments[argument_index]);
    }

    const Size statements_count = function_definition->statements.statements_count;
    if (statements_count == 0)
    {
        Run_Result result = {0};
        result.status = INTERPRETER_RUN_COMPILE_ERROR;
        result.error = format_string(result_arena,
                                     "Functions must have at least one statement for now");
        return result;
    }

    for (Index statement_index = 0;
         statement_index < statements_count;
         ++statement_index)
    {
        const Ast_Statement* statement = &function_definition->statements.statements[statement_index];
        const Run_Result this_result = execute_statement(runtime_arena, result_arena, interpreter, ast, statement);
        if (this_result.status != INTERPRETER_RUN_OK || this_result.should_exit)
        {
            return this_result;
        }
    }

    Run_Result result = {0};
    return result;
}

internal void
interpreter_destroy(Interpreter* interpreter)
{
    UNUSED(interpreter);
}
