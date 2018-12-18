#include <gtest/gtest.h>

#include "Lexer.h"

using lang::Lexer;
using lang::SourceLocation;
using lang::Token;

namespace {

class LexerTest : public ::testing::Test {
 protected:
  std::stringstream input_;
};

TEST_F(LexerTest, EmptyStream) {
  Lexer lexer(input_);
  Token result;
  ASSERT_EQ(lexer.Lex(result), lang::LEX_SUCCESS);
  ASSERT_EQ(result.kind, lang::TOK_END);
  ASSERT_STREQ(result.chars.c_str(), "");
  ASSERT_EQ(result.loc, SourceLocation(0, 0));
  ASSERT_EQ(lexer.getCurrentLoc(), SourceLocation(0, 0));
}

TEST_F(LexerTest, Int) {
  input_ << "128";
  Lexer lexer(input_);
  Token result;
  ASSERT_EQ(lexer.Lex(result), lang::LEX_SUCCESS);
  ASSERT_STREQ(result.chars.c_str(), "128");
  ASSERT_EQ(result.loc, SourceLocation(0, 0));
  ASSERT_EQ(lexer.getCurrentLoc(), SourceLocation(0, 3));
}

TEST_F(LexerTest, IntTrailingSpace) {
  input_ << "128 ";
  Lexer lexer(input_);
  Token result;
  ASSERT_EQ(lexer.Lex(result), lang::LEX_SUCCESS);
  ASSERT_STREQ(result.chars.c_str(), "128");
  ASSERT_EQ(result.loc, SourceLocation(0, 0));
  ASSERT_EQ(lexer.getCurrentLoc(), SourceLocation(0, 3));
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
