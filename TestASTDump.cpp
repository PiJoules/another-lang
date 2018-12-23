#include <gtest/gtest.h>

#include "Nodes.h"

using namespace lang;

namespace {

class ASTDumpTest : public ::testing::Test {
 protected:
  std::stringstream input_;
};

TEST_F(ASTDumpTest, Function) {
  auto func = std::make_unique<Function>("func");
  std::stringstream out;
  ASTDump(out).Dump(*func);
  ASSERT_FALSE(out.str().empty());
}

TEST_F(ASTDumpTest, Call) {
  auto idexpr = std::make_unique<IDExpr>("func");
  auto call = std::make_unique<Call>(std::move(idexpr));
  std::stringstream out;
  ASTDump(out).Dump(*call);
  ASSERT_FALSE(out.str().empty());
}

TEST_F(ASTDumpTest, Return) {
  auto idexpr = std::make_unique<IDExpr>("a");
  auto ret = std::make_unique<Return>(std::move(idexpr));
  std::stringstream out;
  ASTDump(out).Dump(*ret);
  ASSERT_FALSE(out.str().empty());
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
