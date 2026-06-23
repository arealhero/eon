#include "eon_unit_test.h"

#include "eon_lexer.h"
#include "eon_compilation_context.h"

internal void
test_line_comments(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("// line comment");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};
        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("// nested // line // comments");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};
        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("// line comment\n123");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "123");
        ASSERT_EQUAL(token.location.line, 1);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("2 + // line comment\n2");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 1);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Block comments tests.

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("2 + /* block comment */ 2");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 24);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("2 + /* nested /* block /* comments */ */ */ 2");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 44);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("/* // line comment inside block comment is ignored */ 2");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 54);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_numbers(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("2 + 2");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "2");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("1234567890 + 999999999999999");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "1234567890");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 11);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "999999999999999");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 13);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing floating-point numbers.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("0.1");

            Lexer lexer = {0};
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "0.1");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("a := 0.1;");

            Lexer lexer = {0};
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_COLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 2);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_ASSIGN);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "=");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 3);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_NUMBER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "0.1");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 5);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_SEMICOLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ";");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 8);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("&");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_AMPERSAND);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "&");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_identifiers(Test_Context* test_context)
{
    // NOTE(vlad): Variables tests.

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("a + b");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "+");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 2);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "b");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("_hello world");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "_hello");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "world");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 7);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Functions tests.

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("main: () = {}");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "main");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_COLON);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_LEFT_PAREN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "(");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 6);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_RIGHT_PAREN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ")");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 7);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_ASSIGN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "=");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 9);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_LEFT_BRACE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "{");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 11);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_RIGHT_BRACE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "}");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 12);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    // NOTE(vlad): Testing arrays.
    {
        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("arr: [] s32;");

            Lexer lexer = {0};
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "arr");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_COLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 3);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_LEFT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "[");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 5);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_RIGHT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "]");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 6);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "s32");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 8);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_SEMICOLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ";");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 11);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }

        {
            CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("arr: [] [] s32;");

            Lexer lexer = {0};
            create_lexer(&lexer, &context);

            Token token = {0};

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "arr");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 0);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_COLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ":");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 3);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_LEFT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "[");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 5);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_RIGHT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "]");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 6);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_LEFT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "[");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 8);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_RIGHT_BRACKET);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "]");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 9);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "s32");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 11);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_SEMICOLON);
            ASSERT_STRINGS_ARE_EQUAL(token.lexeme, ";");
            ASSERT_EQUAL(token.location.line, 0);
            ASSERT_EQUAL(token.location.column, 14);

            ASSERT_TRUE(get_next_token(&lexer, &token));
            ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
            ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

            destroy_lexer(&lexer);
            destroy_compilation_context(&context);
        }
    }
}

internal void
test_keywords_and_digraphs(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("for a");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_FOR);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "for");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "a");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 4);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("return");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_RETURN);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "return");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("->");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_ARROW);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "->");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("mutable");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_MUTABLE);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "mutable");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }

    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("_");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_WILDCARD);
        ASSERT_STRINGS_ARE_EQUAL(token.lexeme, "_");
        ASSERT_EQUAL(token.location.line, 0);
        ASSERT_EQUAL(token.location.column, 0);

        ASSERT_TRUE(get_next_token(&lexer, &token));
        ASSERT_THAT_THERE_ARE_NO_DIAGNOSTIC_MESSAGES();
        ASSERT_ENUM_VALUES_ARE_EQUAL(token.type, TOKEN_EOF);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

internal void
test_errors(Test_Context* test_context)
{
    {
        CREATE_TEST_COMPILATION_CONTEXT_FOR_CODE("?");

        Lexer lexer = {0};
        create_lexer(&lexer, &context);

        Token token = {0};
        ASSERT_FALSE(get_next_token(&lexer, &token));

        ASSERT_TRUE(has_compilation_errors(&context));

        const String dumped_messages = dump_diagnostic_messages(test_context->arena,
                                                         &context,
                                                         MAX_MESSAGE_LEVEL);

        const String_View expected_output = string_view("<test-input>:1:1: error: Unexpected character encountered\n"
                                                        "  1 | ?\n"
                                                        "    | ^");
        ASSERT_STRINGS_ARE_EQUAL(dumped_messages, expected_output);

        destroy_lexer(&lexer);
        destroy_compilation_context(&context);
    }
}

REGISTER_TESTS(
    test_line_comments,
    test_numbers,
    test_identifiers,
    test_keywords_and_digraphs,
    test_errors
)

#include "eon_cfg.c"
#include "eon_compilation_context.c"
#include "eon_diagnostics.c"
#include "eon_lexer.c"
#include "eon_tac.c"
#include "eon_types.c"
