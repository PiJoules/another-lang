#include "ASTEval.h"

namespace lang {

int64_t ASTEval::EvalNumeric(const Expr &expr) {
  switch (expr.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    break;
#define EXPR(NAME, KIND) \
  case KIND:             \
    return Visit(static_cast<const NAME &>(expr));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("Expected expression");
}

int64_t ASTEval::EvalNumeric(const Node &node) {
  assert(node.isExpr() && "Can only evaluate nodes that are expressions.");
  return EvalNumeric(static_cast<const Expr &>(node));
}

int64_t ASTEval::Visit(const IntLiteral &expr) { return expr.getVal(); }

int64_t ASTEval::Visit(const BinOperator &expr) {
  int64_t lhs_val = EvalNumeric(expr.getLHS());
  int64_t rhs_val = EvalNumeric(expr.getRHS());
  switch (expr.getOp()) {
    case BIN_ADD:
      return lhs_val + rhs_val;
    case BIN_SUB:
      return lhs_val - rhs_val;
    case BIN_MUL:
      return lhs_val * rhs_val;
    case BIN_DIV:
      return lhs_val / rhs_val;
  }
}

int64_t ASTEval::Visit(const ParenExpr &expr) {
  return EvalNumeric(expr.getInner());
}

int64_t ASTEval::Visit(const IDExpr &expr) {
  const Expr *id = sema_.getID(expr.getName());
  if (id) return EvalNumeric(*id);
  // TODO: Come up with more elegant failure reporting system
  failed_ = true;
  return -1;
}

int64_t ASTEval::EvalReturnStmt(const Return &ret) {
  return EvalNumeric(ret.getExpr());
}

void ASTEval::Visit(const Return &ret) {
  LANG_UNREACHABLE(
      "Should not be able to evaluate return statements here. These should be "
      "evaluated in EvalReturnStmt which returns the expected return type.");
}

bool ASTEval::EvalIfStmt(const If &if_stmt, int64_t &result) {
  int64_t cond = EvalNumeric(if_stmt.getCond());
  const std::vector<std::unique_ptr<Stmt>> &if_stmts =
      cond ? if_stmt.getBody() : if_stmt.getElseStmts();
  return EvalStmtsInNewScope(if_stmts, result);
}

bool ASTEval::EvalStmtsInNewScope(
    const std::vector<std::unique_ptr<Stmt>> &stmts, int64_t &result) {
  EnterScopeRAII new_scope(sema_);
  for (const auto &stmt : stmts) {
    if (stmt->getKind() == NODE_RETURN) {
      result = EvalReturnStmt(*CloneNode<Return>(*stmt));
      return true;
    } else if (stmt->getKind() == NODE_IF) {
      const If &if_stmt = GetNodeAs<If>(*stmt);
      if (EvalIfStmt(if_stmt, result)) return true;
    } else {
      EvalStmt(*stmt);
    }
  }
  return false;
}

int64_t ASTEval::EvalFuncBody(const std::vector<std::unique_ptr<Stmt>> &body) {
  int64_t result;
  if (EvalStmtsInNewScope(body, result)) return result;
  LANG_UNREACHABLE(
      "Function body should be checked for a return statement before this.");
  return -1;
}

int64_t ASTEval::Visit(const Call &expr) {
  const Expr &callee = expr.getFunc();
  if (callee.getKind() != NODE_ID) {
    // Expect only calls from IDs for now
    failed_ = true;
    return -1;
  }

  std::unique_ptr<IDExpr> id_expr = CloneNode<IDExpr>(callee);
  const Function *func = sema_.getFunc(id_expr->getName());
  if (!func) {
    failed_ = true;
    return -1;
  }

  const std::vector<std::unique_ptr<Expr>> &caller_args = expr.getArgs();
  const std::vector<std::unique_ptr<IDExpr>> &func_args = func->getArgs();

  // Apply the arguments to the function.
  if (caller_args.size() != func_args.size()) {
    failed_ = true;
    return -1;
  }

  sema_.EnterScope();
  for (unsigned i = 0; i < caller_args.size(); ++i) {
    const IDExpr &id_expr = *(func_args[i]);
    const Expr &arg_expr = *(caller_args[i]);
    sema_.setID(id_expr.getName(), arg_expr);
  }
  int64_t result = EvalFuncBody(func->getBody());
  sema_.ExitScope();

  return result;
}

void ASTEval::EvalStmt(const Stmt &stmt) {
  switch (stmt.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    break;
#define STMT(NAME, KIND) \
  case KIND:             \
    return Visit(static_cast<const NAME &>(stmt));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("Expected statement");
}

void ASTEval::Visit(const ExprStmt &stmt) {
  // Nothing to evaluate
}

void ASTEval::Visit(const Function &func) {
  if (sema_.FunctionHasValidReturn(func)) {
    sema_.setFunc(func.getName(), func);
    return;
  }
  failed_ = true;
}

void ASTEval::Visit(const Assign &stmt) {
  const AssignableExpr &lhs = stmt.getLHS();
  switch (lhs.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    break;
#define ASSIGNABLE(NAME, KIND)
#include "Nodes.def"
    case NODE_ID: {
      const IDExpr &id = static_cast<const IDExpr &>(lhs);
      sema_.setID(id.getName(), stmt.getRHS());
      return;
    }
  }
  LANG_UNREACHABLE("Expected assignable expression");
}

void ASTEval::Visit(const If &if_stmt) {
  LANG_UNREACHABLE("This should instead be handled by EvalIfStmt");
}

}  // namespace lang
