#!/bin/sh

rm -rf build
mkdir -p build

set -e

USE_GCC=0

# NOTE(vlad): https://stackoverflow.com/a/70209891
if [ $(uname) = "Darwin" ]; then
    export MallocNanoZone=0
fi

compiler_warnings="
  -pedantic
  -Wall
  -Wextra
  -Werror
  -Wconversion
  -Wshadow
  -Wunreachable-code

  -Wno-error=unused-function
  -Wno-error=unused-variable
  -Wno-error=unused-parameter
"
compiler_common_flags="
  -std=gnu11
  -O0
  -ggdb
  -I.
  -fsanitize=address
  -fno-omit-frame-pointer
"

if [ $USE_GCC -eq 1 ];
then
    compiler_warnings="$compiler_warnings -Wno-type-limits"
else
    compiler_common_flags="$compiler_common_flags -ferror-limit=0"
    compiler_warnings="$compiler_warnings -Wno-variadic-macro-arguments-omitted"
fi

compile()
{
    echo
    echo "Compiling '$1'"
    TIMEFORMAT="Compilation took %2R seconds"

    if [ $USE_GCC -eq 1 ];
    then
        time gcc $@
    else
        time clang $@
    fi
}

mkdir -p build/grammar

compile grammar/check_grammar_soundness.c -o build/grammar/check_grammar_soundness \
        $compiler_common_flags \
        $compiler_warnings

./build/grammar/check_grammar_soundness grammar/eon-grammar

compile_and_run_unit_test()
{
    test_filename="$1"
    test_dir=$(dirname "$test_filename")
    test_name=$(basename -s _ut.c "$test_filename")

    shift
    additional_flags="$@"

    mkdir -p "build/tests/$test_dir"

    compile "$test_filename" -o "build/tests/$test_dir/$test_name" \
            $compiler_common_flags \
            $compiler_warnings \
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

# compile_and_run_unit_test eon/memory_ut.c
# compile_and_run_unit_test eon/containers_ut.c
# compile_and_run_unit_test eon/string_ut.c
# compile_and_run_unit_test eon_lexer_ut.c
# compile_and_run_unit_test eon_parser_ut.c
# compile_and_run_unit_test eon_lexical_scopes_ut.c
# compile_and_run_unit_test eon_types_ut.c
# compile_and_run_unit_test eon_tac_ut.c
# compile_and_run_unit_test eon_cfg_ut.c

# compile_and_run_unit_test eon/sanitizers/asan_ut.c -fsanitize=address -fsanitize-recover=address

# exit 0

mkdir -p build/tests/ssa-tests
compile tests/ssa-tests/run_ssa_test.c -o build/tests/ssa-tests/run_ssa_test \
        $compiler_common_flags \
        $compiler_warnings

run_ssa_test()
{
    test_directory="$1"
    test_name=$(basename "$test_directory")

    echo
    echo "Running SSA test '$test_name'"
    "build/tests/ssa-tests/run_ssa_test" "$test_directory"
}

# run_ssa_test tests/ssa-tests/general-cases
run_ssa_test tests/ssa-tests/regression-if-statement-with-return
run_ssa_test tests/ssa-tests/regression-nested-if-statement

exit 0

mkdir -p build/tests
compile tests/run_test.c -o build/tests/run_test \
        $compiler_common_flags \
        $compiler_warnings

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
run_test tests/empty-file
run_test tests/empty-main-with-return
run_test tests/factorial
run_test tests/fibonacci
run_test tests/fibonacci-without-recursion
run_test tests/simple-floats-operations
run_test tests/square-root
