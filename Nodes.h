#ifndef NODES_H
#define NODES_H

#include <memory>
#include <ostream>

#include "LangCommon.h"

namespace lang {

enum NodeKind { NODE_INT, NODE_BINOP, NODE_PAREN };

class Node {
 public:
  virtual ~Node() {}
  virtual NodeKind getKind() const = 0;
};

class Expr : public Node {};

bool NodeIsExpr(NodeKind kind);
bool NodeIsExpr(const Node &node);

class IntLiteral : public Expr {
 public:
  IntLiteral(int64_t val) : val_(val) {}

  int64_t getVal() const { return val_; }
  NodeKind getKind() const override { return NODE_INT; }

 private:
  int64_t val_ = 0;
};

enum BinOperatorKind { BIN_ADD, BIN_SUB, BIN_MUL, BIN_DIV };

std::string BinOpKindToString(BinOperatorKind op);

class BinOperator : public Expr {
 public:
  BinOperator(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs,
              BinOperatorKind op)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)), op_(op) {}

  BinOperatorKind getOp() const { return op_; }
  NodeKind getKind() const override { return NODE_BINOP; }
  const Expr &getLHS() const { return *lhs_; }
  const Expr &getRHS() const { return *rhs_; }

 private:
  std::unique_ptr<Expr> lhs_, rhs_;
  BinOperatorKind op_;
};

class ParenExpr : public Expr {
 public:
  ParenExpr(std::unique_ptr<Expr> inner) : inner_(std::move(inner)) {}

  const Expr &getInner() const { return *inner_; }
  NodeKind getKind() const override { return NODE_PAREN; }

 private:
  std::unique_ptr<Expr> inner_;
};

template <class RetTy = void>
class ConstASTVisitor {
 public:
  virtual ~ConstASTVisitor() {}

  virtual RetTy Visit(const IntLiteral &node) { return RetTy(); }
  virtual RetTy Visit(const BinOperator &node) { return RetTy(); }
  virtual RetTy Visit(const ParenExpr &node) { return RetTy(); }

 protected:
  RetTy Dispatch(const Node &node);
};

class ASTDump : public ConstASTVisitor<void> {
 public:
  ASTDump(std::ostream &out) : ASTDump(out, /*indent_size=*/2) {}
  ASTDump(std::ostream &out, unsigned indent_size)
      : out_(out), indent_(indent_size, ' ') {}
  void Visit(const Node &node) { Dispatch(node); }

 private:
  void Visit(const IntLiteral &node) override;
  void Visit(const BinOperator &node) override;
  void Visit(const ParenExpr &node) override;
  void AddSpacing();

  std::ostream &out_;
  unsigned indent_level_ = 0;
  std::string indent_;
};

std::string NodeToString(const Node &node);

class ASTEval : public ConstASTVisitor<int64_t> {
 public:
  int64_t Visit(const Expr &expr);
  int64_t Visit(const Node &node);

 private:
  int64_t Visit(const IntLiteral &expr) override;
  int64_t Visit(const BinOperator &expr) override;
  int64_t Visit(const ParenExpr &expr) override;
};

}  // namespace lang

#endif
