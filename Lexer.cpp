#include <cassert>
#include <iostream>
#include <sstream>

#include "Lexer.h"

namespace lang {

std::string TokenKindToString(TokenKind kind) {
  switch (kind) {
    case TOK_INT:
      return "TOK_INT";
    case TOK_LPAR:
      return "TOK_LPAR";
    case TOK_RPAR:
      return "TOK_RPAR";
    case TOK_PLUS:
      return "TOK_PLUS";
    case TOK_MINUS:
      return "TOK_MINUS";
    case TOK_MUL:
      return "TOK_MUL";
    case TOK_DIV:
      return "TOK_DIV";
    case TOK_END:
      return "TOK_END";
    case TOK_BAD:
      return "TOK_BAD";
  }
}

std::string TokenLocation::toString() const {
  std::stringstream str;
  str << "<lineno=" << lineno << " colno=" << colno << ">";
  return str.str();
}

bool TokenLocation::operator==(const TokenLocation &Other) const {
  return lineno == Other.lineno && colno == Other.colno;
}

bool TokenLocation::operator!=(const TokenLocation &Other) const {
  return !(*this == Other);
}

std::string Token::toString() const {
  std::stringstream str;
  str << "<kind=" << TokenKindToString(kind) << " chars='" << chars << "'"
      << " loc=" << loc.toString() << ">";
  return str.str();
}

Token Lexer::MakeSingleCharToken(std::istream &input_, TokenKind kind) {
  char c = input_.get();
  assert(c != '\n' &&
         "Expected a single character token that is not a newline");
  Token result = {
      .kind = kind, .loc = current_loc_, .chars = std::string(1, c)};
  current_loc_.colno++;
  return result;
}

LexStatus Lexer::Lex(Token &result) {
  if (has_lookahead_) {
    result = lookahead_;
    has_lookahead_ = false;
    return LEX_SUCCESS;
  }

  if (input_.eof()) {
    result.kind = TOK_END;
    result.loc = current_loc_;
    return LEX_SUCCESS;
  }

  assert(input_.good() &&
         "Bad input stream. This could be due to the stream being emptied on "
         "an earlier run then being reused.");

  int c = input_.peek();
  while (c != EOF) {
    switch (c) {
      case '\n':
        input_.get();

        current_loc_.lineno++;
        current_loc_.colno = 0;
        c = input_.peek();
        break;
      case '(':
        result = MakeSingleCharToken(input_, TOK_LPAR);
        return LEX_SUCCESS;
      case ')':
        result = MakeSingleCharToken(input_, TOK_RPAR);
        return LEX_SUCCESS;
      case '+':
        result = MakeSingleCharToken(input_, TOK_PLUS);
        return LEX_SUCCESS;
      case '-':
        result = MakeSingleCharToken(input_, TOK_MINUS);
        return LEX_SUCCESS;
      case '*':
        result = MakeSingleCharToken(input_, TOK_MUL);
        return LEX_SUCCESS;
      case '/':
        result = MakeSingleCharToken(input_, TOK_DIV);
        return LEX_SUCCESS;
      default:
        if (isspace(c)) {
          current_loc_.colno++;
          input_.get();
          c = input_.peek();
        } else if (isdigit(c)) {
          return LexInt(result);
        } else {
          result.loc = current_loc_;
          result.chars = c;
          result.kind = TOK_BAD;
          return LEX_UNKNOWN_CHAR;
        }
    }
  }

  // Reached EOF after reading newlines.
  result.kind = TOK_END;
  result.loc = current_loc_;
  result.chars = "";
  return LEX_SUCCESS;
}

LexStatus Lexer::LexInt(Token &result) {
  int c = input_.peek();
  assert(isdigit(c));

  std::string str;
  while (isdigit(c)) {
    str.push_back(c);
    input_.get();
    c = input_.peek();
  }

  result.kind = TOK_INT;
  result.loc = current_loc_;
  result.chars = str;

  current_loc_.colno += str.size();
  return LEX_SUCCESS;
}

LexStatus Lexer::Peek(Token &result) {
  if (has_lookahead_) {
    result = lookahead_;
    return LEX_SUCCESS;
  } else {
    LexStatus status = Lex(lookahead_);
    result = lookahead_;
    has_lookahead_ = true;
    return status;
  }
}

}  // namespace lang
