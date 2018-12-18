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

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
