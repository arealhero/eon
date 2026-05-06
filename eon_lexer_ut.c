#include <eon/unit_test.h>

#include "eon_lexer.h"
#include "eon_compilation_context.h"

internal void
test_line_comments(Test_Context* test_context)
{
    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("// line comment");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};
        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("// nested // line // comments");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};
        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("// line comment\n123");

            Compilation_Context context = {0};
            Lexer lexer = {0};

            create_compilation_context(&context, &source);
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(token.type, TOKEN_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "123");
            ASSERT_EQUAL(token.location.line, 1);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("2 + // line comment\n2");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 1);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    // NOTE(vlad): Block comments tests.

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("2 + /* block comment */ 2");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 24);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("2 + /* nested /* block /* comments */ */ */ 2");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 44);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("/* // line comment inside block comment is ignored */ 2");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 54);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }
}

internal void
test_numbers(Test_Context* test_context)
{
    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("2 + 2");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("1234567890 + 999999999999999");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "1234567890");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 11);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "999999999999999");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 13);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    // NOTE(vlad): Testing floating-point numbers.
    {
        {
            Source_File source = {0};
            source.filename = string_view("<input>");
            source.code = string_view("0.1");

            Compilation_Context context = {0};
            Lexer lexer = {0};

            create_compilation_context(&context, &source);
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "0.1");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
        }

        {
            Source_File source = {0};
            source.filename = string_view("<input>");
            source.code = string_view("a := 0.1;");

            Compilation_Context context = {0};
            Lexer lexer = {0};

            create_compilation_context(&context, &source);
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_COLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 2);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_ASSIGN);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "=");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 3);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "0.1");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 5);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_SEMICOLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ";");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 8);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
        }
    }

    // FIXME(vlad): Test errors like '123a'.

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("&");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_AMPERSAND);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "&");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }
}

internal void
test_identifiers(Test_Context* test_context)
{
    // NOTE(vlad): Variables tests.

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("a + b");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "b");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("_hello world");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "_hello");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "world");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 7);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    // NOTE(vlad): Functions tests.

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("main: () = {}");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "main");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_COLON);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_LEFT_PAREN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "(");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 6);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_RIGHT_PAREN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ")");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 7);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_ASSIGN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "=");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 9);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_LEFT_BRACE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "{");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 11);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_RIGHT_BRACE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "}");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 12);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    // NOTE(vlad): Testing arrays.
    {
        {
            Source_File source = {0};
            source.filename = string_view("<input>");
            source.code = string_view("arr: [] s32;");

            Compilation_Context context = {0};
            Lexer lexer = {0};

            create_compilation_context(&context, &source);
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "arr");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_COLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 3);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_LEFT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "[");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 5);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_RIGHT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "]");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 6);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "s32");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 8);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_SEMICOLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ";");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 11);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
        }

        {
            Source_File source = {0};
            source.filename = string_view("<input>");
            source.code = string_view("arr: [] [] s32;");

            Compilation_Context context = {0};
            Lexer lexer = {0};

            create_compilation_context(&context, &source);
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "arr");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_COLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 3);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_LEFT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "[");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 5);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_RIGHT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "]");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 6);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_LEFT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "[");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 8);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_RIGHT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "]");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 9);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "s32");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 11);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_SEMICOLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ";");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 14);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_EQUAL(context.errors_count, 0);
            ASSERT_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
        }
    }
}

internal void
test_keywords_and_digraphs(Test_Context* test_context)
{
    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("for a");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_FOR);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "for");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("return");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_RETURN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "return");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("->");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_ARROW);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "->");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("mutable");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_MUTABLE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "mutable");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }

    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("_");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_WILDCARD);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "_");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_EQUAL(context.errors_count, 0);
        ASSERT_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
    }
}

internal void
test_errors(Test_Context* test_context)
{
    {
        Source_File source = {0};
        source.filename = string_view("<input>");
        source.code = string_view("?");

        Compilation_Context context = {0};
        Lexer lexer = {0};

        create_compilation_context(&context, &source);
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_FALSE(get_next_token(&lexer, &token));

        ASSERT_EQUAL(context.errors_count, 1);

        const Error* error = &context.errors[0];
        ASSERT_EQUAL(error->location.line, 0);
        ASSERT_EQUAL(error->location.column, 0);
        ASSERT_STRINGS_ARE_EQUAL(error->message, "Unexpected character encountered");

        destroy_lexer(&lexer);
    }
}

REGISTER_TESTS(
    test_line_comments,
    test_numbers,
    test_identifiers,
    test_keywords_and_digraphs,
    test_errors
)

#include "eon_compilation_context.c"
#include "eon_lexer.c"
