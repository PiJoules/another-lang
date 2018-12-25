#!/bin/bash

set -e

CXX=clang++
CXXOPTS="-std=c++14 -g -Werror -Wall -fno-rtti"

INCLUDE_DIR=include
LIB_DIR=lib

$CXX $CXXOPTS -c -I $INCLUDE_DIR -o Prompt.o            lib/Prompt.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o Lexer.o             lib/Lexer.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o Parser.o            lib/Parser.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o Nodes.o             lib/Nodes.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o LangCommon.o        lib/LangCommon.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o ASTEval.o           lib/ASTEval.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o SemanticAnalysis.o  lib/SemanticAnalysis.cpp &

$CXX $CXXOPTS -c -I $INCLUDE_DIR -o TestLexer.o     tests/TestLexer.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o TestParser.o    tests/TestParser.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o TestEval.o      tests/TestEval.cpp &
$CXX $CXXOPTS -c -I $INCLUDE_DIR -o TestASTDump.o   tests/TestASTDump.cpp &

wait

# The main executable
$CXX $CXXOPTS -ledit -o lang.out Prompt.o Lexer.o Parser.o Nodes.o LangCommon.o ASTEval.o SemanticAnalysis.o &

# Tests
$CXX $CXXOPTS -o TestLexer.out TestLexer.o Lexer.o LangCommon.o -pthread -lgtest &
$CXX $CXXOPTS -o TestParser.out TestParser.o Parser.o Lexer.o LangCommon.o -pthread -lgtest &
$CXX $CXXOPTS -o TestEval.out TestEval.o Parser.o Lexer.o Nodes.o LangCommon.o ASTEval.o SemanticAnalysis.o -pthread -lgtest &
$CXX $CXXOPTS -o TestASTDump.out TestASTDump.o Nodes.o LangCommon.o -pthread -lgtest &

wait
