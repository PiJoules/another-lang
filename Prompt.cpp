#include <iostream>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"
#include "ReadInput.h"

void RunPrompt() {
  std::cout << "Lang Version 0.0.0.0.1" << std::endl;
  std::cout << "Press Ctrl+c to Exit" << std::endl;

  lang::ASTDump dumper(std::cerr);

  while (true) {
    char* input_cstr = readline("lispy> ");
    std::string input(input_cstr);
    add_history(input_cstr);
    free(input_cstr);

    std::cout << "Input: '" << input << "'" << std::endl;

    // <TESTING CODE>
    std::stringstream input_stream(input);
    lang::Parser parser(input_stream);
    std::unique_ptr<lang::Node> result = parser.Parse();
    if (result) {
      dumper.Dump(*result);
      std::cerr << std::endl;

      if (lang::NodeIsExpr(*result)) {
        std::cerr << "Value: " << lang::ASTEval().EvalNumericExpr(*result)
                  << std::endl;
      }
    } else {
      parser.getFailure().Dump(std::cerr);
    }
    // </TESTING CODE>
  }
}

int main(int argc, char** argv) {
  RunPrompt();
  return 0;
}
