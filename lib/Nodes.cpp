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

void ASTDump::Visit(const Function &node) {
  out_ << "<Function name='" << node.getName() << "'\n";
  indent_level_++;

  AddSpacing();
  out_ << "args=\n";
  indent_level_++;

  for (const std::unique_ptr<IDExpr> &arg : node.getArgs()) {
    AddSpacing();
    Dump(*arg);
    out_ << "\n";
  }

  indent_level_--;
  AddSpacing();
  out_ << "body=\n";

  for (const std::unique_ptr<Stmt> &stmt : node.getBody()) {
    AddSpacing();
    Dump(*stmt);
    out_ << "\n";
  }

  indent_level_--;
  AddSpacing();
  out_ << ">";
}

void ASTDump::Visit(const If &node) {
  out_ << "<If\n";
  indent_level_++;

  AddSpacing();
  out_ << "cond=";
  Dump(node.getCond());
  out_ << "\n";

  AddSpacing();
  out_ << "body=\n";
  indent_level_++;

  for (const std::unique_ptr<Stmt> &stmt : node.getBody()) {
    AddSpacing();
    Dump(*stmt);
    out_ << "\n";
  }

  indent_level_--;
  AddSpacing();
  out_ << "else=\n";

  for (const std::unique_ptr<Stmt> &stmt : node.getElseStmts()) {
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

void ASTDump::Visit(const Call &node) {
  out_ << "<Call\n";
  indent_level_++;

  AddSpacing();
  out_ << "func=";
  Dump(node.getFunc());
  out_ << "\n";

  AddSpacing();
  out_ << "args=\n";
  indent_level_++;

  for (const std::unique_ptr<Expr> &arg : node.getArgs()) {
    AddSpacing();
    Dump(*arg);
    out_ << "\n";
  }

  indent_level_ -= 2;
  AddSpacing();
  out_ << ">";
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

void ASTDump::Visit(const Return &node) {
  out_ << "<Return\n";
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

std::unique_ptr<Function> NodeCloner::Clone(const Function &func) const {
  return VisitFunction(func);
}

std::unique_ptr<Node> NodeCloner::Clone(const Node &node) const {
  switch (node.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    return Visit##NAME(static_cast<const NAME &>(node));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("Should've returned before this");
}

std::unique_ptr<Expr> NodeCloner::Clone(const Expr &expr) const {
  switch (expr.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    break;
#define EXPR(NAME, KIND) \
  case KIND:             \
    return Visit##NAME(static_cast<const NAME &>(expr));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("Expected an expression");
}

std::unique_ptr<Stmt> NodeCloner::Clone(const Stmt &stmt) const {
  switch (stmt.getKind()) {
#define NODE(NAME, KIND) \
  case KIND:             \
    break;
#define STMT(NAME, KIND) \
  case KIND:             \
    return Visit##NAME(static_cast<const NAME &>(stmt));
#include "Nodes.def"
  }
  LANG_UNREACHABLE("Expected a statement");
}

std::unique_ptr<IntLiteral> NodeCloner::VisitIntLiteral(
    const IntLiteral &expr) const {
  return std::make_unique<IntLiteral>(expr.getVal());
}

std::unique_ptr<BinOperator> NodeCloner::VisitBinOperator(
    const BinOperator &expr) const {
  return std::make_unique<BinOperator>(Clone(expr.getLHS()),
                                       Clone(expr.getRHS()), expr.getOp());
}

std::unique_ptr<ParenExpr> NodeCloner::VisitParenExpr(
    const ParenExpr &expr) const {
  return std::make_unique<ParenExpr>(Clone(expr.getInner()));
}

std::unique_ptr<IDExpr> NodeCloner::VisitIDExpr(const IDExpr &expr) const {
  return std::make_unique<IDExpr>(expr.getName());
}

std::unique_ptr<ExprStmt> NodeCloner::VisitExprStmt(
    const ExprStmt &stmt) const {
  return std::make_unique<ExprStmt>(Clone(stmt.getExpr()));
}

std::unique_ptr<Function> NodeCloner::VisitFunction(
    const Function &func) const {
  std::vector<std::unique_ptr<IDExpr>> args;
  std::vector<std::unique_ptr<Stmt>> body;
  for (const auto &arg : func.getArgs()) args.push_back(VisitIDExpr(*arg));
  for (const auto &stmt : func.getBody()) body.push_back(Clone(*stmt));
  return std::make_unique<Function>(func.getName(), std::move(args),
                                    std::move(body));
}

std::unique_ptr<If> NodeCloner::VisitIf(const If &if_stmt) const {
  std::vector<std::unique_ptr<Stmt>> body;
  std::vector<std::unique_ptr<Stmt>> else_stmts;
  for (const auto &stmt : if_stmt.getBody()) body.push_back(Clone(*stmt));
  for (const auto &stmt : if_stmt.getElseStmts())
    else_stmts.push_back(Clone(*stmt));
  return std::make_unique<If>(Clone(if_stmt.getCond()), std::move(body),
                              std::move(else_stmts));
}

std::unique_ptr<Assign> NodeCloner::VisitAssign(const Assign &stmt) const {
  return std::make_unique<Assign>(CloneAs<AssignableExpr>(stmt.getLHS()),
                                  Clone(stmt.getRHS()));
}

std::unique_ptr<Module> NodeCloner::VisitModule(const Module &module) const {
  std::vector<std::unique_ptr<Stmt>> stmts;
  for (const auto &stmt : module.getStmts()) stmts.push_back(Clone(*stmt));
  return std::make_unique<Module>(std::move(stmts));
}

std::unique_ptr<Return> NodeCloner::VisitReturn(const Return &ret) const {
  return std::make_unique<Return>(Clone(ret.getExpr()));
}

std::unique_ptr<Call> NodeCloner::VisitCall(const Call &call) const {
  std::vector<std::unique_ptr<Expr>> args;
  for (const auto &arg : call.getArgs()) args.push_back(Clone(*arg));
  return std::make_unique<Call>(Clone(call.getFunc()), std::move(args));
}

}  // namespace lang
