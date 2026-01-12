#!/bin/sh

mkdir -p build

set -e

clang_warnings="
  -pedantic
  -Wall
  -Wextra
  -Werror
  -Wconversion

  -Wno-variadic-macro-arguments-omitted

  -Wno-error=unused-function
  -Wno-error=unused-variable
  -Wno-error=unused-parameter
"
clang_common_flags="
  -O0
  -ggdb
  -I.
  -fsanitize=address
  -fno-omit-frame-pointer
"

compile()
{
    echo -e "\nCompiling '$1'"
    TIMEFORMAT="Compilation took %2R seconds"
    time clang $@
}

compile eon.c -o build/eon \
        $clang_common_flags \
        $clang_warnings \
        -lreadline

compile_and_run_test()
{
    test_filename="$1"
    test_dir=$(dirname "$test_filename")
    test_name=$(basename -s _ut.c "$test_filename")

    mkdir -p "build/tests/$test_dir"

    compile "$test_filename" -o "build/tests/$test_dir/$test_name" \
            $clang_common_flags \
            $clang_warnings

    if [ "$test_dir" = "." ];
    then
        reported_test_name="$test_name"
    else
        reported_test_name="$test_dir/$test_name"
    fi

    echo "Running tests in '$reported_test_name'"
    "./build/tests/$test_dir/$test_name" --hide-stats
}

compile_and_run_test eon/memory_ut.c
compile_and_run_test eon/string_ut.c
compile_and_run_test eon_lexer_ut.c
compile_and_run_test eon_parser_ut.c

mkdir -p build/grammar

compile grammar/check_grammar_soundness.c -o build/grammar/check_grammar_soundness \
        $clang_common_flags \
        $clang_warnings
