#ifndef NODES_H
#define NODES_H

#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "LangCommon.h"

namespace lang {

enum NodeKind {
#define NODE(NAME, KIND) KIND,
#include "Nodes.def"
};

class Node {
 public:
  virtual ~Node() {}
  virtual NodeKind getKind() const = 0;
  virtual bool isExpr() const = 0;
  virtual bool isStmt() const = 0;
};

/**
 * Abstract nodes.
 */
class Stmt : public Node {
 public:
  bool isExpr() const override { return false; }
  bool isStmt() const override { return true; }
};

class Module : public Node {
 public:
  Module(std::vector<std::unique_ptr<Stmt>> stmts) : stmts_(std::move(stmts)) {}

  NodeKind getKind() const override { return NODE_MODULE; }
  bool isExpr() const override { return false; }
  bool isStmt() const override { return false; }

  const std::vector<std::unique_ptr<Stmt>> &getStmts() const { return stmts_; }

 private:
  std::vector<std::unique_ptr<Stmt>> stmts_;
};

class Expr : public Node {
 public:
  virtual bool isAssignable() const { return false; }
  bool isExpr() const override { return true; }
  bool isStmt() const override { return false; }
};
class AssignableExpr : public Expr {
 public:
  bool isAssignable() const override { return true; }
};

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
  bool isAssignable() const override { return getInner().isAssignable(); }

 private:
  std::unique_ptr<Expr> inner_;
};

class IDExpr : public AssignableExpr {
 public:
  IDExpr(const std::string &id) : id_(id) {}

  std::string getName() const { return id_; }
  NodeKind getKind() const override { return NODE_ID; }

 private:
  std::string id_;
};

class Assign : public Stmt {
 public:
  Assign(std::unique_ptr<AssignableExpr> lhs, std::unique_ptr<Expr> rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  NodeKind getKind() const { return NODE_ASSIGN; }
  const AssignableExpr &getLHS() const { return *lhs_; }
  const Expr &getRHS() const { return *rhs_; }

 private:
  std::unique_ptr<AssignableExpr> lhs_;
  std::unique_ptr<Expr> rhs_;
};

class ExprStmt : public Stmt {
 public:
  ExprStmt(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

  NodeKind getKind() const { return NODE_EXPR_STMT; }
  const Expr &getExpr() const { return *expr_; }

 private:
  std::unique_ptr<Expr> expr_;
};

class If : public Stmt {
 public:
  If(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> body,
     std::vector<std::unique_ptr<Stmt>> else_stmts)
      : cond_(std::move(cond)),
        body_(std::move(body)),
        else_stmts_(std::move(else_stmts)) {}
  If(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> body)
      : cond_(std::move(cond)), body_(std::move(body)) {}

  NodeKind getKind() const override { return NODE_IF; }
  const Expr &getCond() const { return *cond_; }
  const std::vector<std::unique_ptr<Stmt>> &getBody() const { return body_; }
  const std::vector<std::unique_ptr<Stmt>> &getElseStmts() const {
    return else_stmts_;
  }

 private:
  std::unique_ptr<Expr> cond_;
  std::vector<std::unique_ptr<Stmt>> body_;
  std::vector<std::unique_ptr<Stmt>> else_stmts_;
};

class Function : public Stmt {
 public:
  Function(const std::string &name, std::vector<std::unique_ptr<IDExpr>> args,
           std::vector<std::unique_ptr<Stmt>> body)
      : name_(name), args_(std::move(args)), body_(std::move(body)) {}
  Function(const std::string &name) : name_(name) {}

  NodeKind getKind() const override { return NODE_FUNCTION; }

  std::string getName() const { return name_; }
  const std::vector<std::unique_ptr<IDExpr>> &getArgs() const { return args_; }
  const std::vector<std::unique_ptr<Stmt>> &getBody() const { return body_; }

 private:
  std::string name_;
  std::vector<std::unique_ptr<IDExpr>> args_;
  std::vector<std::unique_ptr<Stmt>> body_;
};

class Return : public Stmt {
 public:
  Return(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

  NodeKind getKind() const { return NODE_RETURN; }
  const Expr &getExpr() const { return *expr_; }

 private:
  std::unique_ptr<Expr> expr_;
};

class Call : public Expr {
 public:
  Call(std::unique_ptr<Expr> func, std::vector<std::unique_ptr<Expr>> args)
      : func_(std::move(func)), args_(std::move(args)) {}
  Call(std::unique_ptr<Expr> func) : func_(std::move(func)) {}

  NodeKind getKind() const override { return NODE_CALL; }
  const Expr &getFunc() const { return *func_; }
  const std::vector<std::unique_ptr<Expr>> &getArgs() const { return args_; }

 private:
  std::unique_ptr<Expr> func_;
  std::vector<std::unique_ptr<Expr>> args_;
};

template <class RetTy = void>
class AbstractConstASTVisitor {
 public:
  virtual ~AbstractConstASTVisitor() {}

#define NODE(NAME, KIND) virtual RetTy Visit(const NAME &node) = 0;
#include "Nodes.def"

 protected:
  RetTy Dispatch(const Node &node);
};

template <class RetTy = void>
class ConstASTVisitor : public AbstractConstASTVisitor<RetTy> {
 public:
#define NODE(NAME, KIND) \
  virtual RetTy Visit(const NAME &node) override { return RetTy(); }
#include "Nodes.def"
};

class ASTDump : public AbstractConstASTVisitor<void> {
 public:
  ASTDump(std::ostream &out) : ASTDump(out, /*indent_size=*/2) {}
  ASTDump(std::ostream &out, unsigned indent_size)
      : out_(out), indent_(indent_size, ' ') {}
  void Dump(const Node &node) { Dispatch(node); }

 private:
#define NODE(NAME, KIND) void Visit(const NAME &node) override;
#include "Nodes.def"

  void AddSpacing();

  std::ostream &out_;
  unsigned indent_level_ = 0;
  std::string indent_;
};

std::string NodeToString(const Node &node);

/**
 * This class makes copies of every node.
 */
class NodeCloner {
 public:
  std::unique_ptr<Node> Clone(const Node &node) const;
  std::unique_ptr<Stmt> Clone(const Stmt &stmt) const;
  std::unique_ptr<Expr> Clone(const Expr &expr) const;
  std::unique_ptr<Function> Clone(const Function &func) const;

  template <class NodeType>
  std::unique_ptr<NodeType> CloneAs(const Node &node) const {
    std::unique_ptr<Node> cloned = Clone(node);
    return std::unique_ptr<NodeType>(static_cast<NodeType *>(cloned.release()));
  }

 private:
#define NODE(NAME, KIND) \
  std::unique_ptr<NAME> Visit##NAME(const NAME &node) const;
#include "Nodes.def"
};

template <class NodeType>
std::unique_ptr<NodeType> CloneNode(const Node &node) {
  return NodeCloner().CloneAs<NodeType>(node);
}

template <class NodeType>
const NodeType &GetNodeAs(const Node &node) {
  return *static_cast<const NodeType *>(&node);
}

}  // namespace lang

#endif
