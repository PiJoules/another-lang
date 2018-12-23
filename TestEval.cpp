#include <gtest/gtest.h>

#include "Parser.h"

using namespace lang;

namespace {

class EvalTest : public ::testing::Test {
 protected:
  std::stringstream input_;
};

TEST_F(EvalTest, OrderOfOperations) {
  input_ << "1 + 2 * 3";
  Parser parser(input_);
  auto result = parser.ParseExpr();
  ASSERT_NE(result, nullptr);
  auto *expr = static_cast<Expr *>(result.get());

  ASTEval evaluator;
  ASSERT_EQ(evaluator.EvalNumeric(*expr), 7);
}

TEST_F(EvalTest, Paren) {
  input_ << "2 * (1 + 2) * 3";
  Parser parser(input_);
  auto result = parser.ParseExpr();
  ASSERT_NE(result, nullptr);
  auto *expr = static_cast<Expr *>(result.get());

  ASTEval evaluator;
  ASSERT_EQ(evaluator.EvalNumeric(*expr), 18);
}

TEST_F(EvalTest, Assign) {
  input_ << "b = 2;";
  Parser parser(input_);
  auto stmt = parser.ParseStmt();
  ASSERT_NE(stmt, nullptr);

  ASTEval evaluator;
  evaluator.EvalStmt(*stmt);
  ASSERT_FALSE(evaluator.Failed());

  input_ << "b + 3;";
  auto expr = parser.ParseExpr();
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(evaluator.EvalNumeric(*expr), 5);
  ASSERT_FALSE(evaluator.Failed());
}

TEST_F(EvalTest, Function) {
  // def func(arg):
  //   return arg + 2
  // func(3)
  auto arg = std::make_unique<IDExpr>("arg");
  auto arg2 = CloneNode<IDExpr>(*arg);
  std::vector<std::unique_ptr<IDExpr>> args;
  args.push_back(std::move(arg));

  auto i = std::make_unique<IntLiteral>(2);
  auto binop =
      std::make_unique<BinOperator>(std::move(arg2), std::move(i), BIN_ADD);
  auto ret = std::make_unique<Return>(std::move(binop));
  std::vector<std::unique_ptr<Stmt>> body;
  body.push_back(std::move(ret));

  auto func =
      std::make_unique<Function>("func", std::move(args), std::move(body));
  ASSERT_NE(func, nullptr);

  ASTEval evaluator;
  evaluator.EvalStmt(*func);
  ASSERT_FALSE(evaluator.Failed());

  auto i2 = std::make_unique<IntLiteral>(3);
  auto idexpr = std::make_unique<IDExpr>("func");
  std::vector<std::unique_ptr<Expr>> call_args;
  call_args.push_back(std::move(i2));
  auto call = std::make_unique<Call>(std::move(idexpr), std::move(call_args));
  ASSERT_EQ(evaluator.EvalNumeric(*call), 5);
  ASSERT_FALSE(evaluator.Failed());

  auto idexpr2 = std::make_unique<IDExpr>("func");
  std::vector<std::unique_ptr<Expr>> call_args2;
  call_args2.push_back(std::move(call));
  auto call2 =
      std::make_unique<Call>(std::move(idexpr2), std::move(call_args2));
  ASSERT_EQ(evaluator.EvalNumeric(*call2), 7);
  ASSERT_FALSE(evaluator.Failed());
}

TEST_F(EvalTest, FunctionText) {
  input_ << "def func(a) { return a + 2; }";
  Parser parser(input_);
  auto stmt = parser.ParseStmt();
  ASSERT_NE(stmt, nullptr);

  ASTEval evaluator;
  evaluator.EvalStmt(*stmt);
  ASSERT_FALSE(evaluator.Failed());

  input_.clear();
  input_ << "func(3)";
  auto expr = Parser(input_).ParseExpr();
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(evaluator.EvalNumeric(*expr), 5);
  ASSERT_FALSE(evaluator.Failed());

  input_.clear();
  input_ << "func(func(3))";
  expr = Parser(input_).ParseExpr();
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(evaluator.EvalNumeric(*expr), 7);
  ASSERT_FALSE(evaluator.Failed());
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
