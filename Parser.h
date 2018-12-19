#ifndef PARSER_H
#define PARSER_H

#include <istream>
#include <memory>
#include <ostream>

#include "Lexer.h"
#include "Nodes.h"

namespace lang {

struct ParseFailure {
  enum Reason {
    LEX_ERROR,
    EXPECTED_BIN_OPERATOR,
    EXPECTED_BIN_OPERAND_TOK,
    EXPECTED_RPAR,
    EXPECTED_ASSIGNABLE_EXPR,
    EXPECTED_ASSIGNMENT,
    EXPECTED_STMT_END,
  } reason;

  // TODO: Clean this up because it seems like a code smell that we have all
  // these constructors for just setting a few optional parameters.
  ParseFailure() = default;
  ParseFailure(Reason reason, const Token &failing_tok)
      : reason(reason), failing_tok(failing_tok) {}
  ParseFailure(Reason reason, const Token &failing_tok, LexStatus lex_status)
      : reason(reason), failing_tok(failing_tok), lex_status(lex_status) {}
  ParseFailure(Reason reason, const SourceLocation &failing_loc)
      : reason(reason), failing_loc(failing_loc) {}
  ParseFailure(Reason reason, const Token &failing_tok,
               const SourceLocation &failing_loc, LexStatus lex_status)
      : reason(reason),
        failing_tok(failing_tok),
        failing_loc(failing_loc),
        lex_status(lex_status) {}

  void Dump(std::ostream &out) const;

  Token failing_tok;

  // This may or may not be the same location as the one attached to
  // failing_tok.
  SourceLocation failing_loc;
  LexStatus lex_status;
};

class Parser {
 public:
  Parser(std::istream &input) : lexer_(input) {}

  // Return the node on success and nullptr on failue. An error message is
  // printed to the error stream in the event of failure. This is a dispatch to
  // all other ParseXXX methods.
  std::unique_ptr<Node> Parse();
  std::unique_ptr<Module> ParseModule();
  std::unique_ptr<Expr> ParseExpr();
  std::unique_ptr<Stmt> ParseStmt();
  ParseFailure getFailure() const { return failure_; }

 private:
  // Each of these ParseXXX methods expects a Lex call to succeed.
  std::unique_ptr<IntLiteral> ParseIntLiteral();
  std::unique_ptr<IDExpr> ParseIDExpr();
  std::unique_ptr<Expr> ParseBinOperandExpr();
  std::unique_ptr<ParenExpr> ParseParenExpr();
  std::unique_ptr<Expr> ParseMulDivExpr();

  // Handle error messages for any LexStatus that is not a success.
  void DiagnoseLexStatus(LexStatus status, Token &tok);

  // Return true on a successful peeking of the next token. Otherwise, return
  // false and diagnose the lexer related error.
  bool PeekAndDiagnose(Token &tok);
  void ConsumePeekedToken();

  Lexer lexer_;
  ParseFailure failure_;
};

}  // namespace lang

#endif
