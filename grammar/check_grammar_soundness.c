#include <eon/common.h>
#include <eon/string.h>

#include "grammar_lexer.h"
#include "grammar_log.h"
#include "grammar_parser.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

internal bool32 check_grammar_soundness(const char* grammar_filename, const String_View grammar);

int
main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <grammar-file>\n", argv[0]);
        return 1;
    }

    const char* grammar_filename = argv[1];

    const int fd = open(grammar_filename, O_RDONLY);
    if (fd == -1)
    {
        printf("Failed to open '%s'", grammar_filename);
        perror("");
        return 1;
    }

    struct stat file_stats = {0};
    fstat(fd, &file_stats);

    ssize content_length = file_stats.st_size;

    if (content_length == 0)
    {
        printf("Error: file '%s' is empty.\n",
               grammar_filename);
        return 1;
    }

    char* content = (char*) calloc(sizeof(char),
                                   (usize)(content_length + 1));

    read(fd, content, (usize)content_length);
    close(fd);

    String_View grammar = {0};
    grammar.data = content;
    grammar.length = content_length;

    bool32 result = check_grammar_soundness(grammar_filename, grammar);

    free(content);

    return !result;
}

struct Identifiers_Array
{
    const Token** tokens;
    ssize tokens_count;
    ssize tokens_capacity;
};
typedef struct Identifiers_Array Identifiers_Array;

internal void
add_identifier(Arena* arena,
               Identifiers_Array* array,
               const Token* identifier_token)
{
    if (array->tokens_count == array->tokens_capacity)
    {
        const ssize new_capacity = MAX(1, 2 * array->tokens_capacity);

        const ssize element_size = size_of(array->tokens[0]);
        const ssize old_size = array->tokens_capacity * element_size;
        const ssize new_size = new_capacity * element_size;

        array->tokens = arena_reallocate(arena,
                                         as_bytes(array->tokens),
                                         old_size,
                                         new_size);
        array->tokens_capacity = new_capacity;
    }

    array->tokens[array->tokens_count] = identifier_token;
    array->tokens_count += 1;
}

internal bool32
has_identifier(Identifiers_Array* array,
               const Token* identifier_token)
{
    for (ssize i = 0;
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

internal bool32
check_grammar_soundness(const char* grammar_filename, const String_View grammar)
{
    Arena* arena = arena_create(GiB(1), MiB(1));
    Arena* scratch = arena_create(GiB(1), MiB(1));

    Lexer lexer = {0};
    lexer_create(&lexer, grammar);

    Parser parser = {0};
    parser_create(&parser, &lexer);

    Ast ast;
    // FIXME(vlad): Check 'parser_parse' result.
    if (!parser_parse(arena, scratch, &parser, &ast))
    {
        printf("Failed to parse grammar\n");
        return false;
    }

    Grammar_Info info = {0};
    for (ssize definition_index = 0;
         definition_index < ast.definitions_count;
         ++definition_index)
    {
        Ast_Identifier_Definition* definition = &ast.definitions[definition_index];

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

    bool32 found_errors = false;

    // NOTE(vlad): Test that all identifiers were defined.
    for (ssize definition_index = 0;
         definition_index < ast.definitions_count;
         ++definition_index)
    {
        Ast_Identifier_Definition* definition = &ast.definitions[definition_index];

        for (ssize expression_index = 0;
             expression_index < definition->possible_expressions_count;
             ++expression_index)
        {
            Ast_Expression* expression = &definition->possible_expressions[expression_index];

            for (ssize identifier_index = 0;
                 identifier_index < expression->identifiers_count;
                 ++identifier_index)
            {
                Ast_Identifier* identifier = &expression->identifiers[identifier_index];

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

        for (ssize token_index = 0;
             token_index < info.undefined_identifiers.tokens_count;
             ++token_index)
        {
            const Token* token = info.undefined_identifiers.tokens[token_index];
            printf("%s:%ld:%ld: Identifier '%.*s' is undefined\n",
                   grammar_filename, token->line+1, token->column+1,
                   FORMAT_STRING(token->lexeme));
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
    for (ssize definition_index = 0;
         definition_index < ast.definitions_count;
         ++definition_index)
    {
        const Ast_Identifier_Definition* definition = &ast.definitions[definition_index];
        const Ast_Identifier* identifier = &definition->identifier;

        if (definition->possible_expressions_count == 0)
        {
            found_errors = true;

            const Token* token = &identifier->token;

            printf("%s:%ld:%ld: Definition for '%.*s' has no possible expressions\n",
                   grammar_filename, token->line+1, token->column+1,
                   FORMAT_STRING(token->lexeme));
            show_grammar_error(scratch,
                               grammar,
                               token->line,
                               token->column,
                               token->lexeme.length);
        }
        else
        {
            for (ssize expression_index = 0;
                 expression_index < definition->possible_expressions_count;
                 ++expression_index)
            {
                Ast_Expression* expression = &definition->possible_expressions[expression_index];

                if (expression->identifiers_count == 0)
                {
                    found_errors = true;

                    const Token* token = &identifier->token;

                    printf("%s:%ld:%ld: Empty expression detected in '%.*s' definition\n",
                           grammar_filename, token->line+1, token->column+1,
                           FORMAT_STRING(token->lexeme));
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

                    printf("%s:%ld:%ld: Left recursion detected in '%.*s' definition\n",
                           grammar_filename, token->line+1, token->column+1,
                           FORMAT_STRING(token->lexeme));
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

#include "grammar_lexer.c"
#include "grammar_log.c"
#include "grammar_parser.c"
