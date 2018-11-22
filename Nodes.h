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

class ASTDump {
 public:
  ASTDump(std::ostream &out) : ASTDump(out, /*indent_size=*/2) {}
  ASTDump(std::ostream &out, unsigned indent_size)
      : out_(out), indent_(indent_size, ' ') {}
  void Dump(const Node &node);

 private:
  void Dump(const IntLiteral &node);
  void Dump(const BinOperator &node);
  void Dump(const ParenExpr &node);
  void Dispatch(const Node &node);
  void AddSpacing();

  std::ostream &out_;
  unsigned indent_level_ = 0;
  std::string indent_;
};

std::string NodeToString(const Node &node);

class ASTEval {
 public:
  int64_t EvalNumericExpr(const Expr &expr);
  int64_t EvalNumericExpr(const Node &node);

 private:
  int64_t Eval(const IntLiteral &expr);
  int64_t Eval(const BinOperator &expr);
  int64_t Eval(const ParenExpr &expr);
  int64_t Dispatch(const Expr &expr);
};

}  // namespace lang

#endif
