#include "eon_unit_test.h"

#include "eon_cfg.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"
#include "eon_types.h"

internal void
test_functions_without_jumps(Test_Context* test_context)
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_EQUAL(tac_function->instructions_count, 1);

        ASSERT_EQUAL(cfg->blocks_count, 2);
        ASSERT_EQUAL(cfg->edges_count, 0);

        const Cfg_Block* block = &cfg->blocks[INVALID_CFG_BLOCK_INDEX + 1];
        {
            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            ASSERT_EQUAL(block->instructions_range.end_instruction_index - block->instructions_range.start_instruction_index,
                         tac_function->instructions_count);
        }

        // NOTE(vlad): Test that entry block was determined correctly.
        {
            const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
            ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    a := 10;"
                                                 "    b := 20;"
                                                 "    return a + b;"
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(cfg->blocks_count, 2);
        ASSERT_EQUAL(cfg->edges_count, 0);

        const Cfg_Block* block = &cfg->blocks[INVALID_CFG_BLOCK_INDEX + 1];
        {
            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            ASSERT_EQUAL(block->instructions_range.end_instruction_index - block->instructions_range.start_instruction_index,
                         tac_function->instructions_count);
        }

        // NOTE(vlad): Test that entry block was determined correctly.
        {
            const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
            ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {"
                                                 "    a := 10;"
                                                 "    b := 20;"
                                                 "    return bar(a + b);"
                                                 "}"
                                                 "bar: (number: s32) -> s32 = {"
                                                 "    return 2 * number;"
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 2);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(cfg->blocks_count, 3);
        ASSERT_EQUAL(cfg->edges_count, 0);

        const Cfg_Block* block = &cfg->blocks[INVALID_CFG_BLOCK_INDEX + 1];
        {
            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            ASSERT_EQUAL(block->instructions_range.end_instruction_index - block->instructions_range.start_instruction_index,
                         tac_function->instructions_count);
        }

        tac_function += 1;
        block += 1;

        {
            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            ASSERT_EQUAL(block->instructions_range.end_instruction_index - block->instructions_range.start_instruction_index,
                         tac_function->instructions_count);
        }

        // NOTE(vlad): Test that entry block was determined correctly.
        {
            // NOTE(vlad): Testing 'foo()'.
            {
                const Tac_Function* current_function = &tac->functions[0];
                const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac,
                                                                                        current_function->label_id);
                ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
            }

            // NOTE(vlad): Testing 'bar()'.
            {
                const Tac_Function* current_function = &tac->functions[1];
                const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac,
                                                                                        current_function->label_id);
                ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 2);
            }
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
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    if 1 != 2"
                                                 "    {"
                                                 "        a := 10;"
                                                 "    }"
                                                 "    else"
                                                 "    {"
                                                 "        b := 10;"
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(cfg->blocks_count, 5);
        ASSERT_EQUAL(cfg->edges_count, 4);

        const Index condition_block_index = 1;
        const Index then_block_index = 2;
        const Index else_block_index = 3;
        const Index final_block_index = 4;

        {
            const Cfg_Block* block = &cfg->blocks[condition_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);
        }

        {
            const Cfg_Block* block = &cfg->blocks[then_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);
        }

        {
            const Cfg_Block* block = &cfg->blocks[else_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);
        }

        {
            const Cfg_Block* block = &cfg->blocks[final_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[0];
            ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, else_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[1];
            ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, then_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[2];
            ASSERT_EQUAL(edge->source_block_id.index, then_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[3];
            ASSERT_EQUAL(edge->source_block_id.index, else_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
        }

        // NOTE(vlad): Test that entry block was determined correctly.
        {
            const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
            ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
        }

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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(cfg->blocks_count, 5);
        ASSERT_EQUAL(cfg->edges_count, 4);

        const Index condition_block_index = 1;
        const Index then_block_index = 2;
        const Index else_block_index = 3;
        const Index final_block_index = 4;

        {
            const Cfg_Block* block = &cfg->blocks[condition_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);
        }

        {
            const Cfg_Block* block = &cfg->blocks[then_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);
        }

        {
            const Cfg_Block* block = &cfg->blocks[else_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

            const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                                                       - block->instructions_range.start_instruction_index;
            ASSERT_EQUAL(this_block_instructions_count, 1);
        }

        {
            const Cfg_Block* block = &cfg->blocks[final_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[0];
            ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, else_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[1];
            ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, then_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[2];
            ASSERT_EQUAL(edge->source_block_id.index, then_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[3];
            ASSERT_EQUAL(edge->source_block_id.index, else_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
        }

        // NOTE(vlad): Test that entry block was determined correctly.
        {
            const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
            ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_while_loops(Test_Context* test_context)
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(cfg->blocks_count, 4);
        ASSERT_EQUAL(cfg->edges_count, 3);

        const Index condition_block_index = 1;
        const Index body_block_index = 2;
        const Index final_block_index = 3;

        {
            const Cfg_Block* block = &cfg->blocks[condition_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);
        }

        {
            const Cfg_Block* block = &cfg->blocks[body_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);
        }

        {
            const Cfg_Block* block = &cfg->blocks[final_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[0];
            ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[1];
            ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, body_block_index);
        }

        {
            const Cfg_Edge* edge = &cfg->edges[2];
            ASSERT_EQUAL(edge->source_block_id.index, body_block_index);
            ASSERT_EQUAL(edge->destination_block_id.index, condition_block_index);
        }

        // NOTE(vlad): Test that entry block was determined correctly.
        {
            const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
            ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_complex_statements(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    while 1 != 2"
                                                 "    {"
                                                 "        a := 10;"
                                                 "        if a == 10"
                                                 "        {"
                                                 "            return;"
                                                 "        }"
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(cfg->blocks_count, 8);
        ASSERT_EQUAL(cfg->edges_count, 7);

        const Index condition_block_index = 1;
        const Index before_if_block_index = 2;
        const Index then_block_index = 3;
        const Index after_return_block_index = 4;
        const Index else_block_index = 5;
        const Index jump_after_if_block_index = 6;
        const Index final_block_index = 7;

        // NOTE(vlad): Testing blocks.
        {
            {
                const Cfg_Block* block = &cfg->blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);
            }

            {
                const Cfg_Block* block = &cfg->blocks[before_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);
            }

            {
                const Cfg_Block* block = &cfg->blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }

            {
                const Cfg_Block* block = &cfg->blocks[after_return_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);
            }

            {
                const Cfg_Block* block = &cfg->blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 1);
            }

            {
                const Cfg_Block* block = &cfg->blocks[jump_after_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);
            }

            {
                const Cfg_Block* block = &cfg->blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }
        }

        // NOTE(vlad): Testing edges.
        {
            const Cfg_Edge* edge = &cfg->edges[0];

            {
                ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
            }

            edge += 1;

            {
                ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, before_if_block_index);
            }

            edge += 1;

            {
                ASSERT_EQUAL(edge->source_block_id.index, before_if_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, else_block_index);
            }

            edge += 1;

            {
                ASSERT_EQUAL(edge->source_block_id.index, before_if_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, then_block_index);
            }

            edge += 1;

            {
                ASSERT_EQUAL(edge->source_block_id.index, after_return_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, jump_after_if_block_index);
            }

            edge += 1;

            {
                ASSERT_EQUAL(edge->source_block_id.index, else_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, jump_after_if_block_index);
            }

            edge += 1;

            {
                ASSERT_EQUAL(edge->source_block_id.index, jump_after_if_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, condition_block_index);
            }

            ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(edge + 1, cfg->edges), cfg->edges_count);
        }

        // NOTE(vlad): Test that entry block was determined correctly.
        {
            const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
            ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_unreachable_blocks_removal(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    return;"
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_EQUAL(tac_function->instructions_count, 3);

        // NOTE(vlad): Before dead code elimination.
        {
            ASSERT_EQUAL(cfg->blocks_count, 3);
            ASSERT_EQUAL(cfg->edges_count, 0);

            const Cfg_Block* block = &cfg->blocks[INVALID_CFG_BLOCK_INDEX + 1];
            {
                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 1);

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }

            block += 1;

            {
                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 2);

                {
                    const Index instruction_index = block->instructions_range.start_instruction_index;
                    const Tac_Instruction* instruction = &tac_function->instructions[instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_ASSIGN);
                }

                {
                    const Index instruction_index = block->instructions_range.start_instruction_index + 1;
                    const Tac_Instruction* instruction = &tac_function->instructions[instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(instruction->operation, TAC_RETURN);
                }
            }

            ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(block + 1, cfg->blocks), cfg->blocks_count);

            // NOTE(vlad): Test that entry block was determined correctly.
            {
                const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
                ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
            }
        }

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        // NOTE(vlad): After dead code elimination.
        {
            ASSERT_EQUAL(cfg->blocks_count, 2);
            ASSERT_EQUAL(cfg->edges_count, 0);

            const Cfg_Block* block = &cfg->blocks[INVALID_CFG_BLOCK_INDEX + 1];
            {
                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 1);

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }

            ASSERT_EQUAL(DISTANCE_BETWEEN_POINTERS(block + 1, cfg->blocks), cfg->blocks_count);

            // NOTE(vlad): Test that entry block was determined correctly.
            {
                const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
                ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    if 1 != 2"
                                                 "    {"
                                                 "        return;"
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

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;
        const Cfg* cfg = &context.cfg;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        // NOTE(vlad): Before dead code elimination.
        {
            ASSERT_EQUAL(cfg->blocks_count, 6);
            ASSERT_EQUAL(cfg->edges_count, 4);

            const Index condition_block_index = 1;
            const Index then_block_index = 2;
            const Index then_unreachable_block_index = 3;
            const Index else_block_index = 4;
            const Index final_block_index = 5;

            {
                const Cfg_Block* block = &cfg->blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);
            }

            {
                const Cfg_Block* block = &cfg->blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }

            {
                const Cfg_Block* block = &cfg->blocks[then_unreachable_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);
            }

            {
                const Cfg_Block* block = &cfg->blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 1);
            }

            {
                const Cfg_Block* block = &cfg->blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }

            {
                const Cfg_Edge* edge = &cfg->edges[0];
                ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, else_block_index);
            }

            {
                const Cfg_Edge* edge = &cfg->edges[1];
                ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, then_block_index);
            }

            {
                const Cfg_Edge* edge = &cfg->edges[2];
                ASSERT_EQUAL(edge->source_block_id.index, then_unreachable_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
            }

            {
                const Cfg_Edge* edge = &cfg->edges[3];
                ASSERT_EQUAL(edge->source_block_id.index, else_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
            }

            // NOTE(vlad): Test that entry block was determined correctly.
            {
                const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
                ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
            }
        }

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        // NOTE(vlad): After dead code elimination.
        {
            ASSERT_EQUAL(cfg->blocks_count, 5);
            ASSERT_EQUAL(cfg->edges_count, 3);

            const Index condition_block_index = 1;
            const Index then_block_index = 2;
            const Index else_block_index = 3;
            const Index final_block_index = 4;

            {
                const Cfg_Block* block = &cfg->blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);
            }

            {
                const Cfg_Block* block = &cfg->blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }

            {
                const Cfg_Block* block = &cfg->blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 1);
            }

            {
                const Cfg_Block* block = &cfg->blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);
            }

            {
                const Cfg_Edge* edge = &cfg->edges[0];
                ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, else_block_index);
            }

            {
                const Cfg_Edge* edge = &cfg->edges[1];
                ASSERT_EQUAL(edge->source_block_id.index, condition_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, then_block_index);
            }

            {
                const Cfg_Edge* edge = &cfg->edges[2];
                ASSERT_EQUAL(edge->source_block_id.index, else_block_index);
                ASSERT_EQUAL(edge->destination_block_id.index, final_block_index);
            }

            // NOTE(vlad): Test that entry block was determined correctly.
            {
                const Tac_Function_Label* function_label = get_tac_function_label_by_id(&context.tac, tac_function->label_id);
                ASSERT_EQUAL(function_label->entry_cfg_block_id.index, 1);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_functions_without_jumps,
    test_if_statements,
    test_while_loops,
    test_complex_statements,
    test_unreachable_blocks_removal
)

#include "eon_cfg.c"
#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_tac.c"
#include "eon_types.c"
