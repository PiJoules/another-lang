#include <sstream>

#include "Nodes.h"

namespace lang {

std::string BinOpKindToString(BinOperatorKind op) {
  switch (op) {
    case BIN_ADD:
      return "+";
    case BIN_SUB:
      return "-";
    case BIN_MUL:
      return "*";
    case BIN_DIV:
      return "/";
  }
  LANG_UNREACHABLE("found an unhandled BinaryOperatorKind");
}

template <class RetTy>
RetTy AbstractConstASTVisitor<RetTy>::Dispatch(const Node &node) {
  switch (node.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    return Visit(static_cast<const NAME &>(node));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("found an unhandled NodeKind");
}

void ASTDump::AddSpacing() {
  for (unsigned i = 0; i < indent_level_; ++i) out_ << indent_;
}

void ASTDump::Visit(const Module &node) {
  out_ << "<Module stmts=\n";
  indent_level_++;

  for (const std::unique_ptr<Stmt> &stmt : node.getStmts()) {
    AddSpacing();
    Dump(*stmt);
    out_ << "\n";
  }

  indent_level_--;
  AddSpacing();
  out_ << ">";
}

void ASTDump::Visit(const IntLiteral &node) {
  out_ << "<IntLiteral val=" << node.getVal() << ">";
}

void ASTDump::Visit(const BinOperator &node) {
  out_ << "<BinOperator op='" << BinOpKindToString(node.getOp()) << "'\n";
  indent_level_++;

  AddSpacing();
  out_ << "lhs=";
  Dump(node.getLHS());
  out_ << "\n";

  AddSpacing();
  out_ << "rhs=";
  Dump(node.getRHS());
  out_ << "\n";

  indent_level_--;
  AddSpacing();
  out_ << ">";
}

void ASTDump::Visit(const ParenExpr &node) {
  out_ << "<ParenExpr\n";
  indent_level_++;

  AddSpacing();
  out_ << "inner=";
  Dump(node.getInner());
  out_ << "\n";

  indent_level_--;
  AddSpacing();
  out_ << ">";
}

void ASTDump::Visit(const IDExpr &node) {
  out_ << "<IDExpr"
       << " name='" << node.getName() << "'>";
}

void ASTDump::Visit(const ExprStmt &node) {
  out_ << "<ExprStmt\n";
  indent_level_++;

  AddSpacing();
  out_ << "expr=";
  Dump(node.getExpr());
  out_ << "\n";

  indent_level_--;
  AddSpacing();
  out_ << ">";
}

void ASTDump::Visit(const Assign &node) {
  out_ << "<Assign\n";
  indent_level_++;

  AddSpacing();
  out_ << "lhs=";
  Dump(node.getLHS());
  out_ << "\n";

  AddSpacing();
  out_ << "rhs=";
  Dump(node.getRHS());
  out_ << "\n";

  indent_level_--;
  AddSpacing();
  out_ << ">";
}

std::string NodeToString(const Node &node) {
  std::stringstream sstream;
  ASTDump dumper(sstream);
  dumper.Dump(node);
  return sstream.str();
}

std::unique_ptr<Stmt> NodeCloner::Clone(const Stmt &stmt) const {
  switch (stmt.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    break;
#define STMT(NAME, KIND) \
  case KIND:             \
    return Visit(static_cast<const NAME &>(stmt));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("Expected a statement");
}

std::unique_ptr<Expr> NodeCloner::Clone(const Expr &expr) const {
  switch (expr.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    break;
#define EXPR(NAME, KIND) \
  case KIND:             \
    return Visit(static_cast<const NAME &>(expr));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("Expected an expression");
}

std::unique_ptr<AssignableExpr> NodeCloner::CloneAssignableExpr(
    const AssignableExpr &expr) const {
  std::unique_ptr<Expr> cloned_expr = Clone(expr);
  return std::unique_ptr<AssignableExpr>(
      static_cast<AssignableExpr *>(cloned_expr.release()));
}

std::unique_ptr<IntLiteral> NodeCloner::Visit(const IntLiteral &expr) const {
  return std::make_unique<IntLiteral>(expr.getVal());
}

std::unique_ptr<BinOperator> NodeCloner::Visit(const BinOperator &expr) const {
  return std::make_unique<BinOperator>(Clone(expr.getLHS()),
                                       Clone(expr.getRHS()), expr.getOp());
}

std::unique_ptr<ParenExpr> NodeCloner::Visit(const ParenExpr &expr) const {
  return std::make_unique<ParenExpr>(Clone(expr.getInner()));
}

std::unique_ptr<IDExpr> NodeCloner::Visit(const IDExpr &expr) const {
  return std::make_unique<IDExpr>(expr.getName());
}

std::unique_ptr<ExprStmt> NodeCloner::Visit(const ExprStmt &stmt) const {
  return std::make_unique<ExprStmt>(Clone(stmt.getExpr()));
}

std::unique_ptr<Assign> NodeCloner::Visit(const Assign &stmt) const {
  return std::make_unique<Assign>(CloneAssignableExpr(stmt.getLHS()),
                                  Clone(stmt.getRHS()));
}

int64_t ASTEval::EvalNumeric(const Expr &expr) { return DispatchExpr(expr); }

int64_t ASTEval::EvalNumeric(const Node &node) {
  assert(node.isExpr() && "Can only evaluate nodes that are expressions.");
  return EvalNumeric(static_cast<const Expr &>(node));
}

int64_t ASTEval::DispatchExpr(const Expr &expr) {
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
  failed_ = true;
  return -1;
}

void ASTEval::EvalStmt(const Stmt &stmt) { DispatchStmt(stmt); }

void ASTEval::DispatchStmt(const Stmt &stmt) {
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

void ASTEval::Visit(const ExprStmt &stmt) { /*Nothing to evaluate*/
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

}  // namespace lang
