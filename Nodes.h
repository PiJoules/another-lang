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
  Module(std::vector<std::unique_ptr<Stmt>> stmts)
    : stmts_(std::move(stmts)) {}

  NodeKind getKind() const override {
    return NODE_MODULE;
  }
  bool isExpr() const override { return false; }
  bool isStmt() const override { return false; }

  const std::vector<std::unique_ptr<Stmt>> &getStmts() const {
    return stmts_;
  }

 private:
  std::vector<std::unique_ptr<Stmt>> stmts_;
};

class Expr : public Node {
 public:
  virtual bool isAssignable() const = 0;
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
  bool isAssignable() const override { return false; }

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
  bool isAssignable() const override { return false; }
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

template <class RetTy = void>
class AbstractConstASTVisitor {
 public:
  virtual ~AbstractConstASTVisitor() {}

#define NODE(NAME, KIND) \
  virtual RetTy Visit(const NAME &node) = 0;
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
#define NODE(NAME, KIND) \
  void Visit(const NAME &node) override;
#include "Nodes.def"

  void AddSpacing();

  std::ostream &out_;
  unsigned indent_level_ = 0;
  std::string indent_;
};

std::string NodeToString(const Node &node);

struct EvalFailure {
  enum Reason { UNKNOWN_ID } reason;

  EvalFailure() = default;
  EvalFailure(Reason reason, const SourceLocation &failing_loc)
      : reason(reason), failing_loc(failing_loc) {}

  void Dump(std::ostream &out) const;

  SourceLocation failing_loc;
};

/**
 * This class makes copies of every node.
 */
class NodeCloner {
 public:
  std::unique_ptr<Stmt> Clone(const Stmt &stmt) const;
  std::unique_ptr<Expr> Clone(const Expr &expr) const;

 private:
  std::unique_ptr<IntLiteral> Visit(const IntLiteral &expr) const;
  std::unique_ptr<BinOperator> Visit(const BinOperator &expr) const;
  std::unique_ptr<ParenExpr> Visit(const ParenExpr &expr) const;
  std::unique_ptr<IDExpr> Visit(const IDExpr &expr) const;
  std::unique_ptr<AssignableExpr> CloneAssignableExpr(
      const AssignableExpr &expr) const;

  std::unique_ptr<ExprStmt> Visit(const ExprStmt &stmt) const;
  std::unique_ptr<Assign> Visit(const Assign &stmt) const;
};

/**
 * This class performs any semantic analysis and AST transformations on ASTs.
 */
class SemanticAnalyzer {
 public:
  void setID(const std::string &id, const Expr &expr) {
    id_table_[id] = cloner.Clone(expr);
  }

  const Expr *getID(const std::string &id) const {
    auto found_expr = id_table_.find(id);
    if (found_expr == id_table_.end()) return nullptr;
    return found_expr->second.get();
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<Expr>> id_table_;
  NodeCloner cloner;
};

class ASTEval {
 public:
  int64_t EvalNumeric(const Expr &expr);
  int64_t EvalNumeric(const Node &node);
  void EvalStmt(const Stmt &stmt);

  bool Failed() const { return failed_; }
  void ResetFail() { failed_ = false; }

 private:
  int64_t DispatchExpr(const Expr &expr);
  int64_t Visit(const IntLiteral &expr);
  int64_t Visit(const BinOperator &expr);
  int64_t Visit(const ParenExpr &expr);
  int64_t Visit(const IDExpr &expr);

  void DispatchStmt(const Stmt &stmt);
  void Visit(const Assign &stmt);
  void Visit(const ExprStmt &stmt);

  SemanticAnalyzer sema_;
  bool failed_ = false;
  EvalFailure failure_;
};

}  // namespace lang

#endif
