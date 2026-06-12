#include "eon_unit_test.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"
#include "eon_types.h"

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
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "s32");
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_IDENTIFIER);

            const Ast_Identifier* identifier = &initial_value->identifier;
            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);
            ASSERT_TYPE_IDS_ARE_EQUAL(identifier_symbol->type_id, parameter_type_id);

            const Ast_Identifier* variable = &variable_definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TYPE_IDS_ARE_EQUAL(variable_symbol->type_id, parameter_type_id);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);

            ASSERT_TYPE_IDS_ARE_EQUAL(type->type_id, parameter_type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "s32");
        }

        // FIXME(vlad): Test return statement.

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

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
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> s32");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
            ASSERT_TRUE(return_type->integer_info.is_signed);
            ASSERT_EQUAL(return_type->integer_info.width_in_bits, 32);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        // FIXME(vlad): Test that the returned type matches the required return type.

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "s32");
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_IDENTIFIER);

            const Ast_Identifier* identifier = &initial_value->identifier;
            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);
            ASSERT_TYPE_IDS_ARE_EQUAL(identifier_symbol->type_id, parameter_type_id);

            const Ast_Identifier* variable = &variable_definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TYPE_IDS_ARE_EQUAL(variable_symbol->type_id, parameter_type_id);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_IDS_ARE_EQUAL(type->type_id, parameter_type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "s32");
        }

        // FIXME(vlad): Test return statement.

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_pointers(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> * s32 = {\n"
                                                 "    var := parameter&;\n"
                                                 "    return var;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> * s32");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_POINTER);

            ASSERT_FALSE(return_type->pointer_info.pointee_is_mutable);

            const Type_Id pointed_to_type_id = return_type->pointer_info.points_to_type_id;
            const Type* pointed_to_type = get_type_by_id(&context, pointed_to_type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(pointed_to_type->kind, TYPE_INTEGER);
            ASSERT_TRUE(pointed_to_type->integer_info.is_signed);
            ASSERT_EQUAL(pointed_to_type->integer_info.width_in_bits, 32);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(pointed_to_type_id, "s32");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "* s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: * s32) -> s32 = {\n"
                                                 "    var := parameter*;\n"
                                                 "    return var;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(* s32) -> s32");

        const Type* function_type = get_type_by_id(&context, function_type_id);
        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "* s32");

        const Type_Id return_type_id = info->return_type_id;
        ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_comparisons(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    var := parameter == parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "bool");

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_EQUAL);
            ASSERT_TYPE_IS_VALID(initial_value->type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "bool");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    var := parameter != parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "bool");

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_NOT_EQUAL);
            ASSERT_TYPE_IS_VALID(initial_value->type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "bool");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    var := parameter < parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "bool");

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_LESS);
            ASSERT_TYPE_IS_VALID(initial_value->type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "bool");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    var := parameter <= parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "bool");

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_LESS_OR_EQUAL);
            ASSERT_TYPE_IS_VALID(initial_value->type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "bool");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    var := parameter > parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "bool");

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_GREATER);
            ASSERT_TYPE_IS_VALID(initial_value->type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "bool");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    var := parameter >= parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Type* type = variable_definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(type->kind, AST_TYPE_OMITTED);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(type->type_id, "bool");

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_GREATER_OR_EQUAL);
            ASSERT_TYPE_IS_VALID(initial_value->type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "bool");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_if_statements(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    if parameter == parameter {}\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_IF);
            const Ast_If_Statement* if_statement = &statement->if_statement;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(if_statement->condition.type_id, "bool");

            ASSERT_FALSE(if_statement->if_statements.every_path_returns);
            ASSERT_FALSE(if_statement->else_statements.every_path_returns);

            ASSERT_FALSE(type_id_is_valid(&context, if_statement->if_statements.return_type_id));
            ASSERT_FALSE(type_id_is_valid(&context, if_statement->else_statements.return_type_id));
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    if parameter == parameter\n"
                                                 "    {\n"
                                                 "        return parameter;"
                                                 "    }\n"
                                                 "    else\n"
                                                 "    {\n"
                                                 "        return parameter;"
                                                 "    }\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> s32");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
            ASSERT_TRUE(return_type->integer_info.is_signed);
            ASSERT_EQUAL(return_type->integer_info.width_in_bits, 32);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_IF);
            const Ast_If_Statement* if_statement = &statement->if_statement;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(if_statement->condition.type_id, "bool");

            ASSERT_TRUE(if_statement->if_statements.every_path_returns);
            ASSERT_TRUE(if_statement->else_statements.every_path_returns);

            ASSERT_TYPE_STRINGS_ARE_EQUAL(if_statement->if_statements.return_type_id, "s32");
            ASSERT_TYPE_STRINGS_ARE_EQUAL(if_statement->else_statements.return_type_id, "s32");
        }

        ASSERT_TYPE_STRINGS_ARE_EQUAL(body->return_type_id, "s32");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    if parameter == parameter\n"
                                                 "    {\n"
                                                 "        return parameter;"
                                                 "    }\n"
                                                 "    return parameter;"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> s32");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
            ASSERT_TRUE(return_type->integer_info.is_signed);
            ASSERT_EQUAL(return_type->integer_info.width_in_bits, 32);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_IF);
            const Ast_If_Statement* if_statement = &statement->if_statement;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(if_statement->condition.type_id, "bool");

            ASSERT_TRUE(if_statement->if_statements.every_path_returns);
            ASSERT_FALSE(if_statement->else_statements.every_path_returns);

            ASSERT_TYPE_STRINGS_ARE_EQUAL(if_statement->if_statements.return_type_id, "s32");
            ASSERT_FALSE(type_id_is_valid(&context, if_statement->else_statements.return_type_id));
        }

        {
            const Ast_Statement* statement = &body->statements[1];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
            const Ast_Return_Statement* return_statement = &statement->return_statement;

            ASSERT_FALSE(return_statement->is_empty);

            const Ast_Expression* returned_expression = &return_statement->expression;

            ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s32");

            const Ast_Identifier* identifier = &returned_expression->identifier;

            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);

            ASSERT_TYPE_IS_VALID(identifier_symbol->type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(identifier_symbol->type_id, "s32");
        }

        ASSERT_TYPE_STRINGS_ARE_EQUAL(body->return_type_id, "s32");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_while_statements(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    while parameter == parameter {}\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_WHILE);
            const Ast_While_Statement* while_statement = &statement->while_statement;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(while_statement->condition.type_id, "bool");

            ASSERT_FALSE(while_statement->body.every_path_returns);
            ASSERT_FALSE(type_id_is_valid(&context, while_statement->body.return_type_id));
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    while parameter == parameter\n"
                                                 "    {\n"
                                                 "        return parameter;"
                                                 "    }\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32) -> s32");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TRUE(parameter_type->integer_info.is_signed);
                ASSERT_EQUAL(parameter_type->integer_info.width_in_bits, 32);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "s32");
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
            ASSERT_TRUE(return_type->integer_info.is_signed);
            ASSERT_EQUAL(return_type->integer_info.width_in_bits, 32);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_WHILE);
            const Ast_While_Statement* while_statement = &statement->while_statement;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(while_statement->condition.type_id, "bool");

            ASSERT_TRUE(while_statement->body.every_path_returns);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(while_statement->body.return_type_id, "s32");
        }

        ASSERT_TYPE_STRINGS_ARE_EQUAL(body->return_type_id, "s32");

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_assignments(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: mutable s32;\n"
                                                 "    a = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            ASSERT_FALSE(definition->has_initial_value);

            const Ast_Type* ast_type = definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
            ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location,
                                              "mutable s32");

            const Ast_Identifier* variable = &definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");
        }

        {
            const Ast_Statement* statement = &body->statements[1];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_ASSIGNMENT);
            const Ast_Assignment* assignment = &statement->assignment;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->lhs.type_id, "s32");
            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->rhs.type_id, "s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: mutable s32;\n"
                                                 "    a&* = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            ASSERT_FALSE(definition->has_initial_value);

            const Ast_Type* ast_type = definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
            ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location,
                                              "mutable s32");

            const Ast_Identifier* variable = &definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");
        }

        {
            const Ast_Statement* statement = &body->statements[1];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_ASSIGNMENT);
            const Ast_Assignment* assignment = &statement->assignment;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->lhs.type_id, "s32");
            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->rhs.type_id, "s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: mutable _ = 20;\n"
                                                 "    b := a;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            const Ast_Type* ast_type = definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
            ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location,
                                              "mutable _");

            const Ast_Identifier* variable = &definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TRUE(variable_symbol->binding_is_mutable);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");

            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(definition->initial_value.type_id, "s32");
        }

        {
            const Ast_Statement* statement = &body->statements[1];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            const Ast_Type* ast_type = definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_OMITTED);

            const Ast_Identifier* variable = &definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_FALSE(variable_symbol->binding_is_mutable);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");

            ASSERT_TRUE(definition->has_initial_value);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(definition->initial_value.type_id, "s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_pointers_to_mutable_binding(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: mutable s32;\n"
                                                 "    ptr := a&;"
                                                 "    ptr* = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 3);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            ASSERT_FALSE(definition->has_initial_value);

            const Ast_Type* ast_type = definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
            ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location,
                                              "mutable s32");

            const Ast_Identifier* variable = &definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");
        }

        {
            const Ast_Statement* statement = &body->statements[1];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            ASSERT_TRUE(definition->has_initial_value);

            const Ast_Expression* initial_value = &definition->initial_value;
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "* mutable s32");
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_ADDRESS_OF);

            const Ast_Unary_Expression* address_of = &initial_value->unary_expression;
            ASSERT_ENUM_VALUES_ARE_EQUAL(address_of->operand->kind, AST_EXPRESSION_IDENTIFIER);

            const Ast_Identifier* identifier = &address_of->operand->identifier;
            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(identifier_symbol->type_id, "s32");

            const Ast_Type* ast_type = definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_OMITTED);

            const Ast_Identifier* variable = &definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "* mutable s32");
        }

        {
            const Ast_Statement* statement = &body->statements[2];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_ASSIGNMENT);
            const Ast_Assignment* assignment = &statement->assignment;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->lhs.type_id, "s32");
            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->rhs.type_id, "s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("bar: (arg: * mutable s32) -> void = {}"
                                                 "foo: () -> void = {\n"
                                                 "    a: mutable s32;\n"
                                                 "    bar(a&);\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 2);

        // NOTE(vlad): Testing 'bar()'.
        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(* mutable s32) -> void");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 1);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);

                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
            }

            // NOTE(vlad): Test that every symbol has a type.
            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

                const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
                ASSERT_FALSE(parameter->has_default_value);

                const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_symbol->type_id, "* mutable s32");
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 0);
            ASSERT_EQUAL(body->every_path_returns, true);
        }

        // NOTE(vlad): Testing 'foo()'.
        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[1];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);

                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 2);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* definition = &statement->variable_definition;

                ASSERT_FALSE(definition->has_initial_value);

                const Ast_Type* ast_type = definition->type;
                ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
                ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location,
                                                  "mutable s32");

                const Ast_Identifier* variable = &definition->name;
                const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");
            }

            {
                const Ast_Statement* statement = &body->statements[1];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_CALL);

                const Ast_Call_Statement* call_statement = &statement->call_statement;
                const Ast_Expression* call_expression = &call_statement->call_expression;
                ASSERT_ENUM_VALUES_ARE_EQUAL(call_expression->kind, AST_EXPRESSION_CALL);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(call_expression->type_id, "void");

                const Ast_Call* call = &call_expression->call;

                const Ast_Expression* called_expression = call->called_expression;
                ASSERT_ENUM_VALUES_ARE_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(called_expression->type_id, "(* mutable s32) -> void");

                ASSERT_EQUAL(call->arguments_count, 1);
                const Ast_Expression* call_parameter = call->arguments[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(call_parameter->kind, AST_EXPRESSION_ADDRESS_OF);

                const Ast_Expression* address_of_operand = call_parameter->unary_expression.operand;
                ASSERT_ENUM_VALUES_ARE_EQUAL(address_of_operand->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(address_of_operand->type_id, "s32");

                ASSERT_TYPE_STRINGS_ARE_EQUAL(call_parameter->type_id, "* mutable s32");
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Decay from '* mutable s32' to '* s32' is allowed.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("bar: (arg: * s32) -> void = {}"
                                                 "foo: () -> void = {\n"
                                                 "    a: mutable s32;\n"
                                                 "    bar(a&);\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 2);

        // NOTE(vlad): Testing 'bar()'.
        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(* s32) -> void");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 1);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);

                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
            }

            // NOTE(vlad): Test that every symbol has a type.
            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

                const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
                ASSERT_FALSE(parameter->has_default_value);

                const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_symbol->type_id, "* s32");
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 0);
            ASSERT_EQUAL(body->every_path_returns, true);
        }

        // NOTE(vlad): Testing 'foo()'.
        {
            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[1];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);

                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 2);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* definition = &statement->variable_definition;

                ASSERT_FALSE(definition->has_initial_value);

                const Ast_Type* ast_type = definition->type;
                ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
                ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location,
                                                  "mutable s32");

                const Ast_Identifier* variable = &definition->name;
                const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");
            }

            {
                const Ast_Statement* statement = &body->statements[1];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_CALL);

                const Ast_Call_Statement* call_statement = &statement->call_statement;
                const Ast_Expression* call_expression = &call_statement->call_expression;
                ASSERT_ENUM_VALUES_ARE_EQUAL(call_expression->kind, AST_EXPRESSION_CALL);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(call_expression->type_id, "void");

                const Ast_Call* call = &call_expression->call;

                const Ast_Expression* called_expression = call->called_expression;
                ASSERT_ENUM_VALUES_ARE_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(called_expression->type_id, "(* s32) -> void");

                ASSERT_EQUAL(call->arguments_count, 1);
                const Ast_Expression* call_parameter = call->arguments[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(call_parameter->kind, AST_EXPRESSION_ADDRESS_OF);

                const Ast_Expression* address_of_operand = call_parameter->unary_expression.operand;
                ASSERT_ENUM_VALUES_ARE_EQUAL(address_of_operand->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(address_of_operand->type_id, "s32");

                ASSERT_TYPE_STRINGS_ARE_EQUAL(call_parameter->type_id, "* mutable s32");
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_calls(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: * () -> s32) -> s32 = {\n"
                                                 "    return (parameter*)();\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(* () -> s32) -> s32");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 1);

        const Type_Id parameter_type_id = info->parameter_type_ids[0];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_POINTER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(parameter_type_id, "* () -> s32");
                ASSERT_FALSE(parameter_type->pointer_info.pointee_is_mutable);
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 1);

            const Ast_Function_Parameter* parameter = &function_definition->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_for_identifier(&context, &parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(parameter_symbol->type_id, parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
            const Ast_Return_Statement* return_statement = &statement->return_statement;

            ASSERT_FALSE(return_statement->is_empty);

            const Ast_Expression* returned_expression = &return_statement->expression;

            ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_CALL);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s32");

            const Ast_Expression* called_expression = returned_expression->call.called_expression;

            ASSERT_ENUM_VALUES_ARE_EQUAL(called_expression->kind, AST_EXPRESSION_DEREFERENCE);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(called_expression->type_id, "() -> s32");

            const Ast_Expression* dereferenced_expression = called_expression->unary_expression.operand;

            ASSERT_ENUM_VALUES_ARE_EQUAL(dereferenced_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(dereferenced_expression->type_id, "* () -> s32");

            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, &dereferenced_expression->identifier);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(identifier_symbol->type_id, "* () -> s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32, function: * (arg: s32) -> s32) -> s32 = {\n"
                                                 "    return (function*)(parameter);\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        parse_ast(&parser);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "(s32, * (s32) -> s32) -> s32");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 2);

        const Type_Id first_parameter_type_id = info->parameter_type_ids[0];
        const Type_Id second_parameter_type_id = info->parameter_type_ids[1];
        const Type_Id return_type_id = info->return_type_id;

        // NOTE(vlad): Test that every subtype in function type is defined.
        {
            {
                ASSERT_TYPE_IS_VALID(first_parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, first_parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(first_parameter_type_id, "s32");
            }

            {
                ASSERT_TYPE_IS_VALID(second_parameter_type_id);
                const Type* parameter_type = get_type_by_id(&context, second_parameter_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(parameter_type->kind, TYPE_POINTER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(second_parameter_type_id, "* (s32) -> s32");
                ASSERT_FALSE(parameter_type->pointer_info.pointee_is_mutable);
            }

            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 2);

            const Ast_Function_Parameter* first_parameter = &function_definition->type->function.parameters[0];
            const Symbol* first_parameter_symbol = get_symbol_for_identifier(&context, &first_parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(first_parameter_symbol->type_id, first_parameter_type_id);

            const Ast_Function_Parameter* second_parameter = &function_definition->type->function.parameters[1];
            const Symbol* second_parameter_symbol = get_symbol_for_identifier(&context, &second_parameter->name);
            ASSERT_TYPE_IDS_ARE_EQUAL(second_parameter_symbol->type_id, second_parameter_type_id);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
            const Ast_Return_Statement* return_statement = &statement->return_statement;

            ASSERT_FALSE(return_statement->is_empty);

            const Ast_Expression* returned_expression = &return_statement->expression;

            ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_CALL);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s32");

            const Ast_Expression* called_expression = returned_expression->call.called_expression;

            ASSERT_ENUM_VALUES_ARE_EQUAL(called_expression->kind, AST_EXPRESSION_DEREFERENCE);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(called_expression->type_id, "(s32) -> s32");

            const Ast_Expression* dereferenced_expression = called_expression->unary_expression.operand;

            ASSERT_ENUM_VALUES_ARE_EQUAL(dereferenced_expression->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(dereferenced_expression->type_id, "* (s32) -> s32");

            const Symbol* identifier_symbol = get_symbol_for_identifier(&context, &dereferenced_expression->identifier);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(identifier_symbol->type_id, "* (s32) -> s32");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    return bar();\n"
                                                 "}\n"
                                                 "bar: () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 2);

        // NOTE(vlad): Testing 'foo'.

        const Ast_Function_Definition* first_function = &context.ast.function_definitions[0];

        const Type_Id first_function_type_id = first_function->type->type_id;
        ASSERT_TYPE_IS_VALID(first_function_type_id);

        const Type* first_function_type = get_type_by_id(&context, first_function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(first_function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(first_function_type_id, "() -> void");

        const Function_Type_Info* first_function_type_info = &first_function_type->function_info;
        ASSERT_EQUAL(first_function_type_info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = first_function_type_info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &first_function->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, first_function->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(first_function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(first_function->type->kind, AST_TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_symbol->type_id, "() -> void");
        }

        {
            const Ast_Code_Block* body = &first_function->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_CALL);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "void");

                const Ast_Expression* called_expression = returned_expression->call.called_expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(called_expression->type_id, "() -> void");

                const Ast_Identifier* identifier = &called_expression->identifier;

                const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);

                ASSERT_TYPE_IS_VALID(identifier_symbol->type_id);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(identifier_symbol->type_id, "() -> void");
            }

            ASSERT_TYPE_IS_VALID(body->return_type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(body->return_type_id, "void");
        }

        // NOTE(vlad): Testing 'bar'.

        const Ast_Function_Definition* second_function = &context.ast.function_definitions[1];

        const Type_Id second_function_type_id = second_function->type->type_id;
        ASSERT_TYPE_IS_VALID(second_function_type_id);

        const Type* second_function_type = get_type_by_id(&context, second_function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(second_function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(second_function_type_id, "() -> void");

        const Function_Type_Info* second_function_type_info = &second_function_type->function_info;
        ASSERT_EQUAL(second_function_type_info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = second_function_type_info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &second_function->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, second_function->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(second_function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(second_function->type->kind, AST_TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_symbol->type_id, "() -> void");
        }

        {
            const Ast_Code_Block* body = &second_function->body;

            ASSERT_EQUAL(body->statements_count, 0);
            ASSERT_EQUAL(body->every_path_returns, true);

            ASSERT_TYPE_IS_VALID(body->return_type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(body->return_type_id, "void");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    bar();\n"
                                                 "}\n"
                                                 "bar: () -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 2);

        // NOTE(vlad): Testing 'foo'.

        const Ast_Function_Definition* first_function = &context.ast.function_definitions[0];

        const Type_Id first_function_type_id = first_function->type->type_id;
        ASSERT_TYPE_IS_VALID(first_function_type_id);

        const Type* first_function_type = get_type_by_id(&context, first_function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(first_function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(first_function_type_id, "() -> void");

        const Function_Type_Info* first_function_type_info = &first_function_type->function_info;
        ASSERT_EQUAL(first_function_type_info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = first_function_type_info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &first_function->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, first_function->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(first_function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(first_function->type->kind, AST_TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_symbol->type_id, "() -> void");
        }

        {
            const Ast_Code_Block* body = &first_function->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_CALL);
                const Ast_Call_Statement* call_statement = &statement->call_statement;

                const Ast_Expression* call_expression = &call_statement->call_expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(call_expression->kind, AST_EXPRESSION_CALL);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(call_expression->type_id, "void");

                const Ast_Expression* called_expression = call_expression->call.called_expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(called_expression->kind, AST_EXPRESSION_IDENTIFIER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(called_expression->type_id, "() -> void");

                const Ast_Identifier* identifier = &called_expression->identifier;

                const Symbol* identifier_symbol = get_symbol_for_identifier(&context, identifier);

                ASSERT_TYPE_IS_VALID(identifier_symbol->type_id);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(identifier_symbol->type_id, "() -> void");
            }

            ASSERT_TYPE_IS_VALID(body->return_type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(body->return_type_id, "void");
        }

        // NOTE(vlad): Testing 'bar'.

        const Ast_Function_Definition* second_function = &context.ast.function_definitions[1];

        const Type_Id second_function_type_id = second_function->type->type_id;
        ASSERT_TYPE_IS_VALID(second_function_type_id);

        const Type* second_function_type = get_type_by_id(&context, second_function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(second_function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(second_function_type_id, "() -> void");

        const Function_Type_Info* second_function_type_info = &second_function_type->function_info;
        ASSERT_EQUAL(second_function_type_info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = second_function_type_info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &second_function->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, second_function->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(second_function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(second_function->type->kind, AST_TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_symbol->type_id, "() -> void");
        }

        {
            const Ast_Code_Block* body = &second_function->body;

            ASSERT_EQUAL(body->statements_count, 0);
            ASSERT_EQUAL(body->every_path_returns, true);

            ASSERT_TYPE_IS_VALID(body->return_type_id);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(body->return_type_id, "void");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    foo_ptr := foo&;"
                                                 "    (foo_ptr*)();\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 0);

        const Type_Id return_type_id = info->return_type_id;

        {
            ASSERT_TYPE_IS_VALID(return_type_id);
            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        // NOTE(vlad): Test that every symbol has a type.
        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;

            const Ast_Identifier* name = &variable_definition->name;
            const Symbol* name_symbol = get_symbol_for_identifier(&context, name);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(name_symbol->type_id, "* () -> void");

            ASSERT_TRUE(variable_definition->has_initial_value);

            const Ast_Expression* initial_value = &variable_definition->initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_ADDRESS_OF);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "* () -> void");

            const Ast_Unary_Expression* address_of = &initial_value->unary_expression;
            ASSERT_ENUM_VALUES_ARE_EQUAL(address_of->operand->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(address_of->operand->type_id, "() -> void");
        }

        {
            const Ast_Statement* statement = &body->statements[1];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_CALL);
            const Ast_Expression* call_expression = &statement->call_statement.call_expression;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(call_expression->type_id, "void");

            ASSERT_ENUM_VALUES_ARE_EQUAL(call_expression->kind, AST_EXPRESSION_CALL);
            const Ast_Call* call = &call_expression->call;

            ASSERT_EQUAL(call->arguments_count, 0);

            const Ast_Expression* called_expression = call->called_expression;
            ASSERT_ENUM_VALUES_ARE_EQUAL(called_expression->kind, AST_EXPRESSION_DEREFERENCE);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(called_expression->type_id, "() -> void");

            const Ast_Unary_Expression* dereference = &called_expression->unary_expression;
            ASSERT_ENUM_VALUES_ARE_EQUAL(dereference->operand->kind, AST_EXPRESSION_IDENTIFIER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(dereference->operand->type_id, "* () -> void");
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_signed_integers(Test_Context* test_context)
{
    // NOTE(vlad): Testing positive signed integers.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s8 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> s8");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s8");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s8");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s16 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> s16");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s16");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s16");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> s32");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s32");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s32");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s64 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> s64");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s64");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s64");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }
}

internal void
test_unsigned_integers(Test_Context* test_context)
{
    // NOTE(vlad): Testing unsigned integers.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> u8 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> u8");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "u8");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "u8");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> u16 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> u16");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "u16");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "u16");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> u32 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> u32");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "u32");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "u32");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s64 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> s64");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_INTEGER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "s64");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "s64");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }
}

internal void
test_floats(Test_Context* test_context)
{
    // NOTE(vlad): Testing integer literals.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> f32 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> f32");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_FLOAT);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "f32");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "f32");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> f64 = {\n"
                                                     "    return 1;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> f64");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_FLOAT);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "f64");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "f64");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }

    // NOTE(vlad): Testing floating-point literals.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> f32 = {\n"
                                                     "    return 1.0;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> f32");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_FLOAT);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "f32");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "f32");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> f64 = {\n"
                                                     "    return 1.0;\n"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> f64");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_FLOAT);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "f64");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 1);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
                const Ast_Return_Statement* return_statement = &statement->return_statement;

                ASSERT_FALSE(return_statement->is_empty);

                const Ast_Expression* returned_expression = &return_statement->expression;

                ASSERT_ENUM_VALUES_ARE_EQUAL(returned_expression->kind, AST_EXPRESSION_NUMBER);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(returned_expression->type_id, "f64");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }

    // NOTE(vlad): Testing that number types are determined correctly.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                     "    a: mutable _ = 10;\n"
                                                     "    b: f64 = 10.0;\n"
                                                     "    a = b;\n"
                                                     "}\n");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            ASSERT_EQUAL(context.ast.function_definitions_count, 1);

            const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

            const Type_Id function_type_id = function_definition->type->type_id;
            ASSERT_TYPE_IS_VALID(function_type_id);

            const Type* function_type = get_type_by_id(&context, function_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

            const Function_Type_Info* info = &function_type->function_info;
            ASSERT_EQUAL(info->parameter_type_ids_count, 0);

            {
                const Type_Id return_type_id = info->return_type_id;
                ASSERT_TYPE_IS_VALID(return_type_id);
                const Type* return_type = get_type_by_id(&context, return_type_id);
                ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
            }

            {
                const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
                ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
                ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
                ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

                ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
                ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
            }

            const Ast_Code_Block* body = &function_definition->body;

            ASSERT_EQUAL(body->statements_count, 3);
            ASSERT_EQUAL(body->every_path_returns, true);

            {
                const Ast_Statement* statement = &body->statements[0];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* definition = &statement->variable_definition;

                ASSERT_TRUE(definition->has_initial_value);

                const Ast_Expression* initial_value = &definition->initial_value;
                ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "f64");
                ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_NUMBER);

                const Ast_Type* ast_type = definition->type;
                ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
                ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location, "mutable _");

                const Ast_Identifier* variable = &definition->name;
                const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "f64");
            }

            {
                const Ast_Statement* statement = &body->statements[1];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
                const Ast_Variable_Definition* definition = &statement->variable_definition;

                ASSERT_TRUE(definition->has_initial_value);

                const Ast_Expression* initial_value = &definition->initial_value;
                ASSERT_TYPE_STRINGS_ARE_EQUAL(initial_value->type_id, "f64");
                ASSERT_ENUM_VALUES_ARE_EQUAL(initial_value->kind, AST_EXPRESSION_NUMBER);

                const Ast_Type* ast_type = definition->type;
                ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);
                ASSERT_LOCATION_STRINGS_ARE_EQUAL(&ast_type->location, "f64");

                const Ast_Identifier* variable = &definition->name;
                const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
                ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "f64");
            }

            {
                const Ast_Statement* statement = &body->statements[2];

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_ASSIGNMENT);
                const Ast_Assignment* assignment = &statement->assignment;

                ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->lhs.type_id, "f64");
                ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->rhs.type_id, "f64");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }
}

internal void
test_number_types_inference_in_arithmetic_expressions(Test_Context* test_context)
{
    struct Number_Test_Info
    {
        String_View source_code;
        String_View expected_type;
    };
    typedef struct Number_Test_Info Number_Test_Info;

    // NOTE(vlad): Testing that number type inference works correctly in assignments.

#define DECLARE_BINARY_OPERATION_TEST_INFO(Type, operation)         \
    (Number_Test_Info){                                             \
        .source_code = string_view("foo: () -> void = {\n"          \
                                   "    a: mutable " #Type ";\n"    \
                                   "    a = 2 " operation " 2;\n"   \
                                   "}"),                            \
        .expected_type = string_view(#Type),                        \
    },

    const Number_Test_Info assignment_test_infos[] = {
        FOR_EACH_INTEGER_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "+")
        FOR_EACH_FLOAT_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "+")

        FOR_EACH_INTEGER_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "-")
        FOR_EACH_FLOAT_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "-")

        FOR_EACH_INTEGER_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "*")
        FOR_EACH_FLOAT_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "*")

        FOR_EACH_INTEGER_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "/")
        FOR_EACH_FLOAT_TYPE(DECLARE_BINARY_OPERATION_TEST_INFO, "/")
    };
#undef DECLARE_BINARY_OPERATION_TEST_INFO

    for (Index i = 0;
         i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(assignment_test_infos);
         ++i)
    {
        Number_Test_Info test_info = assignment_test_infos[i];

        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE(test_info.source_code);

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);
        ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, "() -> void");

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = info->return_type_id;
            ASSERT_TYPE_IS_VALID(return_type_id);

            const Type* return_type = get_type_by_id(&context, return_type_id);
            ASSERT_ENUM_VALUES_ARE_EQUAL(return_type->kind, TYPE_VOID);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, "void");
        }

        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 2);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);
            const Ast_Variable_Definition* definition = &statement->variable_definition;

            ASSERT_FALSE(definition->has_initial_value);

            const Ast_Type* ast_type = definition->type;
            ASSERT_ENUM_VALUES_ARE_EQUAL(ast_type->kind, AST_TYPE_NAME);

            const Ast_Identifier* variable = &definition->name;
            const Symbol* variable_symbol = get_symbol_for_identifier(&context, variable);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, test_info.expected_type);
            ASSERT_TRUE(variable_symbol->binding_is_mutable);
        }

        {
            const Ast_Statement* statement = &body->statements[1];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_ASSIGNMENT);
            const Ast_Assignment* assignment = &statement->assignment;

            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->lhs.type_id, test_info.expected_type);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(assignment->rhs.type_id, test_info.expected_type);

            const Ast_Expression* rhs = &assignment->rhs;
            switch (rhs->kind)
            {
                case AST_EXPRESSION_ADD:
                case AST_EXPRESSION_SUBTRACT:
                case AST_EXPRESSION_MULTIPLY:
                case AST_EXPRESSION_DIVIDE:
                {
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            const Ast_Binary_Expression* binary_expression = &rhs->binary_expression;
            ASSERT_ENUM_VALUES_ARE_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(binary_expression->lhs->type_id, test_info.expected_type);

            ASSERT_ENUM_VALUES_ARE_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(binary_expression->rhs->type_id, test_info.expected_type);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing that number type inference works correctly in return statements.

#define DECLARE_RETURN_TEST_INFO(Type, operation)                    \
    (Number_Test_Info){                                              \
        .source_code = string_view("foo: () -> " #Type " = {\n"      \
                                   "    return 2 " operation " 2;\n" \
                                   "}"),                             \
        .expected_type = string_view(#Type),                         \
    },

    const Number_Test_Info return_test_infos[] = {
        FOR_EACH_INTEGER_TYPE(DECLARE_RETURN_TEST_INFO, "+")
        FOR_EACH_FLOAT_TYPE(DECLARE_RETURN_TEST_INFO, "+")

        FOR_EACH_INTEGER_TYPE(DECLARE_RETURN_TEST_INFO, "-")
        FOR_EACH_FLOAT_TYPE(DECLARE_RETURN_TEST_INFO, "-")

        FOR_EACH_INTEGER_TYPE(DECLARE_RETURN_TEST_INFO, "*")
        FOR_EACH_FLOAT_TYPE(DECLARE_RETURN_TEST_INFO, "*")

        FOR_EACH_INTEGER_TYPE(DECLARE_RETURN_TEST_INFO, "/")
        FOR_EACH_FLOAT_TYPE(DECLARE_RETURN_TEST_INFO, "/")
    };
#undef DECLARE_RETURN_TEST_INFO

    for (Index i = 0;
         i < NUMBER_OF_STATIC_ARRAY_ELEMENTS(return_test_infos);
         ++i)
    {
        Number_Test_Info test_info = return_test_infos[i];
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE(test_info.source_code);

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        ASSERT_EQUAL(context.ast.function_definitions_count, 1);

        const Ast_Function_Definition* function_definition = &context.ast.function_definitions[0];

        const Type_Id function_type_id = function_definition->type->type_id;
        ASSERT_TYPE_IS_VALID(function_type_id);

        const Type* function_type = get_type_by_id(&context, function_type_id);
        ASSERT_ENUM_VALUES_ARE_EQUAL(function_type->kind, TYPE_FUNCTION);

        {
            String_Builder builder = {0};
            create_string_builder(&builder, test_context->arena);

            append_string(&builder, string_view("() -> "));
            append_string(&builder, test_info.expected_type);

            ASSERT_TYPE_STRINGS_ARE_EQUAL(function_type_id, string_builder_to_string(&builder));
        }

        const Function_Type_Info* info = &function_type->function_info;
        ASSERT_EQUAL(info->parameter_type_ids_count, 0);

        {
            const Type_Id return_type_id = info->return_type_id;
            ASSERT_TYPE_STRINGS_ARE_EQUAL(return_type_id, test_info.expected_type);
        }

        {
            const Symbol* function_symbol = get_symbol_for_identifier(&context, &function_definition->name);
            ASSERT_ENUM_VALUES_ARE_EQUAL(function_symbol->kind, SYMBOL_FUNCTION);
            ASSERT_STRINGS_ARE_EQUAL(function_symbol->name, function_definition->name.token.lexeme);
            ASSERT_TYPE_IDS_ARE_EQUAL(function_type_id, function_symbol->type_id);

            ASSERT_ENUM_VALUES_ARE_EQUAL(function_definition->type->kind, AST_TYPE_FUNCTION);
            ASSERT_EQUAL(function_definition->type->function.parameters_count, 0);
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_RETURN);
            const Ast_Return_Statement* return_statement = &statement->return_statement;

            ASSERT_FALSE(return_statement->is_empty);

            const Ast_Expression* returned_expression = &return_statement->expression;

            switch (returned_expression->kind)
            {
                case AST_EXPRESSION_ADD:
                case AST_EXPRESSION_SUBTRACT:
                case AST_EXPRESSION_MULTIPLY:
                case AST_EXPRESSION_DIVIDE:
                {
                } break;

                default:
                {
                    UNREACHABLE();
                } break;
            }

            const Ast_Binary_Expression* binary_expression = &returned_expression->binary_expression;
            ASSERT_ENUM_VALUES_ARE_EQUAL(binary_expression->lhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(binary_expression->lhs->type_id, test_info.expected_type);

            ASSERT_ENUM_VALUES_ARE_EQUAL(binary_expression->rhs->kind, AST_EXPRESSION_NUMBER);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(binary_expression->rhs->type_id, test_info.expected_type);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_type_mismatches(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s64) -> s32 = {\n"
                                                 "    return parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:12: error: Return type mismatch: expected 's32', got 's64'\n"
                                                        "  2 |     return parameter;\n"
                                                        "    |            ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        // TODO(vlad): Test that some types were actually inferred.

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s64) -> s32 = {\n"
                                                 "    variable := parameter;\n"
                                                 "    return variable;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:12: error: Return type mismatch: expected 's32', got 's64'\n"
                                                        "  3 |     return variable;\n"
                                                        "    |            ^~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        // TODO(vlad): Test that some types were actually inferred.

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: * s32) -> s32 = {\n"
                                                 "    return parameter;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:12: error: Return type mismatch: expected 's32', got '* s32'\n"
                                                        "  2 |     return parameter;\n"
                                                        "    |            ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        // TODO(vlad): Test that some types were actually inferred.

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    var := parameter*;\n"
                                                 "    return var;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:12: error: Cannot dereference a non-pointer type 's32'\n"
                                                        "  2 |     var := parameter*;\n"
                                                        "    |            ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing that return statements are checked even if one of expressions has an invalid type id.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    if parameter == parameter { return parameter; }\n"
                                                 "    var := parameter*;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:12: error: Cannot dereference a non-pointer type 's32'\n"
                                                        "  3 |     var := parameter*;\n"
                                                        "    |            ^~~~~~~~~\n"
                                                        "<test-input>:4:1: error: Non-void function does not return a value in all control paths\n"
                                                        "  4 | }\n"
                                                        "    | ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing function calls.

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    return parameter();\n"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:12: error: Expression of type 's32' is not callable\n"
                                                        "  2 |     return parameter();\n"
                                                        "    |            ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    parameter();\n"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:5: error: Expression of type 's32' is not callable\n"
                                                        "  2 |     parameter();\n"
                                                        "    |     ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    return bar(parameter);\n"
                                                 "}\n"
                                                 "bar: (parameter: s64) -> s64 = {\n"
                                                 "    return parameter;"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:16: error: Passing expression of type 's32' to parameter of type 's64'\n"
                                                        "  2 |     return bar(parameter);\n"
                                                        "    |                ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    return bar(parameter, parameter);\n"
                                                 "}\n"
                                                 "bar: (parameter: s32) -> s32 = {\n"
                                                 "    return parameter;"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:12: error: Too many arguments provided to a function call, expected 1, got 2\n"
                                                        "  2 |     return bar(parameter, parameter);\n"
                                                        "    |            ^~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:1: error: Non-void function does not return a value in all control paths\n"
                                                        "  2 | }\n"
                                                        "    | ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    if parameter == parameter {\n"
                                                 "        return parameter;\n"
                                                 "    }\n"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:5:1: error: Non-void function does not return a value in all control paths\n"
                                                        "  5 | }\n"
                                                        "    | ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {\n"
                                                 "    if parameter {}\n"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:8: error: Implicit conversion from 's32' to 'bool' is forbidden.\n"
                                                        "  2 |     if parameter {}\n"
                                                        "    |        ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing assignment mismatches.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: s32 = 10;\n"
                                                 "    a = 2.0;\n"
                                                 "}\n");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:9: error: Type mismatch: expected 's32', got 'f32'\n"
                                                        "  3 |     a = 2.0;\n"
                                                        "    |         ^~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_number_type_mismatches(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a := 10;\n"
                                                 "    b: f32 = a;\n"
                                                 "    return a;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:12: error: Return type mismatch: expected 's32', got 'f32'\n"
                                                        "  4 |     return a;\n"
                                                        "    |            ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    return 10.0;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:12: error: Return type mismatch: expected 's32', got 'f32'\n"
                                                        "  2 |     return 10.0;\n"
                                                        "    |            ^~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s8 = {\n"
                                                 "    return 128;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        // FIXME(vlad): Come up with a better error message like "value '128' do not fit into 's8' (max s8 value = 127)"
        //              I think that in this case we need to return something
        //              more sophisticated than 'Bool' from 'try_to_unify_types()'.
        const String_View expected_output = string_view("<test-input>:2:12: error: Return type mismatch: expected 's8', got 's32'\n"
                                                        "  2 |     return 128;\n"
                                                        "    |            ^~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: s8 = 10;\n"
                                                 "    b: s32 = 20;\n"
                                                 "    return a + b;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:12: error: Cannot add expressions of different types 's8' and 's32'\n"
                                                        "  4 |     return a + b;\n"
                                                        "    |            ^~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: s8 = 10;\n"
                                                 "    ptr := a&;\n"
                                                 "    return ptr*;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:12: error: Return type mismatch: expected 's32', got 's8'\n"
                                                        "  4 |     return ptr*;\n"
                                                        "    |            ^~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: s8 = 10;\n"
                                                 "    b: f32 = 2;\n"
                                                 "    c := a + 2 * b;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:10: error: Cannot add expressions of different types 's8' and 'f32'\n"
                                                        "  4 |     c := a + 2 * b;\n"
                                                        "    |          ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: s8 = 10;\n"
                                                 "    b: f32 = 2;\n"
                                                 "    c := (a + 2) * b;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:10: error: Cannot multiply expressions of different types 's8' and 'f32'\n"
                                                        "  4 |     c := (a + 2) * b;\n"
                                                        "    |          ^~~~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_lvalue_mismatches(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    10 = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:5: error: lvalue is required as a LHS of assignment\n"
                                                        "  2 |     10 = 10;\n"
                                                        "    |     ^~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a := 10;\n"
                                                 "    a + 1 = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:5: error: lvalue is required as a LHS of assignment\n"
                                                        "  3 |     a + 1 = 10;\n"
                                                        "    |     ^~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a := 10;\n"
                                                 "    a& = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:10: error: Type mismatch: expected '* s32', got 's32'\n"
                                                        "  3 |     a& = 10;\n"
                                                        "    |          ^~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_mutability_mismatches(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: s32;\n"
                                                 "    a = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:5: error: Read-only location is not assignable\n"
                                                        "  3 |     a = 10;\n"
                                                        "    |     ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: s32;\n"
                                                 "    ptr := a&;\n"
                                                 "    ptr* = 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:5: error: Read-only location is not assignable\n"
                                                        "  4 |     ptr* = 10;\n"
                                                        "    |     ^~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("bar: (arg: * mutable s32) -> void = {}"
                                                 "foo: () -> void = {\n"
                                                 "    a: s32;\n"
                                                 "    bar(a&);\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:9: error: Passing expression of type '* s32' to parameter of type '* mutable s32'\n"
                                                        "  3 |     bar(a&);\n"
                                                        "    |         ^~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: s32;\n"
                                                 "    ptr: * mutable _ = a&;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:3:24: error: Cannot initialise variable of type '* mutable _' with expression of type '* s32'\n"
                                                        "  3 |     ptr: * mutable _ = a&;\n"
                                                        "    |                        ^~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 "    b := a;\n"
                                                 "    ptr: * mutable _ = b&;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:24: error: Cannot initialise variable of type '* mutable _' with expression of type '* s32'\n"
                                                        "  4 |     ptr: * mutable _ = b&;\n"
                                                        "    |                        ^~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 "    immutable_ptr: * _ = a&;\n"
                                                 "    mutable_ptr: * mutable _ = immutable_ptr;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                &context,
                                                                MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:4:32: error: Cannot initialise variable of type '* mutable _' with expression of type '* s32'\n"
                                                        "  4 |     mutable_ptr: * mutable _ = immutable_ptr;\n"
                                                        "    |                                ^~~~~~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_builtin_types_resolving,
    test_pointers,
    test_comparisons,
    test_if_statements,
    test_while_statements,
    test_assignments,
    test_pointers_to_mutable_binding,
    test_calls,
    test_signed_integers,
    test_unsigned_integers,
    test_floats,
    test_number_types_inference_in_arithmetic_expressions,
    test_type_mismatches,
    test_number_type_mismatches,
    test_lvalue_mismatches,
    test_mutability_mismatches
)

#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_types.c"
