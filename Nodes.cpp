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

}  // namespace lang
