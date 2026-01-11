#pragma once

#include <eon/common.h>
#include <eon/io.h>
#include <eon/memory.h>
#include <eon/string.h>

#define TRAP_ON_FAILED_ASSERTS 0

#if !defined(TRAP_ON_FAILED_ASSERTS)
#    define TRAP_ON_FAILED_ASSERTS 0
#endif

enum Test_Result
{
    TEST_OK = 0,
    TEST_FAILED,
};
typedef enum Test_Result Test_Result;

struct Test_Context
{
    Arena* arena;

    Test_Result result;

    String_View failure_comment;
    String_View failure_file;
    ssize failure_line;
};
typedef struct Test_Context Test_Context;

typedef void (*Unit_Test)(Test_Context* context);

struct Test_Info
{
    Unit_Test run;
    String_View name;
};
typedef struct Test_Info Test_Info;

struct Tests_Registry
{
    String_View tests_filename;

    Test_Info* tests;
    ssize tests_capacity;

    ssize total_tests_count;
    ssize failed_tests_count;
};
typedef struct Tests_Registry Tests_Registry;

internal void registry_register_tests(Arena* arena, Tests_Registry* registry);
internal void registry_register_test(Arena* arena,
                                     Tests_Registry* registry,
                                     const Unit_Test test,
                                     const String_View test_name);

int
main(void)
{
    init_io_state(MiB(1));

    Arena* permanent_arena = arena_create(GiB(1), MiB(1));

    Tests_Registry registry = {0};
    registry_register_tests(permanent_arena, &registry);

    Arena* test_arena = arena_create(GiB(1), MiB(1));

    for (ssize i = 0;
         i < registry.total_tests_count;
         ++i)
    {
        Test_Context context = {0};
        context.arena = test_arena;

        Test_Info* test = &registry.tests[i];
        test->run(&context);

        if (context.result == TEST_FAILED)
        {
            // FIXME(vlad): Use 'println'.
            println("{}:{}: Test '{}' failed\n"
                   "{}",
                   context.failure_file,
                   context.failure_line,
                   test->name,
                   context.failure_comment);

            registry.failed_tests_count += 1;
        }
    }

    println("\n{} tests completed:\n"
           "  - Total tests count:  {}\n"
           "  - Failed tests count: {}",
           registry.tests_filename,
           registry.total_tests_count,
           registry.failed_tests_count);

    arena_destroy(test_arena);
    arena_destroy(permanent_arena);
}

#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>

#define REGISTER_TEST_IMPL(test) registry_register_test(arena, registry, test, string_view(#test));
#define REGISTER_TESTS(...)                                             \
    internal void                                                       \
    registry_register_tests(Arena* arena, Tests_Registry* registry)     \
    {                                                                   \
        registry->tests_filename = string_view(__FILE__);               \
        FOR_EACH(REGISTER_TEST_IMPL, __VA_ARGS__);                      \
    }

internal void
registry_register_test(Arena* arena,
                       Tests_Registry* registry,
                       const Unit_Test test,
                       const String_View test_name)
{
    if (registry->total_tests_count == registry->tests_capacity)
    {
        const ssize new_capacity = MAX(1, 2 * registry->tests_capacity);
        registry->tests = (Test_Info*) arena_reallocate(arena,
                                                        as_bytes(registry->tests),
                                                        size_of(Test_Info) * registry->tests_capacity,
                                                        size_of(Test_Info) * new_capacity);
        ASSERT(registry->tests != NULL);
        registry->tests_capacity = new_capacity;
    }

    Test_Info* new_test = &registry->tests[registry->total_tests_count];
    new_test->run = test;
    new_test->name = test_name;

    registry->total_tests_count += 1;
}

#define DEFAULT_COMMENT "Assertion failed"
#define ASSERT_EQUAL_IMPL(expression, actual, expected, comment, file, line) \
    if ((expression)) {}                                                \
    else                                                                \
    {                                                                   \
        context->result = TEST_FAILED;                                  \
        const String failure_comment = format_string(context->arena,    \
                                                     "    {}: {}:\n"    \
                                                     "        Expected '{}', got '{}'", \
                                                     (comment),         \
                                                     #expression,       \
                                                     (expected),        \
                                                     (actual));         \
        context->failure_comment = string_view(failure_comment);        \
        context->failure_file = string_view(file);                      \
        context->failure_line = line;                                   \
        if (TRAP_ON_FAILED_ASSERTS) { abort(); }                        \
        return;                                                         \
    }                                                                   \
    REQUIRE_SEMICOLON

#define ASSERT_EQUAL(actual, expected)                                  \
    ASSERT_EQUAL_IMPL((actual) == (expected), actual, expected, DEFAULT_COMMENT, __FILE__, __LINE__)

#define ASSERT_TRUE(expression) ASSERT_EQUAL(expression, true)
#define ASSERT_FALSE(expression) ASSERT_EQUAL(expression, false)

#define ASSERT_STRINGS_ARE_EQUAL(actual, expected)                      \
    ASSERT_EQUAL_IMPL(strings_are_equal(string_view((actual)), string_view((expected))), \
                      actual, expected, DEFAULT_COMMENT, __FILE__, __LINE__)

