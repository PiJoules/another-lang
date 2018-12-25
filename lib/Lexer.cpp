#include <cassert>
#include <iostream>
#include <sstream>

#include "Lexer.h"

namespace lang {

std::string TokenKindToString(TokenKind kind) {
  switch (kind) {
    case TOK_INT:
      return "TOK_INT";
    case TOK_ID:
      return "TOK_ID";
    case TOK_DEF:
      return "TOK_DEF";
    case TOK_RETURN:
      return "TOK_RETURN";
    case TOK_IF:
      return "TOK_IF";
    case TOK_ELSE:
      return "TOK_ELSE";
    case TOK_LPAR:
      return "TOK_LPAR";
    case TOK_RPAR:
      return "TOK_RPAR";
    case TOK_LBRACE:
      return "TOK_LBRACE";
    case TOK_RBRACE:
      return "TOK_RBRACE";
    case TOK_PLUS:
      return "TOK_PLUS";
    case TOK_MINUS:
      return "TOK_MINUS";
    case TOK_MUL:
      return "TOK_MUL";
    case TOK_DIV:
      return "TOK_DIV";
    case TOK_ASSIGN:
      return "TOK_ASSIGN";
    case TOK_SEMICOL:
      return "TOK_SEMICOL";
    case TOK_COMMA:
      return "TOK_COMMA";
    case TOK_END:
      return "TOK_END";
    case TOK_BAD:
      return "TOK_BAD";
  }
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
      case '{':
        result = MakeSingleCharToken(input_, TOK_LBRACE);
        return LEX_SUCCESS;
      case '}':
        result = MakeSingleCharToken(input_, TOK_RBRACE);
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
      case ';':
        result = MakeSingleCharToken(input_, TOK_SEMICOL);
        return LEX_SUCCESS;
      case ',':
        result = MakeSingleCharToken(input_, TOK_COMMA);
        return LEX_SUCCESS;
      case '=':
        result = MakeSingleCharToken(input_, TOK_ASSIGN);
        return LEX_SUCCESS;
      default:
        if (isspace(c)) {
          current_loc_.colno++;
          input_.get();
          c = input_.peek();
        } else if (isdigit(c)) {
          return LexInt(result);
        } else if (isalpha(c) || c == '_') {
          return LexIDOrKeyword(result);
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

/**
 * An ID can start with an underscore or alphabetic character, followed by
 * either underscores, alphabetic characters, or digits.
 */
LexStatus Lexer::LexIDOrKeyword(Token &result) {
  int c = input_.peek();
  assert((isalpha(c) || c == '_') &&
         "Starting character in an ID must be an alphabetic character or "
         "underscore.");

  std::string str;
  while (isalnum(c) || c == '_') {
    str.push_back(c);
    input_.get();
    c = input_.peek();
  }

  result.loc = current_loc_;
  result.chars = str;

  if (str == "def")
    result.kind = TOK_DEF;
  else if (str == "return")
    result.kind = TOK_RETURN;
  else if (str == "if")
    result.kind = TOK_IF;
  else if (str == "else")
    result.kind = TOK_ELSE;
  else
    result.kind = TOK_ID;

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
