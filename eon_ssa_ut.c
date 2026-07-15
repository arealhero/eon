#include "eon_unit_test.h"

#include "eon_ssa.h"

#include "eon_lexical_scopes.h"
#include "eon_parser.h"
#include "eon_types.h"

// NOTE(vlad): Including SSA implementation to be able to test every step of the construction process.
#include "eon_ssa.c"

internal void
test_dominators_and_dominance_frontiers_computing(Test_Context* test_context)
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

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_EQUAL(tac_function->instructions_count, 1);

        ASSERT_EQUAL(tac_function->cfg_blocks_count, 1);

        const Cfg_Block* block = &tac_function->cfg_blocks[0];
        {
            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            ASSERT_EQUAL(block->instructions_range.end_instruction_index - block->instructions_range.start_instruction_index,
                         tac_function->instructions_count);

            ASSERT_EQUAL(block->edges_count, 0);
            ASSERT_EQUAL(block->predecessors_count, 0);

            ASSERT_EQUAL(block->postorder_index, 0);
            ASSERT_EQUAL(block->immediate_dominator_id.index, 0);

            ASSERT_EQUAL(block->dominance_frontier_count, 0);
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

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(tac_function->cfg_blocks_count, 4);

        const Index condition_block_index = 0;
        const Index then_block_index = 1;
        const Index else_block_index = 2;
        const Index final_block_index = 3;

        {
            const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

            ASSERT_EQUAL(block->edges_count, 2);
            ASSERT_EQUAL(block->edges[0].index, else_block_index);
            ASSERT_EQUAL(block->edges[1].index, then_block_index);

            ASSERT_EQUAL(block->predecessors_count, 0);

            ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

            ASSERT_EQUAL(block->dominance_frontier_count, 0);
        }

        {
            const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

            ASSERT_EQUAL(block->edges_count, 1);
            ASSERT_EQUAL(block->edges[0].index, final_block_index);

            ASSERT_EQUAL(block->predecessors_count, 1);
            ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

            ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

            ASSERT_EQUAL(block->dominance_frontier_count, 1);
            ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);
        }

        {
            const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);

            ASSERT_EQUAL(block->edges_count, 1);
            ASSERT_EQUAL(block->edges[0].index, final_block_index);

            ASSERT_EQUAL(block->predecessors_count, 1);
            ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

            ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

            ASSERT_EQUAL(block->dominance_frontier_count, 1);
            ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);
        }

        {
            const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

            ASSERT_EQUAL(block->edges_count, 0);
            ASSERT_EQUAL(block->predecessors_count, 2);
            ASSERT_EQUAL(block->predecessors[0].index, then_block_index);
            ASSERT_EQUAL(block->predecessors[1].index, else_block_index);

            ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

            ASSERT_EQUAL(block->dominance_frontier_count, 0);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

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

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(tac_function->cfg_blocks_count, 3);

        const Index condition_block_index = 0;
        const Index body_block_index = 1;
        const Index final_block_index = 2;

        {
            const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

            ASSERT_EQUAL(block->edges_count, 2);
            ASSERT_EQUAL(block->edges[0].index, final_block_index);
            ASSERT_EQUAL(block->edges[1].index, body_block_index);

            ASSERT_EQUAL(block->predecessors_count, 1);
            ASSERT_EQUAL(block->predecessors[0].index, body_block_index);

            ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

            ASSERT_EQUAL(block->dominance_frontier_count, 0);
        }

        {
            const Cfg_Block* block = &tac_function->cfg_blocks[body_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

            ASSERT_EQUAL(block->edges_count, 1);
            ASSERT_EQUAL(block->edges[0].index, condition_block_index);

            ASSERT_EQUAL(block->predecessors_count, 1);
            ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

            ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

            ASSERT_EQUAL(block->dominance_frontier_count, 0);
        }

        {
            const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
            const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
            ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

            ASSERT_EQUAL(block->edges_count, 0);
            ASSERT_EQUAL(block->predecessors_count, 1);
            ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

            ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

            ASSERT_EQUAL(block->dominance_frontier_count, 0);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

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

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(tac_function->cfg_blocks_count, 6);

        const Index condition_block_index = 0;
        const Index before_if_block_index = 1;
        const Index then_block_index = 2;
        const Index else_block_index = 3;
        const Index jump_after_if_block_index = 4;
        const Index final_block_index = 5;

        // NOTE(vlad): Testing blocks.
        {
            {
                const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, before_if_block_index);
                ASSERT_EQUAL(block->edges[1].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, jump_after_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[before_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, then_block_index);
                ASSERT_EQUAL(block->edges[1].index, else_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 1);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, jump_after_if_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[jump_after_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, condition_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, else_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    while 1 != 2"
                                                 "    {"
                                                 "        a := 10;"
                                                 "        if a == 10"
                                                 "        {"
                                                 "            b := 20;"
                                                 "        }"
                                                 "    }"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(tac_function->cfg_blocks_count, 6);

        const Index condition_block_index = 0;
        const Index before_if_block_index = 1;
        const Index then_block_index = 2;
        const Index else_block_index = 3;
        const Index jump_after_if_block_index = 4;
        const Index final_block_index = 5;

        // NOTE(vlad): Testing blocks.
        {
            {
                const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);
                ASSERT_EQUAL(block->edges[1].index, before_if_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, jump_after_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[before_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, else_block_index);
                ASSERT_EQUAL(block->edges[1].index, then_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, jump_after_if_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, jump_after_if_block_index);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

                const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                    - block->instructions_range.start_instruction_index;
                ASSERT_EQUAL(this_block_instructions_count, 1);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, jump_after_if_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, jump_after_if_block_index);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[jump_after_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, condition_block_index);

                ASSERT_EQUAL(block->predecessors_count, 2);
                ASSERT_EQUAL(block->predecessors[0].index, then_block_index);
                ASSERT_EQUAL(block->predecessors[1].index, else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

#define ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(node, identifier_name) \
    do                                                                  \
    {                                                                   \
        const Tac_Variable_Id variable_id = (node)->destination;        \
        const Tac_Variable* variable = get_tac_variable_by_id(&context.tac, variable_id); \
        const Symbol* symbol = get_symbol_by_id(&context, variable->symbol_id); \
        ASSERT_STRINGS_ARE_EQUAL(symbol->name, identifier_name);        \
    }                                                                   \
    while (0)

internal void
test_phi_nodes_insertion(Test_Context* test_context)
{
    // NOTE(vlad): Testing function without arguments and local variables.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];
        ASSERT_EQUAL(tac_function->instructions_count, 1);

        ASSERT_EQUAL(tac_function->cfg_blocks_count, 1);

        const Cfg_Block* block = &tac_function->cfg_blocks[0];
        {
            const Tac_Instructions_Range* range = &block->instructions_range;
            ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
            ASSERT_FALSE(cfg_block_is_empty(block));

            ASSERT_EQUAL(block->instructions_range.end_instruction_index - block->instructions_range.start_instruction_index,
                         tac_function->instructions_count);

            ASSERT_EQUAL(block->edges_count, 0);
            ASSERT_EQUAL(block->predecessors_count, 0);

            ASSERT_EQUAL(block->postorder_index, 0);
            ASSERT_EQUAL(block->immediate_dominator_id.index, 0);

            ASSERT_EQUAL(block->dominance_frontier_count, 0);

            ASSERT_EQUAL(block->phi_nodes_count, 0);
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing variables created in a child lexical scope.
    {
        // NOTE(vlad): Testing if statement.
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

            validate_ast(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            lower_ast_to_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            construct_cfg_from_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            remove_unreachable_cfg_blocks(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominators(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominance_frontiers(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            insert_phi_nodes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            const Tac* tac = &context.tac;

            ASSERT_EQUAL(tac->functions_count, 1);
            const Tac_Function* tac_function = &tac->functions[0];

            ASSERT_EQUAL(tac_function->cfg_blocks_count, 4);

            const Index condition_block_index = 0;
            const Index then_block_index = 1;
            const Index else_block_index = 2;
            const Index final_block_index = 3;

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, else_block_index);
                ASSERT_EQUAL(block->edges[1].index, then_block_index);

                ASSERT_EQUAL(block->predecessors_count, 0);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 2);
                ASSERT_EQUAL(block->predecessors[0].index, then_block_index);
                ASSERT_EQUAL(block->predecessors[1].index, else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 2);
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[0], "a");
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[1], "b");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        // NOTE(vlad): Testing while loop.
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

            validate_ast(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            lower_ast_to_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            construct_cfg_from_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            remove_unreachable_cfg_blocks(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominators(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominance_frontiers(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            insert_phi_nodes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            const Tac* tac = &context.tac;

            ASSERT_EQUAL(tac->functions_count, 1);
            const Tac_Function* tac_function = &tac->functions[0];

            ASSERT_EQUAL(tac_function->cfg_blocks_count, 3);

            const Index condition_block_index = 0;
            const Index body_block_index = 1;
            const Index final_block_index = 2;

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);
                ASSERT_EQUAL(block->edges[1].index, body_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, body_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[body_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, condition_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }

    // NOTE(vlad): Testing phi-nodes after if statement.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                     "    a: mutable _ = 10;"
                                                     "    b: mutable _ = 20;"
                                                     "    if 1 != 2"
                                                     "    {"
                                                     "        a = 11;"
                                                     "        b = 21;"
                                                     "    }"
                                                     "    else"
                                                     "    {"
                                                     "        a = 12;"
                                                     "        b = 22;"
                                                     "    }"
                                                     "    c := a;"
                                                     "    d := b;"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            validate_ast(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            lower_ast_to_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            construct_cfg_from_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            remove_unreachable_cfg_blocks(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominators(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominance_frontiers(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            insert_phi_nodes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            const Tac* tac = &context.tac;

            ASSERT_EQUAL(tac->functions_count, 1);
            const Tac_Function* tac_function = &tac->functions[0];

            ASSERT_EQUAL(tac_function->cfg_blocks_count, 4);

            const Index condition_block_index = 0;
            const Index then_block_index = 1;
            const Index else_block_index = 2;
            const Index final_block_index = 3;

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, else_block_index);
                ASSERT_EQUAL(block->edges[1].index, then_block_index);

                ASSERT_EQUAL(block->predecessors_count, 0);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 2);
                ASSERT_EQUAL(block->predecessors[0].index, then_block_index);
                ASSERT_EQUAL(block->predecessors[1].index, else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 2);
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[0], "a");
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[1], "b");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: mutable s32) -> s32 = {"
                                                     "    if 1 != 2"
                                                     "    {"
                                                     "        parameter = 10;"
                                                     "    }"
                                                     "    else"
                                                     "    {"
                                                     "        parameter = 20;"
                                                     "    }"
                                                     "    return parameter;"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            validate_ast(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            lower_ast_to_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            construct_cfg_from_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            remove_unreachable_cfg_blocks(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominators(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominance_frontiers(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            insert_phi_nodes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            const Tac* tac = &context.tac;

            ASSERT_EQUAL(tac->functions_count, 1);
            const Tac_Function* tac_function = &tac->functions[0];

            ASSERT_EQUAL(tac_function->cfg_blocks_count, 4);

            const Index condition_block_index = 0;
            const Index then_block_index = 1;
            const Index else_block_index = 2;
            const Index final_block_index = 3;

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, else_block_index);
                ASSERT_EQUAL(block->edges[1].index, then_block_index);

                ASSERT_EQUAL(block->predecessors_count, 0);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 2);
                ASSERT_EQUAL(block->predecessors[0].index, then_block_index);
                ASSERT_EQUAL(block->predecessors[1].index, else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 1);
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[0], "parameter");
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }

    // NOTE(vlad): Testing when phi-node is created in a block
    //             and variable is used in a successor block.
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                 "    a: mutable _ = 10;"
                                                 "    if 1 != 2"
                                                 "    {"
                                                 "        a = 11;"
                                                 "    }"
                                                 "    else"
                                                 "    {"
                                                 "        a = 12;"
                                                 "    }"
                                                 ""
                                                 "    b: mutable _ = 20;"
                                                 "    if 1 != 2"
                                                 "    {"
                                                 "        b = a;"
                                                 "    }"
                                                 "    else"
                                                 "    {"
                                                 "        b = 21;"
                                                 "    }"
                                                 ""
                                                 "    c := b;"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        const Tac* tac = &context.tac;

        ASSERT_EQUAL(tac->functions_count, 1);
        const Tac_Function* tac_function = &tac->functions[0];

        ASSERT_EQUAL(tac_function->cfg_blocks_count, 7);

        const Index before_first_if_block_index = 0;
        const Index first_then_block_index = 1;
        const Index first_else_block_index = 2;
        const Index before_second_if_block_index = 3;
        const Index second_then_block_index = 4;
        const Index second_else_block_index = 5;
        const Index final_block_index = 6;

        // NOTE(vlad): Testing blocks.
        {
            {
                const Cfg_Block* block = &tac_function->cfg_blocks[before_first_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, first_else_block_index);
                ASSERT_EQUAL(block->edges[1].index, first_then_block_index);

                ASSERT_EQUAL(block->predecessors_count, 0);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_first_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[first_then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, before_second_if_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_first_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_first_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, before_second_if_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[first_else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, before_second_if_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_first_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_first_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, before_second_if_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[before_second_if_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, second_else_block_index);
                ASSERT_EQUAL(block->edges[1].index, second_then_block_index);

                ASSERT_EQUAL(block->predecessors_count, 2);
                ASSERT_EQUAL(block->predecessors[0].index, first_then_block_index);
                ASSERT_EQUAL(block->predecessors[1].index, first_else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_first_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 1);
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[0], "a");
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[second_then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_second_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_second_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[second_else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, before_second_if_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_second_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 2);
                ASSERT_EQUAL(block->predecessors[0].index, second_then_block_index);
                ASSERT_EQUAL(block->predecessors[1].index, second_else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, before_second_if_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 1);
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[0], "b");
            }
        }

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing that phi-nodes are not created if variable is just used
    //             and not declared/assigned to.
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

            validate_ast(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            lower_ast_to_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            construct_cfg_from_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            remove_unreachable_cfg_blocks(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominators(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominance_frontiers(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            insert_phi_nodes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            const Tac* tac = &context.tac;

            ASSERT_EQUAL(tac->functions_count, 1);
            const Tac_Function* tac_function = &tac->functions[0];

            ASSERT_EQUAL(tac_function->cfg_blocks_count, 6);

            const Index condition_block_index = 0;
            const Index before_if_block_index = 1;
            const Index then_block_index = 2;
            const Index else_block_index = 3;
            const Index jump_after_if_block_index = 4;
            const Index final_block_index = 5;

            // NOTE(vlad): Testing blocks.
            {
                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                    ASSERT_EQUAL(block->edges_count, 2);
                    ASSERT_EQUAL(block->edges[0].index, before_if_block_index);
                    ASSERT_EQUAL(block->edges[1].index, final_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, jump_after_if_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[before_if_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                    ASSERT_EQUAL(block->edges_count, 2);
                    ASSERT_EQUAL(block->edges[0].index, then_block_index);
                    ASSERT_EQUAL(block->edges[1].index, else_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                    ASSERT_EQUAL(block->edges_count, 0);
                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

                    const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                        - block->instructions_range.start_instruction_index;
                    ASSERT_EQUAL(this_block_instructions_count, 1);

                    ASSERT_EQUAL(block->edges_count, 1);
                    ASSERT_EQUAL(block->edges[0].index, jump_after_if_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[jump_after_if_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                    ASSERT_EQUAL(block->edges_count, 1);
                    ASSERT_EQUAL(block->edges[0].index, condition_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, else_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, else_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                    ASSERT_EQUAL(block->edges_count, 0);
                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {"
                                                     "    while 1 != 2"
                                                     "    {"
                                                     "        a := 10;"
                                                     "        if a == 10"
                                                     "        {"
                                                     "            b := 20;"
                                                     "        }"
                                                     "    }"
                                                     "}");

            Lexer lexer = {0};
            Parser parser = {0};

            create_lexer(&lexer, &context);
            create_parser(&parser, &lexer, &context);

            ASSERT_TRUE(parse_ast(&parser));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            validate_ast(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            lower_ast_to_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            construct_cfg_from_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            remove_unreachable_cfg_blocks(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominators(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominance_frontiers(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            insert_phi_nodes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            const Tac* tac = &context.tac;

            ASSERT_EQUAL(tac->functions_count, 1);
            const Tac_Function* tac_function = &tac->functions[0];

            ASSERT_EQUAL(tac_function->cfg_blocks_count, 6);

            const Index condition_block_index = 0;
            const Index before_if_block_index = 1;
            const Index then_block_index = 2;
            const Index else_block_index = 3;
            const Index jump_after_if_block_index = 4;
            const Index final_block_index = 5;

            // NOTE(vlad): Testing blocks.
            {
                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                    ASSERT_EQUAL(block->edges_count, 2);
                    ASSERT_EQUAL(block->edges[0].index, final_block_index);
                    ASSERT_EQUAL(block->edges[1].index, before_if_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, jump_after_if_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[before_if_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                    ASSERT_EQUAL(block->edges_count, 2);
                    ASSERT_EQUAL(block->edges[0].index, else_block_index);
                    ASSERT_EQUAL(block->edges[1].index, then_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                    ASSERT_EQUAL(block->edges_count, 1);
                    ASSERT_EQUAL(block->edges[0].index, jump_after_if_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 1);
                    ASSERT_EQUAL(block->dominance_frontier[0].index, jump_after_if_block_index);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_LABEL);

                    const Size this_block_instructions_count = block->instructions_range.end_instruction_index
                        - block->instructions_range.start_instruction_index;
                    ASSERT_EQUAL(this_block_instructions_count, 1);

                    ASSERT_EQUAL(block->edges_count, 1);
                    ASSERT_EQUAL(block->edges[0].index, jump_after_if_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, before_if_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 1);
                    ASSERT_EQUAL(block->dominance_frontier[0].index, jump_after_if_block_index);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[jump_after_if_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                    ASSERT_EQUAL(block->edges_count, 1);
                    ASSERT_EQUAL(block->edges[0].index, condition_block_index);

                    ASSERT_EQUAL(block->predecessors_count, 2);
                    ASSERT_EQUAL(block->predecessors[0].index, then_block_index);
                    ASSERT_EQUAL(block->predecessors[1].index, else_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, before_if_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 1);
                    ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[0], "b");
                }

                {
                    const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                    const Tac_Instructions_Range* range = &block->instructions_range;
                    ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                    ASSERT_FALSE(cfg_block_is_empty(block));

                    const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                    const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                    ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                    ASSERT_EQUAL(block->edges_count, 0);
                    ASSERT_EQUAL(block->predecessors_count, 1);
                    ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                    ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                    ASSERT_EQUAL(block->dominance_frontier_count, 0);

                    ASSERT_EQUAL(block->phi_nodes_count, 0);
                }
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }
}

internal void
test_ssa_versions_of_variables(Test_Context* test_context)
{
    // NOTE(vlad): Testing variables created in a child lexical scope.
    {
        // NOTE(vlad): Testing if statement.
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

            validate_ast(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            create_lexical_scopes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            resolve_and_validate_types(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            lower_ast_to_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            construct_cfg_from_tac(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            remove_unreachable_cfg_blocks(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominators(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            compute_cfg_dominance_frontiers(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            insert_phi_nodes(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            set_tac_variable_versions(&context);
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

            const Tac* tac = &context.tac;

            ASSERT_EQUAL(tac->functions_count, 1);
            const Tac_Function* tac_function = &tac->functions[0];

            ASSERT_EQUAL(tac_function->cfg_blocks_count, 4);

            const Index condition_block_index = 0;
            const Index then_block_index = 1;
            const Index else_block_index = 2;
            const Index final_block_index = 3;

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[condition_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP_IF_FALSE);

                ASSERT_EQUAL(block->edges_count, 2);
                ASSERT_EQUAL(block->edges[0].index, else_block_index);
                ASSERT_EQUAL(block->edges[1].index, then_block_index);

                ASSERT_EQUAL(block->predecessors_count, 0);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[then_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_JUMP);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[else_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_ASSIGN);

                ASSERT_EQUAL(block->edges_count, 1);
                ASSERT_EQUAL(block->edges[0].index, final_block_index);

                ASSERT_EQUAL(block->predecessors_count, 1);
                ASSERT_EQUAL(block->predecessors[0].index, condition_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 1);
                ASSERT_EQUAL(block->dominance_frontier[0].index, final_block_index);

                ASSERT_EQUAL(block->phi_nodes_count, 0);
            }

            {
                const Cfg_Block* block = &tac_function->cfg_blocks[final_block_index];

                const Tac_Instructions_Range* range = &block->instructions_range;
                ASSERT_EQUAL(range->function_label_id.index, tac_function->label_id.index);
                ASSERT_FALSE(cfg_block_is_empty(block));

                const Index last_instruction_index = block->instructions_range.end_instruction_index - 1;
                const Tac_Instruction* last_instruction = &tac_function->instructions[last_instruction_index];
                ASSERT_ENUM_VALUES_ARE_EQUAL(last_instruction->operation, TAC_RETURN);

                ASSERT_EQUAL(block->edges_count, 0);
                ASSERT_EQUAL(block->predecessors_count, 2);
                ASSERT_EQUAL(block->predecessors[0].index, then_block_index);
                ASSERT_EQUAL(block->predecessors[1].index, else_block_index);

                ASSERT_EQUAL(block->immediate_dominator_id.index, condition_block_index);

                ASSERT_EQUAL(block->dominance_frontier_count, 0);

                ASSERT_EQUAL(block->phi_nodes_count, 2);
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[0], "a");
                ASSERT_PHI_NODE_WAS_CREATED_FOR_IDENTIFIER(&block->phi_nodes[1], "b");

                ASSERT_NOT_EQUAL(block->phi_nodes[0].destination.ssa_version, 0);
                ASSERT_NOT_EQUAL(block->phi_nodes[1].destination.ssa_version, 0);
            }

            destroy_parser(&parser);
            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }
}

internal void
test_unused_ssa_assignments(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> void = {\n"
                                                 "    a := 10;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:5: error: This variable was never used\n"
                                                        "  2 |     a := 10;\n"
                                                        "    |     ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 "    a = 20;\n"
                                                 "    return a;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        // FIXME(vlad): Highlight assignment here.
        const String_View expected_output = string_view("<test-input>:2:5: error: This assignment is unused\n"
                                                        "  2 |     a: mutable _ = 10;\n"
                                                        "    |     ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 ""
                                                 "    if 1 != 2\n"
                                                 "    {\n"
                                                 "        a = 20;\n"
                                                 "        return a;\n"
                                                 "    }\n"
                                                 ""
                                                 "    return 30;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        // FIXME(vlad): Highlight assignment here.
        const String_View expected_output = string_view("<test-input>:2:5: error: This assignment is unused\n"
                                                        "  2 |     a: mutable _ = 10;\n"
                                                        "    |     ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 ""
                                                 "    if 1 != 2\n"
                                                 "    {\n"
                                                 "        a = 20;\n"
                                                 "    }\n"
                                                 ""
                                                 "    return 30;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        // FIXME(vlad): Highlight assignment here.
        const String_View expected_output = string_view("<test-input>:2:5: error: This variable was never used\n"
                                                        "  2 |     a: mutable _ = 10;\n"
                                                        "    |     ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: (parameter: s32) -> void = {}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:1:7: error: This variable was never used\n"
                                                        "  1 | foo: (parameter: s32) -> void = {}\n"
                                                        "    |       ^~~~~~~~~");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 ""
                                                 "    if 1 != 2\n"
                                                 "    {\n"
                                                 "        a = 21;\n"
                                                 "        return a;\n"
                                                 "    }\n"
                                                 "    else\n"
                                                 "    {\n"
                                                 "        a = 22;\n"
                                                 "    }\n"
                                                 ""
                                                 "    return 30;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:5: error: This assignment is unused\n"
                                                        "  2 |     a: mutable _ = 10;\n"
                                                        "    |     ^\n"
                                                        "<test-input>:10:9: error: This assignment is unused\n"
                                                        "  10 |         a = 22;\n"
                                                        "     |         ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 ""
                                                 "    if 1 != 2\n"
                                                 "    {\n"
                                                 "        if 1 != 2\n"
                                                 "        {\n"
                                                 "            a = 21;\n"
                                                 "            return a;\n"
                                                 "        }\n"
                                                 "        else\n"
                                                 "        {\n"
                                                 "            a = 22;\n"
                                                 "        }\n"
                                                 "    }\n"
                                                 ""
                                                 "    return 30;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:12:13: error: This assignment is unused\n"
                                                        "  12 |             a = 22;\n"
                                                        "     |             ^\n"
                                                        "<test-input>:2:5: error: This assignment is unused\n"
                                                        "  2 |     a: mutable _ = 10;\n"
                                                        "    |     ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("foo: () -> s32 = {\n"
                                                 "    a: mutable _ = 10;\n"
                                                 "    b: mutable _ = 11;\n"
                                                 ""
                                                 "    if 1 != 2\n"
                                                 "    {\n"
                                                 "        a = 20;\n"
                                                 "        b = 21;\n"
                                                 "    }\n"
                                                 "    else\n"
                                                 "    {\n"
                                                 "        a = 30;\n"
                                                 "    }\n"
                                                 ""
                                                 "    return a + b;\n"
                                                 "}");

        Lexer lexer = {0};
        Parser parser = {0};

        create_lexer(&lexer, &context);
        create_parser(&parser, &lexer, &context);

        ASSERT_TRUE(parse_ast(&parser));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        validate_ast(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        create_lexical_scopes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        resolve_and_validate_types(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        lower_ast_to_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        construct_cfg_from_tac(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        remove_unreachable_cfg_blocks(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominators(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        compute_cfg_dominance_frontiers(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        insert_phi_nodes(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        set_tac_variable_versions(&context);
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();

        find_unused_ssa_assignments(&context);
        ASSERT_TRUE(has_diagnostic_messages(&context));

        const String_View dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                                     &context,
                                                                     MAX_MESSAGE_LEVEL);
        const String_View expected_output = string_view("<test-input>:2:5: error: This assignment is unused\n"
                                                        "  2 |     a: mutable _ = 10;\n"
                                                        "    |     ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_parser(&parser);
        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_dominators_and_dominance_frontiers_computing,
    test_phi_nodes_insertion,
    test_ssa_versions_of_variables,
    test_unused_ssa_assignments
)

#include "eon_ast.c"
#include "eon_cfg.c"
#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_lexical_scopes.c"
#include "eon_parser.c"
#include "eon_tac.c"
#include "eon_types.c"
