#include "eon_ast.h"

#include "eon_compilation_context.h"

#include <eon/containers.h>

internal void
validate_ast_jumps_in_code_block(Compilation_Context* context,
                                 Ast_Statement* most_recently_encountered_loop,
                                 Ast_Code_Block* code_block)
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

            case AST_STATEMENT_WHILE:
            {
                Ast_While_Statement* while_statement = &statement->while_statement;
                validate_ast_jumps_in_code_block(context, statement, &while_statement->body);
            } break;

            case AST_STATEMENT_IF:
            {
                Ast_If_Statement* if_statement = &statement->if_statement;

                validate_ast_jumps_in_code_block(context,
                                                 most_recently_encountered_loop,
                                                 &if_statement->if_statements);
                validate_ast_jumps_in_code_block(context,
                                                 most_recently_encountered_loop,
                                                 &if_statement->else_statements);
            } break;

            case AST_STATEMENT_BREAK:
            {
                Ast_Jump* jump = &statement->jump;

                if (most_recently_encountered_loop == NULL)
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = jump->token.location;
                    error.text = string_view("'break' statement outside of loop statements is not allowed");

                    emit_diagnostic_message(context, &error);
                }
                else
                {
                    jump->destination = most_recently_encountered_loop;
                }
            } break;

            case AST_STATEMENT_CONTINUE:
            {
                Ast_Jump* jump = &statement->jump;

                if (most_recently_encountered_loop == NULL)
                {
                    Diagnostic_Message error = {0};
                    error.level = MESSAGE_LEVEL_ERROR;
                    error.location = jump->token.location;
                    error.text = string_view("'continue' statement outside of loop statements is not allowed");

                    emit_diagnostic_message(context, &error);
                }
                else
                {
                    jump->destination = most_recently_encountered_loop;
                }
            } break;

            case AST_STATEMENT_VARIABLE_DEFINITION:
            case AST_STATEMENT_ASSIGNMENT:
            case AST_STATEMENT_RETURN:
            case AST_STATEMENT_CALL:
            {
            } break;
        }
    }
}

internal void
validate_ast(Compilation_Context* context)
{
    Ast* ast = &context->ast;

    for (Index function_index = 0;
         function_index < ast->function_definitions_count;
         ++function_index)
    {
        Ast_Function_Definition* function = &ast->function_definitions[function_index];

        // NOTE(vlad): Validating that 'break' and 'continue' statements occur only inside loops.
        validate_ast_jumps_in_code_block(context, NULL, &function->body);
    }
}
