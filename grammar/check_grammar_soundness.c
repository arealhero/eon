#include <eon/common.h>
#include <eon/io.h>
#include <eon/string.h>

#include <eon/platform/filesystem.h>
#include <eon/platform/time.h>

#include "grammar_lexer.h"
#include "grammar_log.h"
#include "grammar_parser.h"

internal Bool check_grammar_soundness(const String_View grammar_filename, const String_View grammar);

int
main(int argc, const char* argv[])
{
    init_io_state(MiB(40));

    if (argc != 2)
    {
        println("Usage: {} <grammar-file>", argv[0]);
        return 1;
    }

    // TODO(vlad): Allocate more memory, otherwise we wouldn't be able to read files with size > 1 GiB.
    Arena* arena = arena_create(GiB(1), MiB(1));

    const String_View grammar_filename = string_view(argv[1]);
    Read_File_Result read_result = platform_read_entire_text_file(arena, grammar_filename);

    if (read_result.status == READ_FILE_FAILURE)
    {
        println("Failed to read file '{}'", grammar_filename);
        return 1;
    }

    const String_View grammar = string_view(read_result.content);
    const Bool result = check_grammar_soundness(grammar_filename, grammar);

    arena_destroy(arena);

    return !result;
}

struct Identifiers_Array
{
    const Token** tokens;
    Size tokens_count;
    Size tokens_capacity;
};
typedef struct Identifiers_Array Identifiers_Array;

internal void
add_identifier(Arena* arena,
               Identifiers_Array* array,
               const Token* identifier_token)
{
    if (array->tokens_count == array->tokens_capacity)
    {
        const Size new_capacity = MAX(1, 2 * array->tokens_capacity);
        array->tokens = reallocate(arena,
                                   array->tokens,
                                   const Token*,
                                   array->tokens_capacity,
                                   new_capacity);
        array->tokens_capacity = new_capacity;
    }

    array->tokens[array->tokens_count] = identifier_token;
    array->tokens_count += 1;
}

internal Bool
has_identifier(Identifiers_Array* array,
               const Token* identifier_token)
{
    for (Index i = 0;
         i < array->tokens_count;
         ++i)
    {
        if (strings_are_equal(array->tokens[i]->lexeme, identifier_token->lexeme))
        {
            return true;
        }
    }

    return false;
}

struct Grammar_Info
{
    Identifiers_Array defined_identifiers;
    Identifiers_Array undefined_identifiers;

    // TODO(vlad): Support this.
    // String_View* found_terminals;
};
typedef struct Grammar_Info Grammar_Info;

struct Definition_Node
{
    const Ast_Identifier_Definition* definition;

    // NOTE(vlad): Reachable through left recursion.
    Index* reachable_definitions;
    Size reachable_definitions_count;
};
typedef struct Definition_Node Definition_Node;

// FIXME(vlad): Clean up this mess of a code. Also make it fast. @tag(performance)
// FIXME(vlad): Centralise error reporting.
internal Bool
detect_left_recursions(Arena* scratch,
                       const String_View grammar_filename,
                       const String_View grammar,
                       const Ast* ast)
{
    Bool found_errors = false;

    const Size nodes_count = ast->definitions_count;
    Definition_Node* nodes = allocate_array(scratch,
                                            nodes_count,
                                            Definition_Node);

    // NOTE(vlad): Initialise nodes so their identifiers are available for searching.
    for (Index definition_index = 0;
         definition_index < ast->definitions_count;
         ++definition_index)
    {
        Definition_Node* this_node = &nodes[definition_index];
        this_node->definition = &ast->definitions[definition_index];
    }

    // NOTE(vlad): Initialise graph edges.
    //             Also test that every definition has at least 1 possible expression
    //             and every expression has at least one identifier to expand to (terminal or non-terminal).
    for (Index node_index = 0;
         node_index < nodes_count;
         ++node_index)
    {
        Definition_Node* this_node = &nodes[node_index];
        const Ast_Identifier_Definition* definition = this_node->definition;

        this_node->reachable_definitions = allocate_array(scratch,
                                                          definition->possible_expressions_count,
                                                          Index);

        if (definition->possible_expressions_count == 0)
        {
            found_errors = true;

            const Token* token = &definition->identifier.token;

            println("{}:{}:{}: Error: definition for '{}' has no possible expressions",
                    grammar_filename, token->line+1, token->column+1,
                    token->lexeme);
            show_grammar_error(scratch,
                               grammar,
                               token->line,
                               token->column,
                               token->lexeme.length);
        }

        for (Index expression_index = 0;
             expression_index < definition->possible_expressions_count;
             ++expression_index)
        {
            const Ast_Expression* expression = &definition->possible_expressions[expression_index];
            if (expression->identifiers_count == 0)
            {
                found_errors = true;

                const Token* token = &definition->identifier.token;

                println("{}:{}:{}: Error: empty expression detected in '{}' definition",
                        grammar_filename, token->line+1, token->column+1,
                        token->lexeme);
                show_grammar_error(scratch,
                                   grammar,
                                   token->line,
                                   token->column,
                                   token->lexeme.length);

                continue;
            }

            const Ast_Identifier* first_identifier = &expression->identifiers[0];
            if (first_identifier->token.type != TOKEN_NON_TERMINAL)
            {
                continue;
            }

            for (Index reachable_node_index = 0;
                 reachable_node_index < nodes_count;
                 ++reachable_node_index)
            {
                const Definition_Node* node = &nodes[reachable_node_index];

                if (strings_are_equal(node->definition->identifier.token.lexeme, first_identifier->token.lexeme))
                {
                    Bool alreadyAdded = false;
                    for (Index i = 0;
                         i < this_node->reachable_definitions_count;
                         ++i)
                    {
                        if (this_node->reachable_definitions[i] == reachable_node_index)
                        {
                            alreadyAdded = true;
                            break;
                        }
                    }

                    if (!alreadyAdded)
                    {
                        this_node->reachable_definitions[this_node->reachable_definitions_count] = reachable_node_index;
                        this_node->reachable_definitions_count += 1;
                    }

                    break;
                }
            }
        }
    }

    // NOTE(vlad): Traverse this graph via DFS and find cycles.

    Bool* node_was_visited = allocate_array(scratch, nodes_count, Bool);

    enum { JUST_DEFINITION = -1 };
    struct Stack_Element
    {
        Index node_index;
        Index reachable_definition_index;
    };
    typedef struct Stack_Element Stack_Element;

    Stack_Element* stack = allocate_array(scratch, nodes_count, Stack_Element);
    Index stack_index = 0;

    for (Index this_node_index = 0;
         this_node_index < nodes_count;
         ++this_node_index)
    {
        fill_with_zeros(node_was_visited, nodes_count, Bool);

        stack_index = 0;
        stack[stack_index++] = (Stack_Element) {this_node_index, JUST_DEFINITION};

        while (stack_index > 0)
        {
            const Stack_Element element = stack[--stack_index];

            const Index node_index = element.node_index;
            const Index reachable_definition_index = element.reachable_definition_index;

            const Definition_Node* node = &nodes[node_index];

            if (reachable_definition_index == JUST_DEFINITION)
            {
                if (node_was_visited[node_index])
                {
                    found_errors = true;

                    const Definition_Node* start_node = &nodes[this_node_index];

                    const Ast_Identifier* identifier = &start_node->definition->identifier;
                    const Token* token = &identifier->token;

                    println("{}:{}:{}: Error: left recursion detected in {} definition",
                            grammar_filename, token->line+1, token->column+1,
                            token->lexeme);
                    show_grammar_error(scratch,
                                       grammar,
                                       token->line,
                                       token->column,
                                       token->lexeme.length);
                    // TODO(vlad): Print recursion path.
                    // NOTE(vlad): Actually we are finding cycles in this function.
                    //             It would make sense to print them here showing
                    //             every relevant expression on its path.
                    //             Also print its length.

                    break;
                }

                for (Index definition_index = 0;
                     definition_index < node->reachable_definitions_count;
                     ++definition_index)
                {
                    stack[stack_index++] = (Stack_Element) { node_index, definition_index };
                }

                node_was_visited[node_index] = true;
            }
            else
            {
                stack[stack_index++] = (Stack_Element) {
                    .node_index = node->reachable_definitions[reachable_definition_index],
                    .reachable_definition_index = JUST_DEFINITION,
                };
            }
        }
    }

    return found_errors;
}

internal Bool
check_grammar_soundness(const String_View grammar_filename, const String_View grammar)
{
    println("\nChecking '{}' soundness", grammar_filename);

    Arena* arena = arena_create(GiB(1), MiB(1));
    Arena* scratch = arena_create(GiB(1), MiB(1));

    Lexer lexer = {0};
    lexer_create(arena, &lexer, grammar);

    Parser parser = {0};
    parser_create(&parser, &lexer);

    Ast ast = {0};
    {
        const Timestamp parsing_start = platform_get_current_monotonic_timestamp();
        const Bool result = parser_parse(arena, scratch, &parser, &ast);
        const Timestamp parsing_end = platform_get_current_monotonic_timestamp();

        println("Grammar parsed in {} msc", parsing_end - parsing_start);

        if (!result)
        {
            println("Failed to parse grammar");
            return false;
        }
    }

    Grammar_Info info = {0};
    for (Index definition_index = 0;
         definition_index < ast.definitions_count;
         ++definition_index)
    {
        const Ast_Identifier_Definition* definition = &ast.definitions[definition_index];

        add_identifier(arena, &info.defined_identifiers, &definition->identifier.token);
    }

    Bool found_errors = false;

    // NOTE(vlad): Test that all identifiers were defined.
    for (Index definition_index = 0;
         definition_index < ast.definitions_count;
         ++definition_index)
    {
        const Ast_Identifier_Definition* definition = &ast.definitions[definition_index];

        for (Index expression_index = 0;
             expression_index < definition->possible_expressions_count;
             ++expression_index)
        {
            const Ast_Expression* expression = &definition->possible_expressions[expression_index];

            for (Index identifier_index = 0;
                 identifier_index < expression->identifiers_count;
                 ++identifier_index)
            {
                const Ast_Identifier* identifier = &expression->identifiers[identifier_index];

                if (identifier->token.type == TOKEN_NON_TERMINAL
                    && !has_identifier(&info.defined_identifiers, &identifier->token))
                {
                    if (!has_identifier(&info.undefined_identifiers, &identifier->token))
                    {
                        add_identifier(arena, &info.undefined_identifiers, &identifier->token);
                    }
                }
            }
        }
    }

    if (info.undefined_identifiers.tokens_count != 0)
    {
        found_errors = true;

        for (Index token_index = 0;
             token_index < info.undefined_identifiers.tokens_count;
             ++token_index)
        {
            const Token* token = info.undefined_identifiers.tokens[token_index];
            println("{}:{}:{}: Error: identifier '{}' is undefined",
                    grammar_filename, token->line+1, token->column+1,
                    token->lexeme);
            show_grammar_error(scratch,
                               grammar,
                               token->line,
                               token->column,
                               token->lexeme.length);
        }
    }

    {
        const Timestamp time_start = platform_get_current_monotonic_timestamp();
        found_errors |= detect_left_recursions(scratch, grammar_filename, grammar, &ast);
        const Timestamp time_end = platform_get_current_monotonic_timestamp();

        println("Left recursions detected in {} mcs", time_end - time_start);
    }

    // FIXME(vlad): Detect if there are common subexpressions that can be refactored to their own production rules.
    //              Start with detecting proper prefixes like these:
    //
    //                  foo: a b c
    //                  bar: a b c d

    parser_destroy(&parser);
    lexer_destroy(&lexer);

    arena_destroy(scratch);
    arena_destroy(arena);

    return !found_errors;
}

#include <eon/memory.c>
#include <eon/string.c>
#include <eon/io.c>

#include "grammar_lexer.c"
#include "grammar_log.c"
#include "grammar_parser.c"
