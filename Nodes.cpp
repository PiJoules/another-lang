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
}

void ASTDump::Dump(const Node &node) { Dispatch(node); }

void ASTDump::Dispatch(const Node &node) {
  switch (node.getKind()) {
    case NODE_INT:
      return Dump(static_cast<const IntLiteral &>(node));
    case NODE_BINOP:
      return Dump(static_cast<const BinOperator &>(node));
    case NODE_PAREN:
      return Dump(static_cast<const ParenExpr &>(node));
  }
}

void ASTDump::AddSpacing() {
  for (unsigned i = 0; i < indent_level_; ++i) out_ << indent_;
}

void ASTDump::Dump(const IntLiteral &node) {
  out_ << "<IntLiteral val=" << node.getVal() << ">";
}

void ASTDump::Dump(const BinOperator &node) {
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

void ASTDump::Dump(const ParenExpr &node) {
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

std::string NodeToString(const Node &node) {
  std::stringstream sstream;
  ASTDump dumper(sstream);
  dumper.Dump(node);
  return sstream.str();
}

bool NodeIsExpr(NodeKind kind) {
  switch (kind) {
    case NODE_INT:
    case NODE_BINOP:
    case NODE_PAREN:
      return true;
  }
  LANG_UNREACHABLE("should have covered all cases in the switch stmt above");
}

bool NodeIsExpr(const Node &node) { return NodeIsExpr(node.getKind()); }

int64_t ASTEval::EvalNumericExpr(const Expr &expr) { return Dispatch(expr); }

int64_t ASTEval::EvalNumericExpr(const Node &node) {
  assert(NodeIsExpr(node) && "Can only evaluate nodes that are expressions.");
  return EvalNumericExpr(static_cast<const Expr &>(node));
}

int64_t ASTEval::Dispatch(const Expr &expr) {
  switch (expr.getKind()) {
    case NODE_INT:
      return Eval(static_cast<const IntLiteral &>(expr));
    case NODE_BINOP:
      return Eval(static_cast<const BinOperator &>(expr));
    case NODE_PAREN:
      return Eval(static_cast<const ParenExpr &>(expr));
  }
}

int64_t ASTEval::Eval(const IntLiteral &expr) { return expr.getVal(); }

int64_t ASTEval::Eval(const BinOperator &expr) {
  int64_t lhs_val = EvalNumericExpr(expr.getLHS());
  int64_t rhs_val = EvalNumericExpr(expr.getRHS());
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

int64_t ASTEval::Eval(const ParenExpr &expr) {
  return EvalNumericExpr(expr.getInner());
}

}  // namespace lang
