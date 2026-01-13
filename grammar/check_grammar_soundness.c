#include <eon/common.h>
#include <eon/io.h>
#include <eon/string.h>

#include <eon/platform/filesystem.h>

#include "grammar_lexer.h"
#include "grammar_log.h"
#include "grammar_parser.h"

internal Bool check_grammar_soundness(const String_View grammar_filename, const String_View grammar);

int
main(int argc, const char* argv[])
{
    init_io_state(MiB(10));

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

internal Bool
check_grammar_soundness(const String_View grammar_filename, const String_View grammar)
{
    Arena* arena = arena_create(GiB(1), MiB(1));
    Arena* scratch = arena_create(GiB(1), MiB(1));

    Lexer lexer = {0};
    lexer_create(&lexer, grammar);

    Parser parser = {0};
    parser_create(&parser, &lexer);

    // TODO(vlad): Time the parsing.

    Ast ast = {0};
    if (!parser_parse(arena, scratch, &parser, &ast))
    {
        println("Failed to parse grammar");
        return false;
    }

    Grammar_Info info = {0};
    for (Index definition_index = 0;
         definition_index < ast.definitions_count;
         ++definition_index)
    {
        const Ast_Identifier_Definition* definition = &ast.definitions[definition_index];

        add_identifier(arena, &info.defined_identifiers, &definition->identifier.token);
    }

    // printf("Found %ld defined identifiers\n", info.defined_identifiers.tokens_count);
    // for (ssize token_index = 0;
    //      token_index < info.defined_identifiers.tokens_count;
    //      ++token_index)
    // {
    //     const Token* token = info.defined_identifiers.tokens[token_index];
    //     printf("%2ld. %.*s\n", token_index+1, FORMAT_STRING(token->lexeme));
    // }

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
            println("{}:{}:{}: Identifier '{}' is undefined",
                    grammar_filename, token->line+1, token->column+1,
                    token->lexeme);
            show_grammar_error(scratch,
                               grammar,
                               token->line,
                               token->column,
                               token->lexeme.length);
        }
    }

    // NOTE(vlad): Test that there are no left recursions in the grammar.
    //             Also test that every definition has at least 1 possible expression
    //             and every expression has at least one identifier to expand to (terminal or non-terminal).
    for (Index definition_index = 0;
         definition_index < ast.definitions_count;
         ++definition_index)
    {
        const Ast_Identifier_Definition* definition = &ast.definitions[definition_index];
        const Ast_Identifier* identifier = &definition->identifier;

        if (definition->possible_expressions_count == 0)
        {
            found_errors = true;

            const Token* token = &identifier->token;

            println("{}:{}:{}: Definition for '{}' has no possible expressions",
                    grammar_filename, token->line+1, token->column+1,
                    token->lexeme);
            show_grammar_error(scratch,
                               grammar,
                               token->line,
                               token->column,
                               token->lexeme.length);
        }
        else
        {
            for (Index expression_index = 0;
                 expression_index < definition->possible_expressions_count;
                 ++expression_index)
            {
                Ast_Expression* expression = &definition->possible_expressions[expression_index];

                if (expression->identifiers_count == 0)
                {
                    found_errors = true;

                    const Token* token = &identifier->token;

                    println("{}:{}:{}: Empty expression detected in '{}' definition",
                            grammar_filename, token->line+1, token->column+1,
                            token->lexeme);
                    show_grammar_error(scratch,
                                       grammar,
                                       token->line,
                                       token->column,
                                       token->lexeme.length);
                    continue;
                }

                Ast_Identifier* first_identifier = &expression->identifiers[0];

                if (strings_are_equal(identifier->token.lexeme, first_identifier->token.lexeme))
                {
                    found_errors = true;

                    const Token* token = &first_identifier->token;

                    println("{}:{}:{}: Left recursion detected in '{}' definition",
                            grammar_filename, token->line+1, token->column+1,
                            token->lexeme);
                    show_grammar_error(scratch,
                                       grammar,
                                       token->line,
                                       token->column,
                                       token->lexeme.length);
                }
            }
        }
    }

    // FIXME(vlad): Detect cycles and deep left recursions.
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
