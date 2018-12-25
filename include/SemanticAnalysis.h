#ifndef SEMANTICANALYSIS_H
#define SEMANTICANALYSIS_H

#include <unordered_map>

#include "Nodes.h"

namespace lang {

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

  // Check that a function has a valid return stmt across every branch.
  bool FunctionHasValidReturn(const Function &func) const;

 private:
  bool BranchHasValidReturn(
      const std::vector<std::unique_ptr<Stmt>> &stmts) const;

  std::vector<std::unordered_map<std::string, std::unique_ptr<Expr>>>
      id_tables_;
  std::vector<std::unordered_map<std::string, std::unique_ptr<Function>>>
      func_tables_;
  NodeCloner cloner_;
};

/**
 * RAII object for maintaining scope entering and exiting.
 */
class EnterScopeRAII {
 public:
  EnterScopeRAII(SemanticAnalyzer &sema) : sema_(sema) { sema.EnterScope(); }

  ~EnterScopeRAII() { sema_.ExitScope(); }

 private:
  SemanticAnalyzer &sema_;
};

}  // namespace lang

#endif
