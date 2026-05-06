#include "eon_unit_test.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"

internal void
test_function_scopes(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(function_definition->name.symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            const Symbol_Id symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       function_type->return_type->named_type.token.lexeme);
            ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
            ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 0);

        ASSERT_EQUAL(function_definition->body.statements_count, 0);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing function parameters.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (a: s32) -> void = {"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(function_definition->name.symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_type->parameters[0];
            ASSERT_EQUAL(parameter->type->kind, AST_TYPE_NAME);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           parameter->type->named_type.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(parameter->type->symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "s32");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_type->return_type->named_type.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
        const Ast_Function_Type* function_type = &function_definition->type->function;

        ASSERT_EQUAL(function_type->parameters_count, 1);

        {
            const Ast_Function_Parameter* parameter = &function_type->parameters[0];

            const Symbol_Id symbol_id = scope->symbol_ids[0];
            ASSERT_EQUAL(parameter->name.symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 0);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 1;"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            const Symbol_Id symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       function_type->return_type->named_type.token.lexeme);
            ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
            ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        {
            const Symbol_Id symbol_id = scope->symbol_ids[0];
            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, scope->symbol_ids[0]);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_if_statements_scopes(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 1;"
                                                 "    if 2 + 2 == 4 {"
                                                 "        a := 2.0;"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            const Symbol_Id symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       function_type->return_type->named_type.token.lexeme);
            ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
            ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        {
            const Symbol_Id symbol_id = scope->symbol_ids[0];
            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, scope->symbol_ids[0]);
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);

            const Ast_If_Statement* if_statement = &statement->if_statement;

            {
                const Ast_Code_Block* then_code_block = &if_statement->if_statements;
                ASSERT_EQUAL(then_code_block->lexical_scope_id, function_scope_id + 1);

                ASSERT_EQUAL(then_code_block->statements_count, 1);
                ASSERT_EQUAL(then_code_block->statements[0].type, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* variable_definition = &then_code_block->statements[0].variable_definition;

                const Lexical_Scope* then_code_block_scope = &context.lexical_scopes[then_code_block->lexical_scope_id];
                ASSERT_EQUAL(then_code_block_scope->parent_lexical_scope_id, function_scope_id);
                ASSERT_EQUAL(then_code_block_scope->symbol_ids_count, 1);

                const Symbol_Id symbol_id = then_code_block_scope->symbol_ids[0];
                ASSERT_EQUAL(variable_definition->name.symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }

            {
                const Ast_Code_Block* else_code_block = &if_statement->else_statements;
                ASSERT_EQUAL(else_code_block->lexical_scope_id, function_scope_id + 2);

                const Lexical_Scope* else_code_block_scope = &context.lexical_scopes[else_code_block->lexical_scope_id];
                ASSERT_EQUAL(else_code_block_scope->parent_lexical_scope_id, function_scope_id);
                ASSERT_EQUAL(else_code_block_scope->symbol_ids_count, 0);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 1;"
                                                 "    if 2 + 2 == 4 {"
                                                 "    } else {"
                                                 "        a := 2.0;"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            const Symbol_Id symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       function_type->return_type->named_type.token.lexeme);
            ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
            ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        {
            const Symbol_Id symbol_id = scope->symbol_ids[0];
            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, scope->symbol_ids[0]);
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_IF);

            const Ast_If_Statement* if_statement = &statement->if_statement;

            {
                const Ast_Code_Block* then_code_block = &if_statement->if_statements;
                ASSERT_EQUAL(then_code_block->lexical_scope_id, function_scope_id + 1);

                const Lexical_Scope* then_code_block_scope = &context.lexical_scopes[then_code_block->lexical_scope_id];
                ASSERT_EQUAL(then_code_block_scope->parent_lexical_scope_id, function_scope_id);
                ASSERT_EQUAL(then_code_block_scope->symbol_ids_count, 0);
            }

            {
                const Ast_Code_Block* else_code_block = &if_statement->else_statements;
                ASSERT_EQUAL(else_code_block->lexical_scope_id, function_scope_id + 2);

                ASSERT_EQUAL(else_code_block->statements_count, 1);
                ASSERT_EQUAL(else_code_block->statements[0].type, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* variable_definition = &else_code_block->statements[0].variable_definition;

                const Lexical_Scope* else_code_block_scope = &context.lexical_scopes[else_code_block->lexical_scope_id];
                ASSERT_EQUAL(else_code_block_scope->parent_lexical_scope_id, function_scope_id);
                ASSERT_EQUAL(else_code_block_scope->symbol_ids_count, 1);

                const Symbol_Id symbol_id = else_code_block_scope->symbol_ids[0];
                ASSERT_EQUAL(variable_definition->name.symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_while_loops_scopes(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 1;"
                                                 "    while a == 1 {"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            const Symbol_Id symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       function_type->return_type->named_type.token.lexeme);
            ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
            ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        {
            const Symbol_Id symbol_id = scope->symbol_ids[0];
            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, scope->symbol_ids[0]);
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            {
                const Ast_Code_Block* while_code_block = &while_statement->body;
                ASSERT_EQUAL(while_code_block->lexical_scope_id, function_scope_id + 1);

                const Lexical_Scope* then_code_block_scope = &context.lexical_scopes[while_code_block->lexical_scope_id];
                ASSERT_EQUAL(then_code_block_scope->parent_lexical_scope_id, function_scope_id);
                ASSERT_EQUAL(then_code_block_scope->symbol_ids_count, 0);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 1;"
                                                 "    while a == 1 {"
                                                 "        a := 2.0;"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            const Symbol_Id symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       function_type->return_type->named_type.token.lexeme);
            ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
            ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        {
            const Symbol_Id symbol_id = scope->symbol_ids[0];
            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, scope->symbol_ids[0]);
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            {
                const Ast_Code_Block* while_code_block = &while_statement->body;
                ASSERT_EQUAL(while_code_block->lexical_scope_id, function_scope_id + 1);

                ASSERT_EQUAL(while_code_block->statements_count, 1);
                ASSERT_EQUAL(while_code_block->statements[0].type, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* variable_definition = &while_code_block->statements[0].variable_definition;

                const Lexical_Scope* then_code_block_scope = &context.lexical_scopes[while_code_block->lexical_scope_id];
                ASSERT_EQUAL(then_code_block_scope->parent_lexical_scope_id, function_scope_id);
                ASSERT_EQUAL(then_code_block_scope->symbol_ids_count, 1);

                const Symbol_Id symbol_id = then_code_block_scope->symbol_ids[0];
                ASSERT_EQUAL(variable_definition->name.symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Yes, this looks bizzare, but this is valid in C, so I decided to leave it be.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 1;"
                                                 "    while a == 1 {"
                                                 "        a := a;"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            const Symbol_Id symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       function_type->return_type->named_type.token.lexeme);
            ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
            ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        {
            const Symbol_Id symbol_id = scope->symbol_ids[0];
            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 2);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, scope->symbol_ids[0]);
        }

        {
            const Ast_Statement* statement = &function_definition->body.statements[1];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_WHILE);

            const Ast_While_Statement* while_statement = &statement->while_statement;

            {
                const Ast_Code_Block* while_code_block = &while_statement->body;
                ASSERT_EQUAL(while_code_block->lexical_scope_id, function_scope_id + 1);

                ASSERT_EQUAL(while_code_block->statements_count, 1);
                ASSERT_EQUAL(while_code_block->statements[0].type, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* variable_definition = &while_code_block->statements[0].variable_definition;

                const Lexical_Scope* then_code_block_scope = &context.lexical_scopes[while_code_block->lexical_scope_id];
                ASSERT_EQUAL(then_code_block_scope->parent_lexical_scope_id, function_scope_id);
                ASSERT_EQUAL(then_code_block_scope->symbol_ids_count, 1);

                const Symbol_Id symbol_id = then_code_block_scope->symbol_ids[0];
                ASSERT_EQUAL(variable_definition->name.symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_that_every_identifier_has_symbol_id_in_expressions(Test_Context* test_context)
{
    // NOTE(vlad): Testing function parameters.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (a: s32) -> void = {"
                                                 "    var := a;"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_type->parameters[0];
            ASSERT_EQUAL(parameter->type->kind, AST_TYPE_NAME);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           parameter->type->named_type.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(parameter->type->symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "s32");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_type->return_type->named_type.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 2);

        ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
        const Ast_Function_Type* function_type = &function_definition->type->function;

        ASSERT_EQUAL(function_type->parameters_count, 1);

        {
            const Ast_Function_Parameter* parameter = &function_type->parameters[0];

            const Symbol_Id symbol_id = scope->symbol_ids[0];
            ASSERT_EQUAL(parameter->name.symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "a");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            const Symbol_Id symbol_id = scope->symbol_ids[1];
            ASSERT_EQUAL(variable_definition->name.symbol_id, symbol_id);

            const Symbol* symbol = &context.symbols[symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "var");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_EQUAL(initial_value->kind, AST_EXPRESSION_IDENTIFIER);

            const Ast_Identifier* identifier = &initial_value->identifier;
            ASSERT_STRINGS_ARE_EQUAL(identifier->token.lexeme, "a");
            ASSERT_EQUAL(identifier->symbol_id, scope->symbol_ids[0]);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (a: s32, b: s32) -> void = {"
                                                 "    var := a + b * 2;"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];
        ASSERT_EQUAL(function_definition->body.lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        ASSERT_TRUE(create_lexical_scopes(&context));

        {
            const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
            ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_definition->name.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        {
            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            const Ast_Function_Type* function_type = &function_definition->type->function;

            ASSERT_EQUAL(function_type->parameters_count, 2);

            {
                const Ast_Function_Parameter* first_parameter = &function_type->parameters[0];
                ASSERT_EQUAL(first_parameter->type->kind, AST_TYPE_NAME);

                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           first_parameter->type->named_type.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(first_parameter->type->symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "s32");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }

            {
                const Ast_Function_Parameter* second_parameter = &function_type->parameters[1];
                ASSERT_EQUAL(second_parameter->type->kind, AST_TYPE_NAME);

                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           second_parameter->type->named_type.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(second_parameter->type->symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "s32");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }

            ASSERT_EQUAL(function_type->return_type->kind, AST_TYPE_NAME);

            {
                const Symbol_Id symbol_id = find_symbol_id(&context,
                                                           GLOBAL_LEXICAL_SCOPE_ID,
                                                           function_type->return_type->named_type.token.lexeme);
                ASSERT_NOT_EQUAL(symbol_id, INVALID_SYMBOL_ID);
                ASSERT_EQUAL(function_type->return_type->symbol_id, symbol_id);

                const Symbol* symbol = &context.symbols[symbol_id];
                ASSERT_EQUAL(symbol->kind, SYMBOL_TYPE);
                ASSERT_STRINGS_ARE_EQUAL(symbol->name, "void");
                ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
                ASSERT_FALSE(symbol->is_mutable);
            }
        }

        const Lexical_Scope_Id function_scope_id = function_definition->body.lexical_scope_id;
        ASSERT_EQUAL(function_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);

        const Lexical_Scope* scope = &context.lexical_scopes[function_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 3);

        const Symbol_Id a_symbol_id = scope->symbol_ids[0];
        const Symbol_Id b_symbol_id = scope->symbol_ids[1];
        const Symbol_Id var_symbol_id = scope->symbol_ids[2];

        {
            const Symbol* a_symbol = &context.symbols[a_symbol_id];
            ASSERT_EQUAL(a_symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(a_symbol->name, "a");
            ASSERT_EQUAL(a_symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(a_symbol->is_mutable);
        }

        {
            const Symbol* b_symbol = &context.symbols[b_symbol_id];
            ASSERT_EQUAL(b_symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(b_symbol->name, "b");
            ASSERT_EQUAL(b_symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(b_symbol->is_mutable);
        }

        {
            const Symbol* var_symbol = &context.symbols[var_symbol_id];
            ASSERT_EQUAL(var_symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(var_symbol->name, "var");
            ASSERT_EQUAL(var_symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(var_symbol->is_mutable);
        }

        ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
        const Ast_Function_Type* function_type = &function_definition->type->function;

        ASSERT_EQUAL(function_type->parameters_count, 2);

        {
            const Ast_Function_Parameter* parameter = &function_type->parameters[0];
            ASSERT_EQUAL(parameter->name.symbol_id, a_symbol_id);
        }

        {
            const Ast_Function_Parameter* parameter = &function_type->parameters[1];
            ASSERT_EQUAL(parameter->name.symbol_id, b_symbol_id);
        }

        ASSERT_EQUAL(function_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &function_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, var_symbol_id);

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_EQUAL(initial_value->kind, AST_EXPRESSION_ADD);

            const Ast_Binary_Expression* add_expression = &initial_value->binary_expression;
            ASSERT_EQUAL(add_expression->lhs->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_EQUAL(add_expression->lhs->identifier.symbol_id, a_symbol_id);

            ASSERT_EQUAL(add_expression->rhs->kind, AST_EXPRESSION_MULTIPLY);

            const Ast_Binary_Expression* multiply_expression = &add_expression->rhs->binary_expression;
            ASSERT_EQUAL(multiply_expression->lhs->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_EQUAL(multiply_expression->lhs->identifier.symbol_id, b_symbol_id);

            ASSERT_EQUAL(multiply_expression->rhs->kind, AST_EXPRESSION_NUMBER);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {}"
                                                 "bar: () -> s32 = {}"
                                                 "baz: () -> void = {"
                                                 "    var := foo() + bar();"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 3);

        ASSERT_TRUE(create_lexical_scopes(&context));

        const Ast_Function_Definition* foo_definition = &context.ast.function_definitions[0];
        const Ast_Function_Definition* bar_definition = &context.ast.function_definitions[1];
        const Ast_Function_Definition* baz_definition = &context.ast.function_definitions[2];

        ASSERT_EQUAL(foo_definition->body.lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);
        ASSERT_EQUAL(bar_definition->body.lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 2);
        ASSERT_EQUAL(baz_definition->body.lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 3);

        const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
        ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        const Symbol_Id foo_symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       foo_definition->name.token.lexeme);
        const Symbol_Id bar_symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       bar_definition->name.token.lexeme);
        const Symbol_Id baz_symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       baz_definition->name.token.lexeme);

        ASSERT_NOT_EQUAL(foo_symbol_id, INVALID_SYMBOL_ID);
        ASSERT_NOT_EQUAL(bar_symbol_id, INVALID_SYMBOL_ID);
        ASSERT_NOT_EQUAL(baz_symbol_id, INVALID_SYMBOL_ID);

        {
            ASSERT_EQUAL(foo_definition->name.symbol_id, foo_symbol_id);

            const Symbol* symbol = &context.symbols[foo_symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        {
            ASSERT_EQUAL(bar_definition->name.symbol_id, bar_symbol_id);

            const Symbol* symbol = &context.symbols[bar_symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "bar");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        {
            ASSERT_EQUAL(baz_definition->name.symbol_id, baz_symbol_id);

            const Symbol* symbol = &context.symbols[baz_symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "baz");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id baz_lexical_scope_id = baz_definition->body.lexical_scope_id;

        const Lexical_Scope* scope = &context.lexical_scopes[baz_lexical_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        const Symbol_Id var_symbol_id = scope->symbol_ids[0];

        {
            const Symbol* var_symbol = &context.symbols[var_symbol_id];
            ASSERT_EQUAL(var_symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(var_symbol->name, "var");
            ASSERT_EQUAL(var_symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(var_symbol->is_mutable);
        }

        ASSERT_EQUAL(baz_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &baz_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, var_symbol_id);

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_EQUAL(initial_value->kind, AST_EXPRESSION_ADD);

            const Ast_Binary_Expression* add_expression = &initial_value->binary_expression;
            ASSERT_EQUAL(add_expression->lhs->kind, AST_EXPRESSION_CALL);
            ASSERT_EQUAL(add_expression->lhs->call.called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_EQUAL(add_expression->lhs->call.called_expression->identifier.symbol_id, foo_symbol_id);

            ASSERT_EQUAL(add_expression->rhs->kind, AST_EXPRESSION_CALL);
            ASSERT_EQUAL(add_expression->rhs->call.called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_EQUAL(add_expression->rhs->call.called_expression->identifier.symbol_id, bar_symbol_id);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing that the global symbols' order of definition is irrelevant.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("baz: () -> void = {"
                                                 "    var := foo() + bar();"
                                                 "}"
                                                 "foo: () -> s32 = {}"
                                                 "bar: () -> s32 = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_EQUAL(context.errors_count, 0);

        ASSERT_EQUAL(context.ast.function_definitions_count, 3);

        ASSERT_TRUE(create_lexical_scopes(&context));

        const Ast_Function_Definition* baz_definition = &context.ast.function_definitions[0];
        const Ast_Function_Definition* foo_definition = &context.ast.function_definitions[1];
        const Ast_Function_Definition* bar_definition = &context.ast.function_definitions[2];

        ASSERT_EQUAL(baz_definition->body.lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 1);
        ASSERT_EQUAL(foo_definition->body.lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 2);
        ASSERT_EQUAL(bar_definition->body.lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID + 3);

        const Lexical_Scope* global_scope = &context.lexical_scopes[GLOBAL_LEXICAL_SCOPE_ID];
        ASSERT_EQUAL(global_scope->parent_lexical_scope_id, INVALID_LEXICAL_SCOPE_ID);

        const Symbol_Id foo_symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       foo_definition->name.token.lexeme);
        const Symbol_Id bar_symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       bar_definition->name.token.lexeme);
        const Symbol_Id baz_symbol_id = find_symbol_id(&context,
                                                       GLOBAL_LEXICAL_SCOPE_ID,
                                                       baz_definition->name.token.lexeme);

        ASSERT_NOT_EQUAL(foo_symbol_id, INVALID_SYMBOL_ID);
        ASSERT_NOT_EQUAL(bar_symbol_id, INVALID_SYMBOL_ID);
        ASSERT_NOT_EQUAL(baz_symbol_id, INVALID_SYMBOL_ID);

        {
            ASSERT_EQUAL(foo_definition->name.symbol_id, foo_symbol_id);

            const Symbol* symbol = &context.symbols[foo_symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "foo");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        {
            ASSERT_EQUAL(bar_definition->name.symbol_id, bar_symbol_id);

            const Symbol* symbol = &context.symbols[bar_symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "bar");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        {
            ASSERT_EQUAL(baz_definition->name.symbol_id, baz_symbol_id);

            const Symbol* symbol = &context.symbols[baz_symbol_id];
            ASSERT_EQUAL(symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(symbol->name, "baz");
            ASSERT_EQUAL(symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(symbol->is_mutable);
        }

        const Lexical_Scope_Id baz_lexical_scope_id = baz_definition->body.lexical_scope_id;

        const Lexical_Scope* scope = &context.lexical_scopes[baz_lexical_scope_id];
        ASSERT_EQUAL(scope->parent_lexical_scope_id, GLOBAL_LEXICAL_SCOPE_ID);
        ASSERT_EQUAL(scope->symbol_ids_count, 1);

        const Symbol_Id var_symbol_id = scope->symbol_ids[0];

        {
            const Symbol* var_symbol = &context.symbols[var_symbol_id];
            ASSERT_EQUAL(var_symbol->kind, SYMBOL_VARIABLE);
            ASSERT_STRINGS_ARE_EQUAL(var_symbol->name, "var");
            ASSERT_EQUAL(var_symbol->type_id, INVALID_TYPE_ID);
            ASSERT_FALSE(var_symbol->is_mutable);
        }

        ASSERT_EQUAL(baz_definition->body.statements_count, 1);

        {
            const Ast_Statement* statement = &baz_definition->body.statements[0];
            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            ASSERT_EQUAL(variable_definition->name.symbol_id, var_symbol_id);

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_EQUAL(initial_value->kind, AST_EXPRESSION_ADD);

            const Ast_Binary_Expression* add_expression = &initial_value->binary_expression;
            ASSERT_EQUAL(add_expression->lhs->kind, AST_EXPRESSION_CALL);
            ASSERT_EQUAL(add_expression->lhs->call.called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_EQUAL(add_expression->lhs->call.called_expression->identifier.symbol_id, foo_symbol_id);

            ASSERT_EQUAL(add_expression->rhs->kind, AST_EXPRESSION_CALL);
            ASSERT_EQUAL(add_expression->rhs->call.called_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_EQUAL(add_expression->rhs->call.called_expression->identifier.symbol_id, bar_symbol_id);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_function_scopes,
    test_if_statements_scopes,
    test_while_loops_scopes,
    test_that_every_identifier_has_symbol_id_in_expressions
)

#include "eon_compilation_context.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
