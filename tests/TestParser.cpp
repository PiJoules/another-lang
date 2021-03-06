#include <gtest/gtest.h>

#include "Parser.h"

using namespace lang;

namespace {

class ParserTest : public ::testing::Test {
 protected:
  std::stringstream input_;
};

TEST_F(ParserTest, EmptyStream) {
  Parser parser(input_);
  auto result = parser.Parse();
  ASSERT_NE(result, nullptr);
  ASSERT_EQ(result->getKind(), NODE_MODULE);

  const auto *module = static_cast<const Module *>(result.get());
  ASSERT_EQ(module->getStmts().size(), 0);
}

TEST_F(ParserTest, Int) {
  input_ << "123";
  Parser parser(input_);
  std::unique_ptr<Node> result = parser.ParseExpr();

  const auto *int_result = static_cast<const IntLiteral *>(result.get());
  ASSERT_NE(int_result, nullptr);
  ASSERT_EQ(int_result->getKind(), NODE_INT);
  ASSERT_EQ(int_result->getVal(), 123);
}

TEST_F(ParserTest, BinOp) {
  input_ << "123 + 1";
  Parser parser(input_);
  std::unique_ptr<Node> result = parser.ParseExpr();

  const auto *binop_result = static_cast<const BinOperator *>(result.get());
  ASSERT_NE(binop_result, nullptr);
  ASSERT_EQ(binop_result->getKind(), NODE_BINOP);

  ASSERT_EQ(binop_result->getLHS().getKind(), NODE_INT);
  ASSERT_EQ(binop_result->getRHS().getKind(), NODE_INT);
}

TEST_F(ParserTest, NestedBinOp) {
  input_ << "123 + 1 + 2";
  Parser parser(input_);
  std::unique_ptr<Node> result = parser.ParseExpr();

  const auto *binop_result = static_cast<const BinOperator *>(result.get());
  ASSERT_NE(binop_result, nullptr);
  ASSERT_EQ(binop_result->getKind(), NODE_BINOP);

  ASSERT_EQ(binop_result->getLHS().getKind(), NODE_BINOP);
  ASSERT_EQ(binop_result->getRHS().getKind(), NODE_INT);
}

TEST_F(ParserTest, ExpectedBinOperand) {
  input_ << "+";
  Parser parser(input_);
  std::unique_ptr<Node> result = parser.ParseExpr();
  ASSERT_EQ(result, nullptr);
  ASSERT_EQ(parser.getFailure().reason, ParseFailure::EXPECTED_BIN_OPERAND_TOK);
}

TEST_F(ParserTest, ParenExpr) {
  input_ << "(1 + 2)";
  Parser parser(input_);
  std::unique_ptr<Node> result = parser.ParseExpr();

  const auto *paren_result = static_cast<const ParenExpr *>(result.get());
  ASSERT_NE(paren_result, nullptr);
  ASSERT_EQ(paren_result->getKind(), NODE_PAREN);
  ASSERT_EQ(paren_result->getInner().getKind(), NODE_BINOP);
}

TEST_F(ParserTest, AssignStmt) {
  input_ << "a = 1;";
  Parser parser(input_);
  std::unique_ptr<Stmt> result = parser.ParseStmt();

  const auto *assign_result = static_cast<const Assign *>(result.get());
  ASSERT_NE(assign_result, nullptr);
  ASSERT_EQ(assign_result->getLHS().getKind(), NODE_ID);
  ASSERT_EQ(assign_result->getRHS().getKind(), NODE_INT);
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
