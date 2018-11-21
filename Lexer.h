#ifndef LEXER_H
#define LEXER_H

#include <istream>
#include <string>

namespace lang {

enum TokenKind {
  TOK_INT,

  // Container characters
  TOK_LPAR,
  TOK_RPAR,

  // Binary operators
  TOK_PLUS,
  TOK_MINUS,
  TOK_MUL,
  TOK_DIV,

  TOK_END,
  TOK_BAD
};

std::string TokenKindToString(TokenKind kind);

struct TokenLocation {
  // These index from zero.
  int lineno;
  int colno;

  TokenLocation() : lineno(0), colno(0) {}
  TokenLocation(int lineno, int colno) : lineno(lineno), colno(colno) {}

  std::string toString() const;

  bool operator==(const TokenLocation &Other) const;
  bool operator!=(const TokenLocation &Other) const;
};

struct Token {
  TokenKind kind;
  TokenLocation loc;
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

  TokenLocation getCurrentLoc() const { return current_loc_; }

 private:
  // This method assumes the first character in the stream is a digit.
  LexStatus LexInt(Token &result);

  // Set a token consisting of a single character and advance the token
  // location. This also advances the stream. This assumes the character is not
  // a newline.
  Token MakeSingleCharToken(std::istream &input, TokenKind kind);

  std::istream &input_;
  TokenLocation current_loc_;
  Token lookahead_;
  bool has_lookahead_ = false;
};

}  // namespace lang

#endif
