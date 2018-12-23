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

/**
 * This class performs any semantic analysis and AST transformations on ASTs.
 */
class SemanticAnalyzer {
 public:
  SemanticAnalyzer() {
    id_tables_.push_back({});
    func_tables_.push_back({});
  }

  void setID(const std::string &id, const Expr &expr) {
    id_tables_.back()[id] = cloner_.Clone(expr);
  }

  const Expr *getID(const std::string &id) const {
    for (auto it = id_tables_.rbegin(); it != id_tables_.rend(); ++it) {
      auto &id_table_ = *it;
      auto found_expr = id_table_.find(id);
      if (found_expr != id_table_.end()) return found_expr->second.get();
    }
    return nullptr;
  }

  void setFunc(const std::string &name, const Function &func) {
    func_tables_.back()[name] = cloner_.Clone(func);
  }

  const Function *getFunc(const std::string &name) const {
    for (auto it = func_tables_.rbegin(); it != func_tables_.rend(); ++it) {
      auto &func_table_ = *it;
      auto found_func = func_table_.find(name);
      if (found_func != func_table_.end()) return found_func->second.get();
    }
    return nullptr;
  }

  void EnterScope() {
    id_tables_.push_back({});
    func_tables_.push_back({});
  }

  void ExitScope() {
    id_tables_.pop_back();
    func_tables_.pop_back();
  }

 private:
  std::vector<std::unordered_map<std::string, std::unique_ptr<Expr>>>
      id_tables_;
  std::vector<std::unordered_map<std::string, std::unique_ptr<Function>>>
      func_tables_;
  NodeCloner cloner_;
};

class ASTEval {
 public:
  int64_t EvalNumeric(const Expr &expr);
  int64_t EvalNumeric(const Node &node);
  void EvalStmt(const Stmt &stmt);

  bool Failed() const { return failed_; }
  void ResetFail() { failed_ = false; }

 private:
#define NODE(NAME, KIND)
#define EXPR(NAME, KIND) int64_t Visit(const NAME &expr);
#include "Nodes.def"

#define NODE(NAME, KIND)
#define STMT(NAME, KIND) void Visit(const NAME &stmt);
#include "Nodes.def"

  int64_t EvalFuncBody(const std::vector<std::unique_ptr<Stmt>> &body);
  int64_t EvalReturnStmt(const Return &ret);

  SemanticAnalyzer sema_;
  bool failed_ = false;
};

}  // namespace lang

#endif
