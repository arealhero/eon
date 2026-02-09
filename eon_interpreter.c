#include "eon_interpreter.h"

#include <eon/string.h>

internal void
push_empty_lexical_scope(Interpreter* interpreter,
                         Interpreter_Lexical_Scope* parent_scope)
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
    lexical_scope->parent_scope = parent_scope;
}

internal void
pop_lexical_scope(Interpreter* interpreter)
{
    interpreter->lexical_scopes_count -= 1;
}

internal Interpreter_Lexical_Scope*
get_current_lexical_scope(Interpreter* interpreter)
{
    if (interpreter->lexical_scopes_count == 0)
    {
        return &interpreter->global_lexical_scope;
    }
    else
    {
        return &interpreter->lexical_scopes[interpreter->lexical_scopes_count - 1];
    }
}

internal Interpreter_Variable*
find_variable(Interpreter* interpreter, const String_View variable_name)
{
    for (const Interpreter_Lexical_Scope* scope = get_current_lexical_scope(interpreter);
         scope != NULL;
         scope = scope->parent_scope)
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
                   s32* result) // TODO(vlad): Remove hardcoded 's32'.
{
    switch (expression->type)
    {
        case AST_EXPRESSION_NUMBER:
        {
            if (!parse_integer(expression->number.token.lexeme, result))
            {
                FAIL("Failed to parse integer");
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

            *result = variable->value;
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

            Call_Info call_info = {0};
            call_info.arguments_count = call->arguments_count;
            call_info.arguments_capacity = call->arguments_capacity;
            call_info.arguments = allocate_array(runtime_arena, call_info.arguments_count, s32);

            for (Index argument_index = 0;
                 argument_index < call_info.arguments_count;
                 ++argument_index)
            {
                s32 value;
                if (!execute_expression(runtime_arena,
                                        interpreter,
                                        ast,
                                        call->arguments[argument_index],
                                        &value))
                {
                    FAIL("Failed to execute expression");
                }

                call_info.arguments[argument_index] = value;
            }

            push_empty_lexical_scope(interpreter, &interpreter->global_lexical_scope);

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

            *result = call_result.return_value;
            return true;
        } break;

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        case AST_EXPRESSION_EQUAL:
        case AST_EXPRESSION_NOT_EQUAL:
        {
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;

            s32 lhs;
            if (!execute_expression(runtime_arena, interpreter, ast, binary_expression->lhs, &lhs))
            {
                return false;
            }

            s32 rhs;
            if (!execute_expression(runtime_arena, interpreter, ast, binary_expression->rhs, &rhs))
            {
                return false;
            }

            switch (expression->type)
            {
                case AST_EXPRESSION_ADD:
                {
                    *result = lhs + rhs;
                } break;

                case AST_EXPRESSION_SUBTRACT:
                {
                    *result = lhs - rhs;
                } break;

                case AST_EXPRESSION_MULTIPLY:
                {
                    *result = lhs * rhs;
                } break;

                case AST_EXPRESSION_DIVIDE:
                {
                    *result = lhs / rhs;
                } break;

                case AST_EXPRESSION_EQUAL:
                {
                    *result = lhs == rhs;
                } break;

                case AST_EXPRESSION_NOT_EQUAL:
                {
                    *result = lhs != rhs;
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

internal void
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
            variable->value = 0;
        } break;

        case AST_INITIALISATION_WITH_VALUE:
        {
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &variable_definition->initial_value,
                                    &variable->value))
            {
                FAIL("Failed to initialise variable");
            }
        } break;

        default:
        {
            FAIL("Unknown initialisation type");
        } break;
    }
}

internal void
set_new_value_for_the_variable(Interpreter* interpreter,
                               const Ast_Identifier* identifier,
                               const s32 new_value)
{
    Interpreter_Variable* variable = find_variable(interpreter, identifier->token.lexeme);

    if (variable == NULL)
    {
        FAIL("Variable was not found");
    }

    variable->value = new_value;
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

            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &return_statement->expression,
                                    &result.return_value))
            {
                FAIL("Failed to execute return statement's expression");
            }

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

            s32 value;
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &assignment->expression,
                                    &value))
            {
                FAIL("Failed to execute assignment statement's expression");
            }

            set_new_value_for_the_variable(interpreter, &assignment->name, value);
        } break;

        case AST_STATEMENT_IF:
        {
            const Ast_If_Statement* if_statement = &statement->if_statement;

            s32 result;
            if (!execute_expression(runtime_arena,
                                    interpreter,
                                    ast,
                                    &if_statement->condition,
                                    &result))
            {
                FAIL("Failed to execute if statement's expression");
            }

            const Ast_Statements* statements = result
                ? &if_statement->if_statements
                : &if_statement->else_statements;

            push_empty_lexical_scope(interpreter, get_current_lexical_scope(interpreter));

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
                    return this_result;
                }
            }

            pop_lexical_scope(interpreter);
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
    interpreter->global_lexical_scope = (Interpreter_Lexical_Scope){0};
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
        const Ast_Function_Argument* argument = &function_definition->type->arguments.arguments[argument_index];

        if (argument->type->type != AST_TYPE_INT_32)
        {
            Run_Result result = {0};
            result.status = INTERPRETER_RUN_COMPILE_ERROR;
            result.error = format_string(result_arena,
                                         "Function arguments with a type other than Int32"
                                         " are not yet supported");
            return result;
        }

        // TODO(vlad): Stop using Ast structure here.
        Ast_Variable_Definition definition = {0};
        definition.name = argument->name;
        definition.type = argument->type;
        definition.initialisation_type = AST_INITIALISATION_DEFAULT;
        interpreter_add_variable_to_current_lexical_scope(runtime_arena,
                                                          interpreter,
                                                          ast,
                                                          &definition);

        set_new_value_for_the_variable(interpreter, &definition.name, call_info->arguments[argument_index]);
    }

    const Ast_Type_Type return_type = function_definition->type->return_type->type;
    if (return_type != AST_TYPE_INT_32)
    {
        Run_Result result = {0};
        result.status = INTERPRETER_RUN_COMPILE_ERROR;
        result.error = format_string(result_arena,
                                     "Execution of functions with a return type other than Int32"
                                     " is not yet supported");
        return result;
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
