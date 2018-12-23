#!/bin/bash

set -e

valgrind ./TestLexer.out
valgrind ./TestParser.out
valgrind ./TestEval.out
valgrind ./TestASTDump.out
