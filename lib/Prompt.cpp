#include <iostream>
#include <sstream>

#include "ASTEval.h"
#include "Parser.h"
#include "ReadInput.h"

void RunPrompt() {
  std::cout << "Lang Version 0.0.0.0.1" << std::endl;
  std::cout << "Press Ctrl+c to Exit" << std::endl;

  lang::ASTDump dumper(std::cerr);
  lang::ASTEval evaluator;

  while (true) {
    char* input_cstr = readline("lispy> ");
    std::string input(input_cstr);
    add_history(input_cstr);
    free(input_cstr);

    std::cout << "Input: '" << input << "'" << std::endl;

    // <TESTING CODE>
    std::stringstream input_stream(input);
    lang::Parser parser(input_stream);
    std::unique_ptr<lang::Module> result = parser.ParseModule();
    if (result) {
      dumper.Dump(*result);
      std::cerr << std::endl;

      for (const auto& stmt : result->getStmts()) {
        if (stmt->isExpr()) {
          std::cerr << "Value: " << evaluator.EvalNumeric(*stmt) << std::endl;
        } else if (stmt->getKind() == lang::NODE_EXPR_STMT) {
          std::cerr << "Value: "
                    << evaluator.EvalNumeric(
                           static_cast<lang::ExprStmt*>(stmt.get())->getExpr())
                    << std::endl;
        } else if (stmt->isStmt()) {
          evaluator.EvalStmt(*static_cast<lang::Stmt*>(stmt.get()));
        }

        if (evaluator.Failed()) {
          std::cerr << "Failed evaluation" << std::endl;
          evaluator.ResetFail();
          break;
        }
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
