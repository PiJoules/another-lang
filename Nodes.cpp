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

template <class RetTy>
RetTy ConstASTVisitor<RetTy>::Dispatch(const Node &node) {
  switch (node.getKind()) {
    case NODE_INT:
      return Visit(static_cast<const IntLiteral &>(node));
    case NODE_BINOP:
      return Visit(static_cast<const BinOperator &>(node));
    case NODE_PAREN:
      return Visit(static_cast<const ParenExpr &>(node));
  }
}

void ASTDump::AddSpacing() {
  for (unsigned i = 0; i < indent_level_; ++i) out_ << indent_;
}

void ASTDump::Visit(const IntLiteral &node) {
  out_ << "<IntLiteral val=" << node.getVal() << ">";
}

void ASTDump::Visit(const BinOperator &node) {
  out_ << "<BinOperator op='" << BinOpKindToString(node.getOp()) << "'\n";
  indent_level_++;

  AddSpacing();
  out_ << "lhs=";
  Visit(node.getLHS());
  out_ << "\n";

  AddSpacing();
  out_ << "rhs=";
  Visit(node.getRHS());
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
  Visit(node.getInner());
  out_ << "\n";

  indent_level_--;
  AddSpacing();
  out_ << ">";
}

std::string NodeToString(const Node &node) {
  std::stringstream sstream;
  ASTDump dumper(sstream);
  dumper.Visit(node);
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

int64_t ASTEval::Visit(const Expr &expr) { return Dispatch(expr); }

int64_t ASTEval::Visit(const Node &node) {
  assert(NodeIsExpr(node) && "Can only evaluate nodes that are expressions.");
  return Visit(static_cast<const Expr &>(node));
}

int64_t ASTEval::Visit(const IntLiteral &expr) { return expr.getVal(); }

int64_t ASTEval::Visit(const BinOperator &expr) {
  int64_t lhs_val = Visit(expr.getLHS());
  int64_t rhs_val = Visit(expr.getRHS());
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

int64_t ASTEval::Visit(const ParenExpr &expr) { return Visit(expr.getInner()); }

}  // namespace lang
