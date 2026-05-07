#include "eon_unit_test.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"
#include "eon_types.h"

#define ASSERT_TYPE_IS_VALID(type_id)                   \
    do                                                  \
    {                                                   \
        ASSERT_NOT_EQUAL((type_id), UNDEFINED_TYPE_ID); \
        ASSERT_NOT_EQUAL((type_id), INVALID_TYPE_ID);   \
    }                                                   \
    while (0)

internal void
test_builtin_types_resolving(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    var := parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_FALSE(has_diagnostic_messages(&context));

        create_lexical_scopes(&context);
        ASSERT_FALSE(has_diagnostic_messages(&context));

        resolve_and_validate_types(&context);
        ASSERT_FALSE(has_diagnostic_messages(&context));

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_EQUAL(function_type->parent_type_id, UNDEFINED_TYPE_ID);

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_EQUAL(parameter_type->parent_type_id, UNDEFINED_TYPE_ID);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_EQUAL(return_type->parent_type_id, UNDEFINED_TYPE_ID);
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        // NOTE(vlad): Test that every variable in lexical scope has a type.
        {
        }

        // FIXME(vlad): Test lexical scope.
        // FIXME(vlad): Test that the returned type matches the required return type.

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        // ASSERT_EQUAL(body->every_path_returns, true); // FIXME(vlad): We should probably determine this during lexical scope analysis.

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_EQUAL(initial_value->type_id, parameter_type_id);
            ASSERT_EQUAL(initial_value->kind, AST_EXPRESSION_IDENTIFIER);

            const Ast_Identifier* identifier = &initial_value->identifier;
            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);
            ASSERT_EQUAL(identifier_symbol->type_id, parameter_type_id);

            const Ast_Identifier* variable = &variable_definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_EQUAL(find_root_type_id(&context, variable_symbol->type_id), parameter_type_id);

            const Ast_Type* type = variable_definition->type;
            ASSERT_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_EQUAL(find_root_type_id(&context, type->type_id), parameter_type_id);
        }

        // FIXME(vlad): Test return statement.

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

#if 0
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    var := parameter;\n"
                                                 "    return var;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_FALSE(has_diagnostic_messages(&context));

        create_lexical_scopes(&context);
        ASSERT_FALSE(has_diagnostic_messages(&context));

        resolve_and_validate_types(&context);
        ASSERT_FALSE(has_diagnostic_messages(&context));

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_EQUAL(function_type->parent_type_id, UNDEFINED_TYPE_ID);

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_EQUAL(parameter_type->parent_type_id, UNDEFINED_TYPE_ID);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_EQUAL(return_type->kind, TYPE_INTEGER);
            ASSERT_EQUAL(return_type->parent_type_id, UNDEFINED_TYPE_ID);
            ASSERT_TRUE(return_type->integer_info.is_signed);
            ASSERT_EQUAL(return_type->integer_info.width_in_bits, 32);
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        // FIXME(vlad): Test that the returned type matches the required return type.

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        // ASSERT_EQUAL(body->every_path_returns, true); // FIXME(vlad): We should probably determine this during lexical scope analysis.

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_EQUAL(initial_value->type_id, parameter_type_id);
            ASSERT_EQUAL(initial_value->kind, AST_EXPRESSION_IDENTIFIER);

            const Ast_Identifier* identifier = &initial_value->identifier;
            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);
            ASSERT_EQUAL(identifier_symbol->type_id, parameter_type_id);

            const Ast_Identifier* variable = &variable_definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_EQUAL(variable_symbol->type_id, parameter_type_id);

            const Ast_Type* type = variable_definition->type;
            ASSERT_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_EQUAL(type->type_id, parameter_type_id);
        }

        // FIXME(vlad): Test return statement.

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
#endif
}

REGISTER_TESTS(
    test_builtin_types_resolving
)

#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_types.c"
