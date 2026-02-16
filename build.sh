#!/bin/sh

mkdir -p build

set -e

clang_warnings="
  -pedantic
  -Wall
  -Wextra
  -Werror
  -Wconversion
  -Wshadow

  -Wno-variadic-macro-arguments-omitted

  -Wno-error=unused-function
  -Wno-error=unused-variable
  -Wno-error=unused-parameter
"
clang_common_flags="
  -std=gnu11
  -O0
  -ggdb
  -I.
  -fsanitize=address
  -fno-omit-frame-pointer
"

compile()
{
    echo
    echo "Compiling '$1'"
    TIMEFORMAT="Compilation took %2R seconds"
    time clang $@
}

mkdir -p build/grammar

compile grammar/check_grammar_soundness.c -o build/grammar/check_grammar_soundness \
        $clang_common_flags \
        $clang_warnings

./build/grammar/check_grammar_soundness grammar/eon-grammar

compile eon.c -o build/eon \
        $clang_common_flags \
        $clang_warnings

compile_and_run_unit_test()
{
    test_filename="$1"
    test_dir=$(dirname "$test_filename")
    test_name=$(basename -s _ut.c "$test_filename")

    shift
    additional_flags="$@"

    mkdir -p "build/tests/$test_dir"

    compile "$test_filename" -o "build/tests/$test_dir/$test_name" \
            $clang_common_flags \
            $clang_warnings \
            $additional_flags

    if [ "$test_dir" = "." ];
    then
        reported_test_name="$test_name"
    else
        reported_test_name="$test_dir/$test_name"
    fi

    echo "Running tests in '$reported_test_name'"
    "./build/tests/$test_dir/$test_name" --hide-stats
}

compile_and_run_unit_test eon/memory_ut.c
compile_and_run_unit_test eon/string_ut.c
compile_and_run_unit_test eon_lexer_ut.c
compile_and_run_unit_test eon_parser_ut.c
compile_and_run_unit_test eon_interpreter_ut.c
compile_and_run_unit_test eon/sanitizers/asan_ut.c -fsanitize=address -fsanitize-recover=address

mkdir -p build/tests
compile tests/run_test.c -o build/tests/run_test \
        $clang_common_flags \
        $clang_warnings

run_test()
{
    test_directory="$1"
    test_name=$(basename "$test_directory")
    eon_executable="build/eon"

    echo
    echo "Running test '$test_name'"
    "build/tests/run_test" "$eon_executable" "$test_directory"
}

echo
echo " === Running interpreter tests ==="

run_test tests/calls
# run_test tests/empty-file
run_test tests/empty-main-with-return
run_test tests/factorial
run_test tests/fibonacci
run_test tests/fibonacci-without-recursion
run_test tests/simple-floats-operations
