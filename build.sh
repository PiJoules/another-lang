#!/bin/bash

set -e

# The main executable
clang++ -std=c++14 -g -Werror -Wall -fno-rtti -ledit -o lang.out Prompt.cpp Lexer.cpp Parser.cpp Nodes.cpp LangCommon.cpp

clang++ -std=c++14 -Werror -Wall -fno-rtti -o TestLexer.out TestLexer.cpp Lexer.cpp LangCommon.cpp -pthread -lgtest
clang++ -std=c++14 -Werror -Wall -fno-rtti -o TestParser.out TestParser.cpp Parser.cpp Lexer.cpp LangCommon.cpp -pthread -lgtest
clang++ -std=c++14 -Werror -Wall -fno-rtti -o TestEval.out TestEval.cpp Parser.cpp Lexer.cpp Nodes.cpp LangCommon.cpp -pthread -lgtest
