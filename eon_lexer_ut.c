#include <eon/unit_test.h>

#include "eon_lexer.h"

internal void
test_line_comments(Test_Context* context)
{
    Errors errors = {0};
    errors.errors_arena = context->arena;

    {
        const String_View input = string_view("// line comment");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};
        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("// nested // line // comments");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};
        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("// line comment\n123");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "123");
        ASSERT_EQUAL(token.line, 1);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("2 + // line comment\n2");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 2);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 1);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Block comments tests.

    {
        const String_View input = string_view("2 + /* block comment */ 2");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 2);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 24);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("2 + /* nested /* block /* comments */ */ */ 2");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 2);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 44);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("/* // line comment inside block comment is ignored */ 2");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 54);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }
}

internal void
test_numbers(Test_Context* context)
{
    Errors errors = {0};
    errors.errors_arena = context->arena;

    {
        const String_View input = string_view("2 + 2");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 2);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 4);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("1234567890 + 999999999999999");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "1234567890");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 11);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "999999999999999");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 13);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    // FIXME(vlad): Test errors like '123a'.
}

internal void
test_identifiers(Test_Context* context)
{
    Errors errors = {0};
    errors.errors_arena = context->arena;

    // NOTE(vlad): Variables tests.

    {
        const String_View input = string_view("a + b");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 2);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "b");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 4);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("_hello world");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "_hello");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "world");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 7);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    // NOTE(vlad): Functions tests.

    {
        const String_View input = string_view("main: () = {}");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "main");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_COLON);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 4);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_LEFT_PAREN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "(");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 6);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_RIGHT_PAREN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ")");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 7);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_ASSIGN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "=");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 9);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_LEFT_BRACE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "{");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 11);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_RIGHT_BRACE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "}");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 12);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }
}

internal void
test_keywords_and_digraphs(Test_Context* context)
{
    Errors errors = {0};
    errors.errors_arena = context->arena;

    {
        const String_View input = string_view("for a");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_FOR);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "for");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 4);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("return");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_RETURN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "return");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("->");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_ARROW);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "->");
        ASSERT_EQUAL(token.line, 0);
        ASSERT_EQUAL(token.column, 0);

        ASSERT_TRUE(lexer_get_next_token(&lexer, &token, &errors));
        ASSERT_EQUAL(errors.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        lexer_destroy(&lexer);
    }
}

internal void
test_errors(Test_Context* context)
{
    Errors errors = {0};
    errors.errors_arena = context->arena;

    {
        const String_View input = string_view("?");

        Lexer lexer = {0};
        lexer_create(&lexer, string_view("<input>"), input);

        Token token = {0};

        ASSERT_FALSE(lexer_get_next_token(&lexer, &token, &errors));

        ASSERT_EQUAL(errors.errors_count, 1);

        const Error* error = &errors.errors[0];
        ASSERT_EQUAL(error->line, 0);
        ASSERT_EQUAL(error->column, 0);
        ASSERT_STRINGS_ARE_EQUAL(error->message, "Unexpected character encountered");

        clear_errors(&errors);
        lexer_destroy(&lexer);
    }
}

REGISTER_TESTS(
    test_line_comments,
    test_numbers,
    test_identifiers,
    test_keywords_and_digraphs,
    test_errors
)

#include "eon_errors.c"
#include "eon_lexer.c"
