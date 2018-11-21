#ifndef NODES_H
#define NODES_H

#include <memory>
#include <ostream>

namespace lang {

enum NodeKind { NODE_INT, NODE_BINOP, NODE_PAREN };

class Node {
 public:
  virtual ~Node() {}
  virtual NodeKind getKind() const = 0;
};

class IntLiteral : public Node {
 public:
  IntLiteral(int64_t val) : val_(val) {}

  int64_t getVal() const { return val_; }
  NodeKind getKind() const override { return NODE_INT; }

 private:
  int64_t val_ = 0;
};

enum BinOperatorKind { BIN_ADD, BIN_SUB, BIN_MUL, BIN_DIV };

std::string BinOpKindToString(BinOperatorKind op);

class BinOperator : public Node {
 public:
  BinOperator(std::unique_ptr<Node> lhs, std::unique_ptr<Node> rhs,
              BinOperatorKind op)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)), op_(op) {}

  BinOperatorKind getOp() const { return op_; }
  NodeKind getKind() const override { return NODE_BINOP; }
  const Node &getLHS() const { return *lhs_; }
  const Node &getRHS() const { return *rhs_; }

 private:
  std::unique_ptr<Node> lhs_, rhs_;
  BinOperatorKind op_;
};

class ParenExpr : public Node {
 public:
  ParenExpr(std::unique_ptr<Node> inner) : inner_(std::move(inner)) {}

  const Node &getInner() const { return *inner_; }
  NodeKind getKind() const override { return NODE_PAREN; }

 private:
  std::unique_ptr<Node> inner_;
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

}  // namespace lang

#endif
