#!/bin/sh

mkdir -p build

set -ex

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
"

# clang eon.c -o build/eon \
#       $clang_common_flags \
#       $clang_warnings \
#       -lreadline

compile_and_run_test()
{
    test_filename="$1"
    test_dir=$(dirname "$test_filename")
    test_name=$(basename -s _ut.c "$test_filename")

    mkdir -p "build/tests/$test_dir"

    set -x
    clang "$test_filename" -o "build/tests/$test_dir/$test_name" \
          $clang_common_flags \
          $clang_warnings

    "./build/tests/$test_dir/$test_name"
    set +x
}

set +x
# compile_and_run_test eon/string_ut.c
# compile_and_run_test eon_lexer_ut.c
# compile_and_run_test eon_parser_ut.c

mkdir -p build/grammar
clang grammar/check_grammar_soundness.c -o build/grammar/check_grammar_soundness \
      $clang_common_flags \
      $clang_warnings
