#include <eon/unit_test.h>

#include "eon_parser.h"

internal void
test_expression_conversion_to_rpn(Test_Context* context)
{
    {
        Token number_two = {0};
        number_two.type = TOKEN_NUMBER;
        number_two.lexeme = string_view("2");

        Literal_Expression literal_two = {0};
        literal_two.token = number_two;

        Expression two = {0};
        two.type = EXPRESSION_LITERAL;
        two.literal = &literal_two;

        Token operator_plus = {0};
        operator_plus.type = TOKEN_PLUS;
        operator_plus.lexeme = string_view("+");

        Binary_Expression sum = {0};
        sum.operator = operator_plus;
        sum.lhs = &two;
        sum.rhs = &two;

        Expression expression = {0};
        expression.type = EXPRESSION_BINARY;
        expression.binary = &sum;

        const String_View expression_in_rpn = convert_expression_to_prn(context->arena,
                                                                        &expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_rpn, "2 2 +");
    }
}

internal void
test_parsing_literals(Test_Context* context)
{
    {
        const String_View input = string_view("2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);

        ASSERT_EQUAL(expression->type, EXPRESSION_LITERAL);
        ASSERT_EQUAL(expression->literal->token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(expression->literal->token.lexeme, "2");

        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "2");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("hello");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);

        ASSERT_EQUAL(expression->type, EXPRESSION_LITERAL);
        ASSERT_EQUAL(expression->literal->token.type, TOKEN_IDENTIFIER);
        ASSERT_STRINGS_ARE_EQUAL(expression->literal->token.lexeme, "hello");

        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "hello");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("true");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);

        ASSERT_EQUAL(expression->type, EXPRESSION_LITERAL);
        ASSERT_EQUAL(expression->literal->token.type, TOKEN_TRUE);
        ASSERT_STRINGS_ARE_EQUAL(expression->literal->token.lexeme, "true");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("false");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);

        ASSERT_EQUAL(expression->type, EXPRESSION_LITERAL);
        ASSERT_EQUAL(expression->literal->token.type, TOKEN_FALSE);
        ASSERT_STRINGS_ARE_EQUAL(expression->literal->token.lexeme, "false");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

internal void
test_parsing_unary(Test_Context* context)
{
    // XXX(vlad): Is unary plus really useful? We can accept this production but report it as a compile error.
    //            I don't know if I want to do it though. Like, if we have this production already, why would we treat
    //            it as an error?
    //
    //            We can disable its support by default and let the user opt-in into unary '+' via a compiler flag.
    {
        const String_View input = string_view("+2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);

        ASSERT_EQUAL(expression->type, EXPRESSION_UNARY);

        Token* operator = &expression->unary->operator;
        ASSERT_EQUAL(operator->type, TOKEN_PLUS);
        ASSERT_STRINGS_ARE_EQUAL(operator->lexeme, "+");

        Expression* subexpression = expression->unary->expression;
        ASSERT_EQUAL(subexpression->type, EXPRESSION_LITERAL);
        ASSERT_EQUAL(subexpression->literal->token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(subexpression->literal->token.lexeme, "2");

        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "2 +");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("-2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);

        ASSERT_EQUAL(expression->type, EXPRESSION_UNARY);

        Token* operator = &expression->unary->operator;
        ASSERT_EQUAL(operator->type, TOKEN_MINUS);
        ASSERT_STRINGS_ARE_EQUAL(operator->lexeme, "-");

        Expression* subexpression = expression->unary->expression;
        ASSERT_EQUAL(subexpression->type, EXPRESSION_LITERAL);
        ASSERT_EQUAL(subexpression->literal->token.type, TOKEN_NUMBER);
        ASSERT_STRINGS_ARE_EQUAL(subexpression->literal->token.lexeme, "2");

        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "2 -");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("!false");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);

        ASSERT_EQUAL(expression->type, EXPRESSION_UNARY);

        Token* operator = &expression->unary->operator;
        ASSERT_EQUAL(operator->type, TOKEN_NOT);
        ASSERT_STRINGS_ARE_EQUAL(operator->lexeme, "!");

        Expression* subexpression = expression->unary->expression;
        ASSERT_EQUAL(subexpression->type, EXPRESSION_LITERAL);
        ASSERT_EQUAL(subexpression->literal->token.type, TOKEN_FALSE);
        ASSERT_STRINGS_ARE_EQUAL(subexpression->literal->token.lexeme, "false");

        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "false !");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

internal void
test_simple_expressions(Test_Context* context)
{
    {
        const String_View input = string_view("2 + 2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "2 2 +");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("2 - 2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "2 2 -");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("2 * 2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "2 2 *");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("2 / 2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "2 2 /");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("hello + true");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_prn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_prn, "hello true +");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

internal void
test_operator_precedence(Test_Context* context)
{
    {
        const String_View input = string_view("2 + 2 * 2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_rpn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_rpn, "2 2 2 * +");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("2 + (2 * 2)");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_rpn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_rpn, "2 2 2 * +");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }

    {
        const String_View input = string_view("(2 + 2) * 2");

        Lexer lexer = {0};
        Parser parser = {0};

        lexer_create(&lexer, input);
        parser_create(&parser, &lexer);

        Expression* expression = parser_parse(&parser);
        const String_View expression_in_rpn = convert_expression_to_prn(context->arena,
                                                                        expression);
        ASSERT_STRINGS_ARE_EQUAL(expression_in_rpn, "2 2 + 2 *");

        expression_destroy(expression);

        parser_destroy(&parser);
        lexer_destroy(&lexer);
    }
}

internal void
test_syntax_errors(Test_Context* context)
{
    // FIXME(vlad): Write the tests after implementing a centralized compile errors reporting system.
    UNUSED(context);
}

REGISTER_TESTS(
    test_expression_conversion_to_rpn,
    test_parsing_literals,
    test_parsing_unary,
    test_simple_expressions,
    test_operator_precedence,
    test_syntax_errors
)

#include "eon_parser.c"
#include "eon_lexer.c"
#include "eon_log.c"
