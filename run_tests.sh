#!/bin/bash

set -e

source build.sh

valgrind ./TestLexer.out
valgrind ./TestParser.out
valgrind ./TestEval.out
