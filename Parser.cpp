#include <cassert>
#include <iostream>
#include <string>

#include "LangCommon.h"
#include "Parser.h"

namespace lang {

void Parser::DiagnoseLexStatus(LexStatus status, Token &tok) {
  failure_ = ParseFailure(ParseFailure::LEX_ERROR, tok, status);
}

bool Parser::PeekAndDiagnose(Token &tok) {
  LexStatus status = lexer_.Peek(tok);
  if (status != LEX_SUCCESS) {
    DiagnoseLexStatus(status, tok);
    return false;
  }
  return true;
}

void Parser::ConsumePeekedToken() {
  Token tok;
  assert(lexer_.Lex(tok) == LEX_SUCCESS &&
         "Expected successful Lex after successful Peek on lexer.");
}

void ParseFailure::Dump(std::ostream &out) const {
  switch (reason) {
    case LEX_ERROR:
      switch (lex_status) {
        case LEX_SUCCESS:
          out << "Lexer successfully read a token. This should not error.";
          return;
        case LEX_UNKNOWN_CHAR:
          out << "Unable to read character from stream: "
              << failing_tok.toString() << std::endl;
          return;
        case LEX_BAD_INT:
          out << "Unable to read an int from stream: " << failing_tok.toString()
              << std::endl;
          return;
      }
      LANG_UNREACHABLE("should have exited before this");
    case EXPECTED_BIN_OPERATOR:
      out << "Expected a binary operator token, but found "
          << failing_tok.toString() << std::endl;
      return;
    case EXPECTED_BIN_OPERAND_TOK:
      out << "Expected a token forming a binary operand expression, but found: "
          << failing_tok.toString() << std::endl;
      return;
    case EXPECTED_RPAR:
      out << "Expected a closing ')', but found " << failing_tok.toString()
          << std::endl;
      return;
    case EXPECTED_ASSIGNABLE_EXPR:
      out << "Expected an assignable expression at " << failing_loc.toString()
          << std::endl;
      return;
    case EXPECTED_ASSIGNMENT:
      out << "Expected an '=', but found " << failing_tok.toString()
          << std::endl;
      return;
    case EXPECTED_STMT_END:
      out << "Expected the statement to end with a ';', but found "
          << failing_tok.toString() << std::endl;
      return;
  }
  LANG_UNREACHABLE("should have exited before this");
}

std::unique_ptr<Node> Parser::Parse() { return ParseStmt(); }

/**
 * stmt : <expr> ('=' <expr>)* ';'
 */
std::unique_ptr<Stmt> Parser::ParseStmt() {
  auto lhs = ParseExpr();
  if (!lhs) return nullptr;

  // '=' or ';'
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;
  ConsumePeekedToken();

  std::unique_ptr<Stmt> stmt;
  if (tok.kind == TOK_ASSIGN) {
    // Assign
    if (!lhs->isAssignable()) {
      failure_ = ParseFailure(ParseFailure::EXPECTED_ASSIGNABLE_EXPR,
                              lexer_.getCurrentLoc());
      return nullptr;
    }

    auto rhs = ParseExpr();
    if (!rhs) return nullptr;

    std::unique_ptr<AssignableExpr> as_assignable(
        static_cast<AssignableExpr *>(lhs.release()));
    stmt = std::make_unique<Assign>(std::move(as_assignable), std::move(rhs));

    // Ending ';'
    if (!PeekAndDiagnose(tok)) return nullptr;
  } else {
    // ExprStmt
    stmt = std::make_unique<ExprStmt>(std::move(lhs));
  }

  if (tok.kind != TOK_SEMICOL) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_STMT_END, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  return stmt;
}

/**
 * mul_div_expr : <bin_operand_expr> (('*' | '/') <bin_operand_expr>)*
 */
std::unique_ptr<Expr> Parser::ParseMulDivExpr() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  std::unique_ptr<Expr> result = ParseBinOperandExpr();
  if (!result) return nullptr;

  if (!PeekAndDiagnose(tok)) return nullptr;

  while (tok.kind != TOK_END) {
    BinOperatorKind binop;
    switch (tok.kind) {
      case TOK_MUL:
        binop = BIN_MUL;
        break;
      case TOK_DIV:
        binop = BIN_DIV;
        break;
      default:
        return result;
    }
    ConsumePeekedToken();

    std::unique_ptr<Expr> RHS = ParseBinOperandExpr();
    if (RHS)
      result = std::make_unique<BinOperator>(std::move(result), std::move(RHS),
                                             binop);
    else
      return nullptr;

    if (!PeekAndDiagnose(tok)) return nullptr;
  }
  return result;
}

/**
 * expr : <mul_div_expr> (('+' | '-') <mul_div_expr>)*
 */
std::unique_ptr<Expr> Parser::ParseExpr() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  std::unique_ptr<Expr> result = ParseMulDivExpr();
  if (!result) return nullptr;

  if (!PeekAndDiagnose(tok)) return nullptr;

  while (tok.kind != TOK_END) {
    BinOperatorKind binop;
    switch (tok.kind) {
      case TOK_PLUS:
        binop = BIN_ADD;
        break;
      case TOK_MINUS:
        binop = BIN_SUB;
        break;
      default:
        return result;
    }
    ConsumePeekedToken();

    std::unique_ptr<Expr> RHS = ParseMulDivExpr();
    if (RHS)
      result = std::make_unique<BinOperator>(std::move(result), std::move(RHS),
                                             binop);
    else
      return nullptr;

    if (!PeekAndDiagnose(tok)) return nullptr;
  }
  return result;
}

std::unique_ptr<IDExpr> Parser::ParseIDExpr() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;
  ConsumePeekedToken();
  assert(tok.kind == TOK_ID &&
         "Expected the first token to be a TOK_ID when parsing an IDExpr");
  return std::make_unique<IDExpr>(tok.chars);
}

/**
 * Parse an expression that makes up a operand in a binary operation.
 *
 * bin_operand_expr : <number>
 *                  | <ID>
 *                  | <paren_expr>
 */
std::unique_ptr<Expr> Parser::ParseBinOperandExpr() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  switch (tok.kind) {
    case TOK_INT:
      return ParseIntLiteral();
    case TOK_ID:
      return ParseIDExpr();
    case TOK_LPAR:
      return ParseParenExpr();
    default:
      break;
  }

  // Parser error from unexpected token.
  failure_ = ParseFailure(ParseFailure::EXPECTED_BIN_OPERAND_TOK, tok);
  return nullptr;
}

/**
 * paren_expr : '(' <expr> ')'
 */
std::unique_ptr<ParenExpr> Parser::ParseParenExpr() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;
  assert(tok.kind == TOK_LPAR && "Expected opening parenthesis");
  ConsumePeekedToken();

  std::unique_ptr<Expr> inner = ParseExpr();
  if (!inner) return nullptr;

  auto result = std::make_unique<ParenExpr>(std::move(inner));

  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_RPAR) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_RPAR, tok);
    return nullptr;
  }
  ConsumePeekedToken();
  return result;
}

/**
 * number : [0-9]+
 */
std::unique_ptr<IntLiteral> Parser::ParseIntLiteral() {
  Token tok;
  LexStatus status = lexer_.Lex(tok);
  assert(status == LEX_SUCCESS);
  assert(
      tok.kind == TOK_INT &&
      "This method should only be called if an int is expected off the stream");

  int64_t val = std::stoll(tok.chars);
  return std::make_unique<IntLiteral>(val);
}

}  // namespace lang
