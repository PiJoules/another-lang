#include "SemanticAnalysis.h"

namespace lang {

/**
 * Check every branch that a function can lead into. A function has a valid
 * return if every branch it can diverge into has a return stmt.
 */
bool SemanticAnalyzer::FunctionHasValidReturn(const Function &func) const {
  return BranchHasValidReturn(func.getBody());
}

bool SemanticAnalyzer::BranchHasValidReturn(
    const std::vector<std::unique_ptr<Stmt>> &stmts) const {
  for (const std::unique_ptr<Stmt> &stmt : stmts) {
    if (stmt->getKind() == NODE_RETURN) {
      return true;
    } else if (stmt->getKind() == NODE_IF) {
      const If &if_stmt = GetNodeAs<If>(*stmt);
      if (BranchHasValidReturn(if_stmt.getBody()) &&
          BranchHasValidReturn(if_stmt.getElseStmts()))
        return true;
    }
  }
  return false;
}

}  // namespace lang
