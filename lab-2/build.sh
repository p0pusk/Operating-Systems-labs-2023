#!/usr/bin/sh

set -xe

CFLAGS="-std=c++17 -Wall -Wextra"
SRC="./main.cpp"

clang++ $SRC $CFLAGS -o lab2
