#!/bin/sh

set -ex

clang -fsanitize=address eon.c -o eon
