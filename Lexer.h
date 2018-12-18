#ifndef LEXER_H
#define LEXER_H

#include <istream>
#include <string>

#include "LangCommon.h"

namespace lang {

enum TokenKind {
  TOK_INT,
  TOK_ID,

  // Container characters
  TOK_LPAR,
  TOK_RPAR,

  // Binary operators
  TOK_PLUS,
  TOK_MINUS,
  TOK_MUL,
  TOK_DIV,

  // Assignment operators
  TOK_ASSIGN,

  TOK_SEMICOL,
  TOK_END,
  TOK_BAD
};

std::string TokenKindToString(TokenKind kind);

struct Token {
  TokenKind kind;
  SourceLocation loc;
  std::string chars;

  std::string toString() const;
};

enum LexStatus { LEX_SUCCESS, LEX_UNKNOWN_CHAR, LEX_BAD_INT };

class Lexer {
 public:
  Lexer(std::istream &input) : input_(input) {}

  // Return true on successfully reading a token off the stream.
  // The stream may or may not be advanced regardless. Each lex method is in
  // charge of advancing the tream on it's own.
  //
  // In the event of lexing an unknown character, the stored character and
  // location are stored in the result.
  LexStatus Lex(Token &result);

  // Does the same thing as Lex, but does not advance the stream of tokens.
  LexStatus Peek(Token &result);

  SourceLocation getCurrentLoc() const { return current_loc_; }

 private:
  // This method assumes the first character in the stream is a digit.
  LexStatus LexInt(Token &result);

  // This method assumes the first character in the stream is an alphabetic
  // character.
  LexStatus LexID(Token &result);

  // Set a token consisting of a single character and advance the token
  // location. This also advances the stream. This assumes the character is not
  // a newline.
  Token MakeSingleCharToken(std::istream &input, TokenKind kind);

  std::istream &input_;
  SourceLocation current_loc_;
  Token lookahead_;
  bool has_lookahead_ = false;
};

}  // namespace lang

#endif
