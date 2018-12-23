#!/bin/bash

set -e

CXX=clang++
CXXOPTS="-std=c++14 -g -Werror -Wall -fno-rtti"

$CXX $CXXOPTS -c -o Prompt.o Prompt.cpp
$CXX $CXXOPTS -c -o Lexer.o Lexer.cpp
$CXX $CXXOPTS -c -o Parser.o Parser.cpp
$CXX $CXXOPTS -c -o Nodes.o Nodes.cpp
$CXX $CXXOPTS -c -o LangCommon.o LangCommon.cpp

$CXX $CXXOPTS -c -o TestLexer.o TestLexer.cpp
$CXX $CXXOPTS -c -o TestParser.o TestParser.cpp
$CXX $CXXOPTS -c -o TestEval.o TestEval.cpp
$CXX $CXXOPTS -c -o TestASTDump.o TestASTDump.cpp

# The main executable
$CXX $CXXOPTS -ledit -o lang.out Prompt.o Lexer.o Parser.o Nodes.o LangCommon.o

# Tests
$CXX $CXXOPTS -o TestLexer.out TestLexer.o Lexer.o LangCommon.o -pthread -lgtest
$CXX $CXXOPTS -o TestParser.out TestParser.o Parser.o Lexer.o LangCommon.o -pthread -lgtest
$CXX $CXXOPTS -o TestEval.out TestEval.o Parser.o Lexer.o Nodes.o LangCommon.o -pthread -lgtest
$CXX $CXXOPTS -o TestASTDump.out TestASTDump.o Nodes.o LangCommon.o -pthread -lgtest
