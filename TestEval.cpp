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
  auto result = parser.Parse();
  ASSERT_NE(result, nullptr);
  auto *expr = static_cast<Expr *>(result.get());

  ASTEval evaluator;
  ASSERT_EQ(evaluator.Visit(*expr), 7);
}

TEST_F(EvalTest, Paren) {
  input_ << "2 * (1 + 2) * 3";
  Parser parser(input_);
  auto result = parser.Parse();
  ASSERT_NE(result, nullptr);
  auto *expr = static_cast<Expr *>(result.get());

  ASTEval evaluator;
  ASSERT_EQ(evaluator.Visit(*expr), 18);
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
