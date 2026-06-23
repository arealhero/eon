#include "eon_unit_test.h"

#include "eon_tac.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"
#include "eon_types.h"

#define ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(function_label_id, ast_function) \
    do                                                                  \
    {                                                                   \
        ASSERT_NOT_EQUAL(function_label_id.index, 0);                   \
        const Tac_Function_Label* label = get_tac_function_label_by_id(&context.tac, \
                                                                       function_label_id); \
        ASSERT_EQUAL(label->symbol_id, ast_function->name.symbol_id);   \
    }                                                                   \
    while (0)

#define ASSERT_VARIABLE_POINTS_TO_SYMBOL(variable_id, expected_symbol_id) \
    do                                                                  \
    {                                                                   \
        const Tac_Variable* variable = get_tac_variable_by_id(&context.tac, variable_id); \
        ASSERT_FALSE(variable->is_temporary);                           \
                                                                        \
        const Symbol* symbol = get_symbol_by_id(&context, expected_symbol_id); \
        ASSERT_EQUAL(variable->symbol_id, expected_symbol_id);          \
        ASSERT_TYPE_IDS_ARE_EQUAL(variable->type_id, symbol->type_id);  \
    }                                                                   \
    while (0)

#define ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(variable_id, expected_type)  \
    do                                                                  \
    {                                                                   \
        const Tac_Variable* variable = get_tac_variable_by_id(&context.tac, variable_id); \
        ASSERT_TRUE(variable->is_temporary);                            \
        ASSERT_TYPE_STRINGS_ARE_EQUAL(variable->type_id, expected_type); \
    }                                                                   \
    while (0)

#define ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(constant_id, expected_type, expected_value) \
    do                                                                  \
    {                                                                   \
        ASSERT_NOT_EQUAL(constant_id.index, 0);                         \
        const Tac_Constant* constant = get_tac_constant_by_id(&context.tac, constant_id); \
        ASSERT_TYPE_STRINGS_ARE_EQUAL(constant->type_id, expected_type); \
        ASSERT_STRINGS_ARE_EQUAL(constant->ast_number->token.lexeme, expected_value); \
    }                                                                   \
    while (0)

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
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 2);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_GET_PARAMETER);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_EQUAL(destination->variable_id.index, 1);

            const Symbol_Id first_parameter_symbol_id = ast_function->type->function.parameters[0].name.symbol_id;
            ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id, first_parameter_symbol_id);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_PARAMETER_INDEX);
            ASSERT_EQUAL(first_argument->parameter_index.index, 0);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[1];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        // NOTE(vlad): Testing that parameter symbol has a non-empty TAC instruction id.
        {
            const Ast_Function_Parameter* parameter = &ast_function->type->function.parameters[0];
            const Symbol* parameter_symbol = get_symbol_by_id(&context, parameter->name.symbol_id);
            ASSERT_EQUAL(parameter_symbol->tac_instruction_id.function_label_id.index, 1);
            ASSERT_EQUAL(parameter_symbol->tac_instruction_id.instruction_index, 0);
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
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 2);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                ASSERT_EQUAL(ast_function->body.statements_count, 1);
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[1];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        // NOTE(vlad): Testing that variable symbol has a non-empty TAC instruction id.
        {
            ASSERT_EQUAL(ast_function->body.statements_count, 1);
            const Ast_Statement* statement = &ast_function->body.statements[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            const Symbol_Id variable_symbol_id = variable_definition->name.symbol_id;
            const Symbol* variable_symbol = get_symbol_by_id(&context, variable_symbol_id);

            ASSERT_EQUAL(variable_symbol->tac_instruction_id.function_label_id.index, 1);
            ASSERT_EQUAL(variable_symbol->tac_instruction_id.instruction_index, 0);
        }

        // NOTE(vlad): Testing that every statement and expression has a valid TAC instructions range.
        {
            const Ast_Code_Block* ast_function_body = &ast_function->body;
            ASSERT_EQUAL(ast_function_body->statements_count, 1);

            const Ast_Statement* statement = &ast_function_body->statements[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

            {
                const Tac_Instructions_Range* instructions_range = &statement->tac_instructions_range;
                ASSERT_EQUAL(instructions_range->function_label_id.index, tac_function->label_id.index);
                ASSERT_EQUAL(instructions_range->start_instruction_index, 0);
                ASSERT_EQUAL(instructions_range->end_instruction_index, 1);
            }

            ASSERT_TRUE(statement->variable_definition.has_initial_value);

            const Ast_Expression* initial_expression = &statement->variable_definition.initial_value;
            ASSERT_ENUM_VALUES_ARE_EQUAL(initial_expression->kind, AST_EXPRESSION_NUMBER);

            {
                const Tac_Instructions_Range* instructions_range = &initial_expression->tac_instructions_range;
                ASSERT_EQUAL(instructions_range->function_label_id.index, tac_function->label_id.index);
                ASSERT_EQUAL(instructions_range->start_instruction_index, 0);

                // FIXME(vlad): Should numbers have their own instruction? Like 'ASSIGN temp, CONSTANT'. We will be able
                //              to get rid of these temp variables later (in SSA, during constant folding).
                ASSERT_EQUAL(instructions_range->end_instruction_index, 0);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a: mutable _ = 10;"
                                                 "    a = 20;"
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
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        Symbol_Id variable_symbol_id = 0;
        {
            ASSERT_EQUAL(ast_function->body.statements_count, 2);
            const Ast_Statement* statement = &ast_function->body.statements[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            variable_symbol_id = variable_definition->name.symbol_id;
        }
        ASSERT_NOT_EQUAL(variable_symbol_id, 0);

        ASSERT_EQUAL(tac_function->instructions_count, 3);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id, variable_symbol_id);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[1];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id, variable_symbol_id);
            ASSERT_EQUAL(destination->variable_id.index, tac_function->instructions[0].destination.variable_id.index);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "20");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[2];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        // NOTE(vlad): Testing that variable symbol has a non-empty TAC instruction id.
        {
            const Symbol* variable_symbol = get_symbol_by_id(&context, variable_symbol_id);

            ASSERT_EQUAL(variable_symbol->tac_instruction_id.function_label_id.index, 1);
            ASSERT_EQUAL(variable_symbol->tac_instruction_id.instruction_index, 0);
        }

        // NOTE(vlad): Testing that every statement and expression has a valid TAC instructions range.
        {
            const Ast_Code_Block* ast_function_body = &ast_function->body;
            ASSERT_EQUAL(ast_function_body->statements_count, 2);

            {
                const Ast_Statement* statement = &ast_function_body->statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                {
                    const Tac_Instructions_Range* instructions_range = &statement->tac_instructions_range;
                    ASSERT_EQUAL(instructions_range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_EQUAL(instructions_range->start_instruction_index, 0);
                    ASSERT_EQUAL(instructions_range->end_instruction_index, 1);
                }

                ASSERT_TRUE(statement->variable_definition.has_initial_value);

                const Ast_Expression* initial_expression = &statement->variable_definition.initial_value;
                ASSERT_ENUM_VALUES_ARE_EQUAL(initial_expression->kind, AST_EXPRESSION_NUMBER);

                {
                    const Tac_Instructions_Range* instructions_range = &initial_expression->tac_instructions_range;
                    ASSERT_EQUAL(instructions_range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_EQUAL(instructions_range->start_instruction_index, 0);
                    ASSERT_EQUAL(instructions_range->end_instruction_index, 0);
                }
            }

            {
                const Ast_Statement* statement = &ast_function_body->statements[1];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_ASSIGNMENT);

                {
                    const Tac_Instructions_Range* instructions_range = &statement->tac_instructions_range;
                    ASSERT_EQUAL(instructions_range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_EQUAL(instructions_range->start_instruction_index, 1);
                    ASSERT_EQUAL(instructions_range->end_instruction_index, 2);
                }

                {
                    const Ast_Expression* lhs = &statement->assignment.lhs;
                    ASSERT_ENUM_VALUES_ARE_EQUAL(lhs->kind, AST_EXPRESSION_IDENTIFIER);

                    {
                        const Tac_Instructions_Range* instructions_range = &lhs->tac_instructions_range;
                        ASSERT_EQUAL(instructions_range->function_label_id.index, tac_function->label_id.index);
                        ASSERT_EQUAL(instructions_range->start_instruction_index, 1);
                        ASSERT_EQUAL(instructions_range->end_instruction_index, 1);
                    }
                }

                {
                    const Ast_Expression* lhs = &statement->assignment.rhs;
                    ASSERT_ENUM_VALUES_ARE_EQUAL(lhs->kind, AST_EXPRESSION_NUMBER);

                    {
                        const Tac_Instructions_Range* instructions_range = &lhs->tac_instructions_range;
                        ASSERT_EQUAL(instructions_range->function_label_id.index, tac_function->label_id.index);
                        ASSERT_EQUAL(instructions_range->start_instruction_index, 1);
                        ASSERT_EQUAL(instructions_range->end_instruction_index, 1);
                    }
                }
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_return_statements_lowering(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
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
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 1);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): We do not perform dead code elimination during TAC construction
    //             because we cannot determine if the code is actually reachable (e.g. via goto).
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    return;"
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
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 2);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[1];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    return 10;"
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
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 1);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    a := 10;"
                                                 "    return a;"
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
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        Symbol_Id variable_symbol_id = 0;
        {
            ASSERT_EQUAL(ast_function->body.statements_count, 2);
            const Ast_Statement* statement = &ast_function->body.statements[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            variable_symbol_id = variable_definition->name.symbol_id;
        }
        ASSERT_NOT_EQUAL(variable_symbol_id, 0);

        ASSERT_EQUAL(tac_function->instructions_count, 2);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id, variable_symbol_id);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[1];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_VARIABLE_POINTS_TO_SYMBOL(first_argument->variable_id, variable_symbol_id);
            ASSERT_EQUAL(first_argument->variable_id.index, tac_function->instructions[0].destination.variable_id.index);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        // NOTE(vlad): Testing that variable symbol has a non-empty TAC instruction id.
        {
            const Symbol* variable_symbol = get_symbol_by_id(&context, variable_symbol_id);

            ASSERT_EQUAL(variable_symbol->tac_instruction_id.function_label_id.index, 1);
            ASSERT_EQUAL(variable_symbol->tac_instruction_id.instruction_index, 0);
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
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    return 10;"
                                                 "}"
                                                 "bar: () -> s32 = {"
                                                 "    return foo();"
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
        ASSERT_EQUAL(ast->function_definitions_count, 2);

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 2);

        {
            const Ast_Function_Definition* ast_foo_function = &ast->function_definitions[0];

            const Tac_Function* tac_foo_function = &tac->functions[0];
            ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_foo_function->label_id, ast_foo_function);

            ASSERT_EQUAL(tac_foo_function->instructions_count, 1);

            {
                const Tac_Instruction* instruction = &tac_foo_function->instructions[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

                const Tac_Operand* destination = &instruction->destination;
                ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

                const Tac_Operand* first_argument = &instruction->first_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
                ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

                const Tac_Operand* second_argument = &instruction->second_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

                ASSERT_FALSE(instruction->was_automatically_inserted);
            }
        }

        {
            const Ast_Function_Definition* ast_bar_function = &ast->function_definitions[1];

            const Tac_Function* tac_bar_function = &tac->functions[1];
            ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_bar_function->label_id, ast_bar_function);

            ASSERT_EQUAL(tac_bar_function->instructions_count, 2);

            {
                const Tac_Instruction* instruction = &tac_bar_function->instructions[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_CALL);

                const Tac_Operand* destination = &instruction->destination;
                ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
                ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "s32");

                const Tac_Operand* first_argument = &instruction->first_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_FUNCTION_LABEL);
                {
                    const Ast_Function_Definition* foo_definition = &ast->function_definitions[0];
                    const Symbol* foo_symbol = get_symbol_by_id(&context, foo_definition->name.symbol_id);
                    ASSERT_EQUAL(first_argument->function_label_id.index,
                                 foo_symbol->tac_instruction_id.function_label_id.index);
                }

                const Tac_Operand* second_argument = &instruction->second_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

                ASSERT_FALSE(instruction->was_automatically_inserted);
            }

            {
                const Tac_Instruction* instruction = &tac_bar_function->instructions[1];
                ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

                const Tac_Operand* destination = &instruction->destination;
                ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

                const Tac_Operand* first_argument = &instruction->first_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
                ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, "s32");
                {
                    const Tac_Instruction* first_instruction = &tac_bar_function->instructions[0];
                    ASSERT_EQUAL(first_argument->variable_id.index, first_instruction->destination.variable_id.index);
                }

                const Tac_Operand* second_argument = &instruction->second_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

                ASSERT_FALSE(instruction->was_automatically_inserted);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    return 10;"
                                                 "}"
                                                 "bar: () -> void = {"
                                                 "    foo();"
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
        ASSERT_EQUAL(ast->function_definitions_count, 2);

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 2);

        {
            const Ast_Function_Definition* ast_foo_function = &ast->function_definitions[0];

            const Tac_Function* tac_foo_function = &tac->functions[0];
            ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_foo_function->label_id, ast_foo_function);

            ASSERT_EQUAL(tac_foo_function->instructions_count, 1);

            {
                const Tac_Instruction* instruction = &tac_foo_function->instructions[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

                const Tac_Operand* destination = &instruction->destination;
                ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

                const Tac_Operand* first_argument = &instruction->first_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
                ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

                const Tac_Operand* second_argument = &instruction->second_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

                ASSERT_FALSE(instruction->was_automatically_inserted);
            }
        }

        {
            const Ast_Function_Definition* ast_bar_function = &ast->function_definitions[1];

            const Tac_Function* tac_bar_function = &tac->functions[1];
            ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_bar_function->label_id, ast_bar_function);

            ASSERT_EQUAL(tac_bar_function->instructions_count, 2);

            {
                const Tac_Instruction* instruction = &tac_bar_function->instructions[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_CALL);

                const Tac_Operand* destination = &instruction->destination;
                ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
                ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "s32");

                const Tac_Operand* first_argument = &instruction->first_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_FUNCTION_LABEL);
                {
                    const Ast_Function_Definition* foo_definition = &ast->function_definitions[0];
                    const Symbol* foo_symbol = get_symbol_by_id(&context, foo_definition->name.symbol_id);
                    ASSERT_EQUAL(first_argument->function_label_id.index,
                                 foo_symbol->tac_instruction_id.function_label_id.index);
                }

                const Tac_Operand* second_argument = &instruction->second_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

                ASSERT_FALSE(instruction->was_automatically_inserted);
            }

            {
                const Tac_Instruction* instruction = &tac_bar_function->instructions[1];
                ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

                const Tac_Operand* destination = &instruction->destination;
                ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

                const Tac_Operand* first_argument = &instruction->first_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

                const Tac_Operand* second_argument = &instruction->second_argument;
                ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

                ASSERT_TRUE(instruction->was_automatically_inserted);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_binary_expressions_lowering(Test_Context* test_context)
{
    struct Number_Test_Info
    {
        String_View source_code;
        Tac_Operation expected_operation;
        String_View expected_type;
    };
    typedef struct Number_Test_Info Number_Test_Info;

#define DECLARE_BINARY_OPERATION_TEST_INFO(operator, tac_operation, type) \
    (Number_Test_Info){                                                   \
        .source_code = string_view("foo: () -> void = {\n"                \
                                   "    a := 10 " operator " 20;\n"       \
                                   "}"),                                  \
        .expected_operation = tac_operation,                              \
        .expected_type = string_view(type),                               \
    }

    const Number_Test_Info assignment_test_infos[] = {
        DECLARE_BINARY_OPERATION_TEST_INFO("+", TAC_ADD, "s32"),
        DECLARE_BINARY_OPERATION_TEST_INFO("-", TAC_SUBTRACT, "s32"),
        DECLARE_BINARY_OPERATION_TEST_INFO("*", TAC_MULTIPLY, "s32"),
        DECLARE_BINARY_OPERATION_TEST_INFO("/", TAC_DIVIDE, "s32"),

        DECLARE_BINARY_OPERATION_TEST_INFO("==", TAC_EQUAL, "bool"),
        DECLARE_BINARY_OPERATION_TEST_INFO("!=", TAC_NOT_EQUAL, "bool"),
        DECLARE_BINARY_OPERATION_TEST_INFO("<", TAC_LESS, "bool"),
        DECLARE_BINARY_OPERATION_TEST_INFO("<=", TAC_LESS_OR_EQUAL, "bool"),
        DECLARE_BINARY_OPERATION_TEST_INFO(">", TAC_GREATER, "bool"),
        DECLARE_BINARY_OPERATION_TEST_INFO(">=", TAC_GREATER_OR_EQUAL, "bool"),
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

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Ast* ast = &context.ast;
        ASSERT_EQUAL(ast->function_definitions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 3);

        {
            const Tac_Instruction* instruction = &tac_function->instructions[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, test_info.expected_operation);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, test_info.expected_type);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(second_argument->constant_id, "s32", "20");

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[1];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                ASSERT_EQUAL(ast_function->body.statements_count, 1);
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, test_info.expected_type);
            ASSERT_EQUAL(first_argument->variable_id.index,
                         tac_function->instructions[0].destination.variable_id.index);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        {
            const Tac_Instruction* instruction = &tac_function->instructions[2];
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        // NOTE(vlad): Testing that variable symbol has a non-empty TAC instruction id.
        {
            ASSERT_EQUAL(ast_function->body.statements_count, 1);
            const Ast_Statement* statement = &ast_function->body.statements[0];
            ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

            const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
            const Symbol_Id variable_symbol_id = variable_definition->name.symbol_id;
            const Symbol* variable_symbol = get_symbol_by_id(&context, variable_symbol_id);

            ASSERT_EQUAL(variable_symbol->tac_instruction_id.function_label_id.index, 1);
            ASSERT_EQUAL(variable_symbol->tac_instruction_id.instruction_index, 1);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_while_loops_lowering(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    while 1 != 2"
                                                 "    {"
                                                 "        a := 10;"
                                                 "    }"
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

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 7);

        const Tac_Instruction* instruction = &tac_function->instructions[0];

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_LABEL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 1);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 0);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_NOT_EQUAL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "bool");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "1");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(second_argument->constant_id, "s32", "2");

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_JUMP_IF_FALSE);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 2);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 5);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, "bool");
            ASSERT_EQUAL(first_argument->variable_id.index, 1);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                ASSERT_EQUAL(ast_function->body.statements_count, 1);
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_WHILE);

                const Ast_Code_Block* while_body = &statement->while_statement.body;
                ASSERT_EQUAL(while_body->statements_count, 1);

                const Ast_Statement* substatement = &while_body->statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(substatement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &substatement->variable_definition;
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_JUMP);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 1);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 0);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_LABEL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 2);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 5);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(instruction + 1, tac_function->instructions),
                     tac_function->instructions_count);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_if_statements_lowering(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    if 1 != 2"
                                                 "    {"
                                                 "        a := 10;"
                                                 "    }"
                                                 "    else"
                                                 "    {"
                                                 "        b := 20;"
                                                 "    }"
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

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 8);

        const Tac_Instruction* instruction = &tac_function->instructions[0];

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_NOT_EQUAL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "bool");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "1");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(second_argument->constant_id, "s32", "2");

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_JUMP_IF_FALSE);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 1);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 4);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, "bool");
            ASSERT_EQUAL(first_argument->variable_id.index, 1);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                ASSERT_EQUAL(ast_function->body.statements_count, 1);
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_IF);

                const Ast_Code_Block* if_block = &statement->if_statement.if_statements;
                ASSERT_EQUAL(if_block->statements_count, 1);

                const Ast_Statement* substatement = &if_block->statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(substatement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &substatement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_JUMP);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 2);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 6);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_LABEL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 1);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 4);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                ASSERT_EQUAL(ast_function->body.statements_count, 1);
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_IF);

                const Ast_Code_Block* else_block = &statement->if_statement.else_statements;
                ASSERT_EQUAL(else_block->statements_count, 1);

                const Ast_Statement* substatement = &else_block->statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(substatement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &substatement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "b");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "20");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_LABEL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 2);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 6);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(instruction + 1, tac_function->instructions),
                     tac_function->instructions_count);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    if 1 != 2"
                                                 "    {"
                                                 "        a := 10;"
                                                 "    }"
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

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(tac_function->instructions_count, 7);

        const Tac_Instruction* instruction = &tac_function->instructions[0];

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_NOT_EQUAL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "bool");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "1");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(second_argument->constant_id, "s32", "2");

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_JUMP_IF_FALSE);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 1);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 4);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, "bool");
            ASSERT_EQUAL(first_argument->variable_id.index, 1);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                ASSERT_EQUAL(ast_function->body.statements_count, 1);
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_IF);

                const Ast_Code_Block* if_block = &statement->if_statement.if_statements;
                ASSERT_EQUAL(if_block->statements_count, 1);

                const Ast_Statement* substatement = &if_block->statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(substatement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &substatement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_JUMP);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 2);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 5);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_LABEL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 1);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 4);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_LABEL);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_LABEL);
            {
                ASSERT_EQUAL(destination->label_id.index, 2);
                const Tac_Label* label = get_tac_label_by_id(&context.tac, destination->label_id);
                ASSERT_EQUAL(label->instruction_id.function_label_id.index, 1);
                ASSERT_FALSE(label->instruction_id.is_a_global_function);
                ASSERT_EQUAL(label->instruction_id.instruction_index, 5);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_NONE);

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_TRUE(instruction->was_automatically_inserted);
        }

        ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(instruction + 1, tac_function->instructions),
                     tac_function->instructions_count);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_indirect_memory_access(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    a := 10;"
                                                 "    ptr := a&;"
                                                 "    return ptr*;"
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

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(ast_function->body.statements_count, 3);
        ASSERT_EQUAL(tac_function->instructions_count, 5);

        const Tac_Instruction* instruction = &tac_function->instructions[0];

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_GET_ADDRESS);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "* s32");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            {
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(first_argument->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                const Ast_Statement* statement = &ast_function->body.statements[1];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "ptr");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, "* s32");
            {
                const Tac_Instruction* previous_instruction = instruction - 1;
                ASSERT_EQUAL(first_argument->variable_id.index, previous_instruction->destination.variable_id.index);
            }

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_LOAD_BY_ADDRESS);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "s32");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            {
                const Ast_Statement* statement = &ast_function->body.statements[1];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "ptr");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(first_argument->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, "s32");
            {
                const Tac_Instruction* previous_instruction = (instruction - 1);
                ASSERT_EQUAL(first_argument->variable_id.index, previous_instruction->destination.variable_id.index);
            }

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(instruction + 1, tac_function->instructions),
                     tac_function->instructions_count);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    a: mutable _ = 10;"
                                                 "    ptr := a&;"
                                                 "    ptr* = 20;"
                                                 "    return a;"
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

        const Tac* tac = &context.tac;
        ASSERT_EQUAL(tac->functions_count, 1);

        const Ast_Function_Definition* ast_function = &ast->function_definitions[0];

        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_FUNCTION_LABEL_POINTS_TO_FUNCTION(tac_function->label_id, ast_function);

        ASSERT_EQUAL(ast_function->body.statements_count, 4);
        ASSERT_EQUAL(tac_function->instructions_count, 5);

        const Tac_Instruction* instruction = &tac_function->instructions[0];

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "10");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_GET_ADDRESS);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(destination->variable_id, "* mutable s32");

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            {
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(first_argument->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);

            {
                const Ast_Statement* statement = &ast_function->body.statements[1];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "ptr");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            ASSERT_TEMPORARY_VARIABLE_HAS_TYPE(first_argument->variable_id, "* mutable s32");
            {
                const Tac_Instruction* previous_instruction = instruction - 1;
                ASSERT_EQUAL(first_argument->variable_id.index, previous_instruction->destination.variable_id.index);
            }

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_STORE_BY_ADDRESS);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_VARIABLE);
            {
                const Ast_Statement* statement = &ast_function->body.statements[1];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "ptr");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(destination->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_CONSTANT);
            ASSERT_CONSTANT_HAS_NUMERIC_VALUE_AND_TYPE(first_argument->constant_id, "s32", "20");

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        instruction += 1;

        {
            ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);

            const Tac_Operand* destination = &instruction->destination;
            ASSERT_ENUM_VALUES_ARE_EQUAL(destination->kind, TAC_OPERAND_NONE);

            const Tac_Operand* first_argument = &instruction->first_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(first_argument->kind, TAC_OPERAND_VARIABLE);
            {
                const Ast_Statement* statement = &ast_function->body.statements[0];
                ASSERT_ENUM_VALUES_ARE_EQUAL(statement->kind, AST_STATEMENT_VARIABLE_DEFINITION);

                const Ast_Variable_Definition* variable_definition = &statement->variable_definition;
                ASSERT_STRINGS_ARE_EQUAL(variable_definition->name.token.lexeme, "a");
                ASSERT_VARIABLE_POINTS_TO_SYMBOL(first_argument->variable_id,
                                                 variable_definition->name.symbol_id);
            }

            const Tac_Operand* second_argument = &instruction->second_argument;
            ASSERT_ENUM_VALUES_ARE_EQUAL(second_argument->kind, TAC_OPERAND_NONE);

            ASSERT_FALSE(instruction->was_automatically_inserted);
        }

        ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(instruction + 1, tac_function->instructions),
                     tac_function->instructions_count);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_function_parameters_lowering,
    test_expression_lowering,
    test_return_statements_lowering,
    test_calls,
    test_binary_expressions_lowering,
    test_while_loops_lowering,
    test_if_statements_lowering,
    test_indirect_memory_access
)

#include "eon_cfg.c"
#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_tac.c"
#include "eon_types.c"
