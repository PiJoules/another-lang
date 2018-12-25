#ifndef ASTEVAL_H
#define ASTEVAL_H

#include "Nodes.h"
#include "SemanticAnalysis.h"

namespace lang {

class ASTEval {
 public:
  int64_t EvalNumeric(const Expr &expr);
  int64_t EvalNumeric(const Node &node);
  void EvalStmt(const Stmt &stmt);

  bool Failed() const { return failed_; }
  void ResetFail() { failed_ = false; }

 private:
#define NODE(NAME, KIND)
#define EXPR(NAME, KIND) int64_t Visit(const NAME &expr);
#include "Nodes.def"

#define NODE(NAME, KIND)
#define STMT(NAME, KIND) void Visit(const NAME &stmt);
#include "Nodes.def"

  int64_t EvalFuncBody(const std::vector<std::unique_ptr<Stmt>> &body);
  int64_t EvalReturnStmt(const Return &ret);

  // Evaluate statements that may contain a return stmt. If it contains an
  // evaluatable result, set the result and return true.
  bool EvalStmtsInNewScope(const std::vector<std::unique_ptr<Stmt>> &stmts,
                           int64_t &result);

  // Evalate an if stmt that may contain a return stmt. If it contains a return,
  // set the result and return true.
  bool EvalIfStmt(const If &if_stmt, int64_t &result);

  SemanticAnalyzer sema_;
  bool failed_ = false;
};

}  // namespace lang

#endif
