#!/bin/bash

set -e

clang++ -std=c++14 -Werror -Wall -fno-rtti TestLexer.cpp Lexer.cpp -pthread -lgtest
valgrind ./a.out

clang++ -std=c++14 -Werror -Wall -fno-rtti TestParser.cpp Parser.cpp Lexer.cpp -pthread -lgtest
valgrind ./a.out
