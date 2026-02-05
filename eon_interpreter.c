#include "eon_interpreter.h"

#include <eon/string.h>

internal const Interpreter_Variable*
find_variable(Interpreter* interpreter, const String_View variable_name)
{
    const Interpreter_Lexical_Scope* scope = &interpreter->lexical_scope;

    // TODO(vlad): Support nested lexical scopes.
    for (Index variable_index = 0;
         variable_index < scope->variables_count;
         ++variable_index)
    {
        const Interpreter_Variable* variable = &scope->variables[variable_index];

        if (strings_are_equal(variable->name.token.lexeme, variable_name))
        {
            return variable;
        }
    }

    return NULL;
}

internal Bool
execute_expression(Arena* runtime_arena,
                   Interpreter* interpreter,
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

        case AST_EXPRESSION_ADD:
        case AST_EXPRESSION_SUBTRACT:
        case AST_EXPRESSION_MULTIPLY:
        case AST_EXPRESSION_DIVIDE:
        {
            const Ast_Binary_Expression* binary_expression = &expression->binary_expression;

            s32 lhs;
            if (!execute_expression(runtime_arena, interpreter, binary_expression->lhs, &lhs))
            {
                return false;
            }

            s32 rhs;
            if (!execute_expression(runtime_arena, interpreter, binary_expression->rhs, &rhs))
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
                                                  const Ast_Variable_Definition* variable_definition)
{
    Interpreter_Lexical_Scope* scope = &interpreter->lexical_scope;

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
interpreter_create(Interpreter* interpreter)
{
    interpreter->lexical_scope = (Interpreter_Lexical_Scope){0};
}

internal Run_Result
interpreter_run(Arena* runtime_arena,
                Arena* result_arena,
                Interpreter* interpreter,
                const Ast* ast,
                const String_View name_of_the_function_to_run)
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
    if (arguments_count != 0)
    {
        Run_Result result = {0};
        result.status = INTERPRETER_RUN_COMPILE_ERROR;
        result.error = format_string(result_arena,
                                     "Execution of functions with arguments is not yet supported"
                                     " (function '{}' requires {} argument{})",
                                     name_of_the_function_to_run,
                                     arguments_count,
                                     (arguments_count == 1 ? "" : "s"));
        return result;
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

                if (!execute_expression(runtime_arena,
                                        interpreter,
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
                                                                  definition);
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
    }

    Run_Result result = {0};
    return result;
}

internal void
interpreter_destroy(Interpreter* interpreter)
{
    UNUSED(interpreter);
}
