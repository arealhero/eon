#include "eon_unit_test.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"
#include "eon_types.h"

#define ASSERT_TYPE_IS_VALID(type_id)           \
    ASSERT_TRUE(type_id_is_valid(&context, type_id))

#define ASSERT_TYPE_IDS_ARE_EQUAL(lhs, rhs) \
    ASSERT_TRUE(type_ids_are_equal(&context, (lhs), (rhs)))

#define ASSERT_TYPE_STRINGS_ARE_EQUAL(type_id, expected)                \
    do                                                                  \
    {                                                                   \
        ASSERT_TYPE_IS_VALID(type_id);                                  \
        ASSERT_STRINGS_ARE_EQUAL(convert_type_to_string(test_context->arena, \
                                                        &context,       \
                                                        type_id),       \
                                 expected);                             \
    }                                                                   \
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

        // NOTE(vlad): Test that every variable in lexical scope has a type.
        {
        }

        const Ast_Code_Block* body = &function_definition->body;

        ASSERT_EQUAL(body->statements_count, 1);
        ASSERT_EQUAL(body->every_path_returns, true);

        {
            const Ast_Statement* statement = &body->statements[0];

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_VARIABLE_DEFINITION);
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
test_function_calls(Test_Context* test_context)
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_RETURN);
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

            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_RETURN);
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

                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->type, AST_STATEMENT_RETURN);
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
}

internal void
test_types_mismatches(Test_Context* test_context)
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

    // FIXME(vlad): Enable this test.
#if 0
    // NOTE(vlad): Testing that return statements are not checked if one of expressions has an invalid type id.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> s32 = {\n"
                                                 "    if 1 == 1 { return parameter; }"
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
#endif

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

    // FIXME(vlad): Enable this test.
#if 0
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
#endif
}

REGISTER_TESTS(
    test_builtin_types_resolving,
    test_pointers,
    test_comparisons,
    test_function_calls,
    test_types_mismatches
)

#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_types.c"
