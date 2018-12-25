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
    case EXPECTED_LPAR:
      out << "Expected an opening '(', but found " << failing_tok.toString()
          << std::endl;
      return;
    case EXPECTED_RPAR:
      out << "Expected a closing ')', but found " << failing_tok.toString()
          << std::endl;
      return;
    case EXPECTED_LBRACE:
      out << "Expected an opening '{', but found " << failing_tok.toString()
          << std::endl;
      return;
    case EXPECTED_RBRACE:
      out << "Expected a closing '}', but found " << failing_tok.toString()
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

std::unique_ptr<Node> Parser::Parse() { return ParseModule(); }

/**
 * module : <stmt>+
 */
std::unique_ptr<Module> Parser::ParseModule() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  std::vector<std::unique_ptr<Stmt>> stmts;
  while (tok.kind != TOK_END) {
    auto stmt = ParseStmt();
    if (!stmt) return nullptr;
    stmts.push_back(std::move(stmt));

    if (!PeekAndDiagnose(tok)) return nullptr;
  }
  return std::make_unique<Module>(std::move(stmts));
}

/**
 * stmt : <expr> ('=' <expr>)* ';'
 *      | <function>
 *      | <return>
 *      | <if>
 */
std::unique_ptr<Stmt> Parser::ParseStmt() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  if (tok.kind == TOK_DEF) return ParseFunction();
  if (tok.kind == TOK_RETURN) return ParseReturn();
  if (tok.kind == TOK_IF) return ParseIf();

  auto lhs = ParseExpr();
  if (!lhs) return nullptr;

  // '=' or ';'
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
 * if : 'if' <expr> '{' <stmt>+ '}' ('else' '{' <stmt>+ '}')?
 */
std::unique_ptr<If> Parser::ParseIf() {
  Token tok;

  // if
  if (!PeekAndDiagnose(tok)) return nullptr;
  assert(tok.kind == TOK_IF && "Expected 'if' keyword");
  ConsumePeekedToken();

  // <expr>
  std::unique_ptr<Expr> expr = ParseExpr();

  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_LBRACE) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_LBRACE, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  // <stmt>+
  std::vector<std::unique_ptr<Stmt>> if_body;
  if (!PeekAndDiagnose(tok)) return nullptr;
  while (tok.kind != TOK_RBRACE) {
    auto stmt = ParseStmt();
    if (!stmt) return nullptr;
    if_body.push_back(std::move(stmt));

    if (!PeekAndDiagnose(tok)) return nullptr;
  }

  // '}'
  ConsumePeekedToken();

  // else
  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_ELSE)
    return std::make_unique<If>(std::move(expr), std::move(if_body));
  ConsumePeekedToken();

  // '{'
  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_LBRACE) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_LBRACE, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  // <stmt>+
  std::vector<std::unique_ptr<Stmt>> else_body;
  if (!PeekAndDiagnose(tok)) return nullptr;
  while (tok.kind != TOK_RBRACE) {
    auto stmt = ParseStmt();
    if (!stmt) return nullptr;
    else_body.push_back(std::move(stmt));

    if (!PeekAndDiagnose(tok)) return nullptr;
  }

  // '}'
  ConsumePeekedToken();

  return std::make_unique<If>(std::move(expr), std::move(if_body),
                              std::move(else_body));
}

/**
 * function : def <ID> '(' <id_list_optional> ')' '{' <stmt>+ '}'
 */
std::unique_ptr<Function> Parser::ParseFunction() {
  Token tok;

  // def
  if (!PeekAndDiagnose(tok)) return nullptr;
  assert(tok.kind == TOK_DEF && "Expected 'def' keyword");
  ConsumePeekedToken();

  // <ID>
  std::unique_ptr<IDExpr> func_name = ParseIDExpr();
  if (!func_name) return nullptr;

  // (
  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_LPAR) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_LPAR, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  // <id_list_optional>
  std::vector<std::unique_ptr<IDExpr>> id_list;
  if (!ParseIDListOptional(id_list)) return nullptr;

  // ')'
  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_RPAR) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_RPAR, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  // '{'
  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_LBRACE) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_LBRACE, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  // <stmt>+
  std::vector<std::unique_ptr<Stmt>> body;
  if (!PeekAndDiagnose(tok)) return nullptr;
  while (tok.kind != TOK_RBRACE) {
    auto stmt = ParseStmt();
    if (!stmt) return nullptr;
    body.push_back(std::move(stmt));

    if (!PeekAndDiagnose(tok)) return nullptr;
  }

  // '}'
  ConsumePeekedToken();

  return std::make_unique<Function>(func_name->getName(), std::move(id_list),
                                    std::move(body));
}

/**
 * return : 'return' <expr> ';'
 */
std::unique_ptr<Return> Parser::ParseReturn() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;
  assert(tok.kind == TOK_RETURN && "Expected 'return' keyword");
  ConsumePeekedToken();

  std::unique_ptr<Expr> expr = ParseExpr();
  if (!expr) return nullptr;

  std::unique_ptr<Return> ret = std::make_unique<Return>(std::move(expr));

  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind != TOK_SEMICOL) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_STMT_END, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  return ret;
}

/**
 * <id_list_optional> : <id_expr> (',' <id_expr>)*
 *                    | none
 */
bool Parser::ParseIDListOptional(std::vector<std::unique_ptr<IDExpr>> &ids) {
  Token tok;
  if (!PeekAndDiagnose(tok)) return false;

  // Nothing to parse
  if (tok.kind != TOK_ID) return true;

  auto id_expr = ParseIDExpr();
  if (!id_expr) return false;
  ids.push_back(std::move(id_expr));

  if (!PeekAndDiagnose(tok)) return false;
  while (tok.kind == TOK_COMMA) {
    ConsumePeekedToken();

    auto other_id_expr = ParseIDExpr();
    if (!other_id_expr) return false;
    ids.push_back(std::move(other_id_expr));

    if (!PeekAndDiagnose(tok)) return false;
  }
  return true;
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
 *                  | <callable_or_call>
 *                  | <id_or_call>
 *                  | <paren_expr>
 */
std::unique_ptr<Expr> Parser::ParseBinOperandExpr() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  switch (tok.kind) {
    case TOK_INT:
      return ParseIntLiteral();
    case TOK_ID:
    case TOK_LPAR:
      return ParseCallableOrCall();
    default:
      break;
  }

  // Parser error from unexpected token.
  failure_ = ParseFailure(ParseFailure::EXPECTED_BIN_OPERAND_TOK, tok);
  return nullptr;
}

/**
 * callable_or_call : <callable> ('(' <expr_list_optional> ')'?
 */
std::unique_ptr<Expr> Parser::ParseCallableOrCall() {
  auto callable = ParseCallable();
  if (!callable) return nullptr;

  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  if (tok.kind != TOK_LPAR) return callable;

  // '('
  ConsumePeekedToken();

  if (!PeekAndDiagnose(tok)) return nullptr;
  if (tok.kind == TOK_RPAR) {
    // No args
    ConsumePeekedToken();
    return std::make_unique<Call>(std::move(callable));
  }

  auto arg = ParseExpr();
  if (!arg) return nullptr;
  std::vector<std::unique_ptr<Expr>> args;
  args.push_back(std::move(arg));

  if (!PeekAndDiagnose(tok)) return nullptr;
  while (tok.kind == TOK_COMMA) {
    ConsumePeekedToken();

    auto other_arg = ParseExpr();
    if (!other_arg) return nullptr;
    args.push_back(std::move(other_arg));

    if (!PeekAndDiagnose(tok)) return nullptr;
  }

  // ')'
  if (tok.kind != TOK_RPAR) {
    failure_ = ParseFailure(ParseFailure::EXPECTED_RPAR, tok);
    return nullptr;
  }
  ConsumePeekedToken();

  return std::make_unique<Call>(std::move(callable), std::move(args));
}

/**
 * callable : <id_expr>
 *          | <paren_expr>
 */
std::unique_ptr<Expr> Parser::ParseCallable() {
  Token tok;
  if (!PeekAndDiagnose(tok)) return nullptr;

  switch (tok.kind) {
    case TOK_ID:
      return ParseIDExpr();
    case TOK_LPAR:
      return ParseParenExpr();
    default:
      LANG_UNREACHABLE("Expected callable expression");
      return nullptr;
  }
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
