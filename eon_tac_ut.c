#include "eon_unit_test.h"

#include "eon_tac.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"
#include "eon_types.h"

internal void
test_function_parameters_lowering(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {"
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

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Ast* ast = &context.ast;
        ASSERT_EQUAL(ast->function_definitions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_EQUAL(tac_function->label.id, 1);
        ASSERT_EQUAL(tac_function->label.symbol_id, ast_function->name.symbol_id);

        ASSERT_EQUAL(tac_function->instructions_count, 1);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_GET_PARAMETER);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_EQUAL(destination->variable.id, 1);
            ASSERT_FALSE(destination->variable.is_temporary);

            const Symbol_Id first_parameter_symbol_id = ast_function->type->function.parameters[0].name.symbol_id;
            const Symbol* first_parameter_symbol = get_symbol_by_id(&context, first_parameter_symbol_id);

            ASSERT_EQUAL(destination->variable.type_id.index, first_parameter_symbol->type_id.index);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_PARAMETER_INDEX);
            ASSERT_EQUAL(first_argument->parameter_index.index, 0);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_expression_lowering(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 10;"
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

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Ast* ast = &context.ast;
        ASSERT_EQUAL(ast->function_definitions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_EQUAL(tac_function->label.id, 1);
        ASSERT_EQUAL(tac_function->label.symbol_id, ast_function->name.symbol_id);

        ASSERT_EQUAL(tac_function->instructions_count, 1);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_EQUAL(destination->variable.id, 1);
            ASSERT_FALSE(destination->variable.is_temporary);

            const Symbol* variable_symbol = get_symbol_by_id(&context, destination->variable.symbol_id);
            ASSERT_STRINGS_ARE_EQUAL(variable_symbol->name, "a");
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(first_argument->constant.id, 1);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(first_argument->constant.type_id, "s32");
            ASSERT_STRINGS_ARE_EQUAL(first_argument->constant.ast_number->token.lexeme, "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a := 10 + 20;"
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

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Ast* ast = &context.ast;
        ASSERT_EQUAL(ast->function_definitions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_EQUAL(tac_function->label.id, 1);
        ASSERT_EQUAL(tac_function->label.symbol_id, ast_function->name.symbol_id);

        ASSERT_EQUAL(tac_function->instructions_count, 2);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ADD);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_EQUAL(destination->variable.id, 1);
            ASSERT_TRUE(destination->variable.is_temporary);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(destination->variable.type_id, "s32");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(first_argument->constant.id, 1);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(first_argument->constant.type_id, "s32");
            ASSERT_STRINGS_ARE_EQUAL(first_argument->constant.ast_number->token.lexeme, "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_EQUAL(second_argument->constant.id, 2);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(second_argument->constant.type_id, "s32");
            ASSERT_STRINGS_ARE_EQUAL(second_argument->constant.ast_number->token.lexeme, "20");
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[1];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_EQUAL(destination->variable.id, 2);
            ASSERT_FALSE(destination->variable.is_temporary);

            const Symbol* variable_symbol = get_symbol_by_id(&context, destination->variable.symbol_id);
            ASSERT_STRINGS_ARE_EQUAL(variable_symbol->name, "a");
            ASSERT_TYPE_STRINGS_ARE_EQUAL(variable_symbol->type_id, "s32");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_EQUAL(first_argument->variable.id, 1);
            ASSERT_TRUE(first_argument->variable.is_temporary);
            ASSERT_TYPE_STRINGS_ARE_EQUAL(destination->variable.type_id, "s32");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_function_parameters_lowering,
    test_expression_lowering
)

#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_tac.c"
#include "eon_types.c"
