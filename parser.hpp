# pragma once
# include "token.hpp"
# include "ast.hpp"
# include <stdexcept>
# include <stacktrace>

class Parser {
public:
  Parser(const std::vector<Token>& tokens)
    : m_tokens(tokens), m_position(0) {}
  std::shared_ptr<ProgramNode> parse() {
    const auto program = std::make_shared<ProgramNode>();
    while (current().type != TokenType::EndOfFile) {
      program->statements.push_back(parseStatement());
    }
    return program;
  }
  size_t getCurrentPosition() {
    return m_position;
  }
  Token getCurrentToken() {
    return current();
  }
private:
  std::vector<Token> m_tokens;
  size_t m_position;
  const Token& current() const {
    return m_tokens[m_position];
  }
  const Token& peek(const int offset = 1) const {
    size_t pos = m_position + offset;
    return pos < m_tokens.size() ? m_tokens[pos] : m_tokens.back();
  }
  void advance() {
    if (m_position < m_tokens.size() - 1) m_position++;
  }
  bool match(const TokenType type) {
    if (current().type == type) {
      advance();
      return true;
    }
    return false;
  }
  void expect(const TokenType type, const std::string& message) {
    if (!match(type)) {
      throw std::runtime_error(message + " at line " + std::to_string(current().line));
    }
  }
  std::shared_ptr<Type> parseType() {
    const TokenType t = current().type;
    advance();
    switch (t) {
      case TokenType::Entjera:
        return std::make_shared<Type>(TypeKind::Entjera);
      case TokenType::Reala:
        return std::make_shared<Type>(TypeKind::Reala);
      case TokenType::Teksta:
        return std::make_shared<Type>(TypeKind::Teksta);
      case TokenType::Bulea:
        return std::make_shared<Type>(TypeKind::Bulea);
      case TokenType::Funkcia:
        return std::make_shared<Type>(TypeKind::Funkcia);
      default:
        throw std::runtime_error("Expected type");
    }
  }
  std::shared_ptr<ExprNode> parsePrimary() {
    // 数値リテラル
    const TokenType type = current().type;
    if (type == TokenType::Number) {
      const double value = std::stod(current().value);
      const bool isInt = current().value.contains('.');
      advance();
      return std::make_shared<NumberLiteral>(value, isInt);
    }
    // 文字列リテラル
    if (type == TokenType::String) {
      const std::string value = current().value;
      advance();
      return std::make_shared<StringLiteral>(value);
    }
    // 真偽値リテラル
    if (type == TokenType::Vero) {
      advance();
      return std::make_shared<BoolLiteral>(true);
    }
    if (type == TokenType::Malvero) {
      advance();
      return std::make_shared<BoolLiteral>(false);
    }
    // アット関数 @(Type x) RetType {}
    if (match(TokenType::At)) {
      // '('
      expect(TokenType::LParen, "Expected '(' after '@'");
      // Type x
      const auto paramType = parseType();
      const std::string paramName = current().value;
      expect(TokenType::Identifier, "Expected parameter name");
      // ')'
      expect(TokenType::RParen, "Expected ')'");
      // RetType
      const auto returnType = parseType();
      auto atFunc = std::make_shared<AtFunctionNode>(
        paramName, paramType, returnType
      );
      expect(TokenType::LBrace, "Expected '{'");
      while (!match(TokenType::RBrace)) {
        atFunc->body.push_back(parseStatement());
      }
      return atFunc;
    }
    // 丸括弧
    if (match(TokenType::LParen)) {
      auto expr = parseExpression();
      expect(TokenType::RParen, "Expected ')'");
      return expr;
    }
    // 変数参照
    if (type == TokenType::Identifier || type == TokenType::Tiu) {
      const std::string name = current().value;
      advance();
      return std::make_shared<VarRefNode>(name);
    }
    throw std::runtime_error("Unexpected token in expression");
  }
  std::shared_ptr<ExprNode> parsePostfix() {
    auto expr = parsePrimary();
    while (true) {
      // 関数呼び出し
      if (match(TokenType::LParen)) {
        auto arg = parseExpression();
        expect(TokenType::RParen, "Expected ')'");
        expr = std::make_shared<CallNode>(expr, arg);
      }
      // メンバーアクセス
      else if (match(TokenType::Dot)) {
        const std::string member = current().value;
        expect(TokenType::Identifier, "Expected member name");
        expr = std::make_shared<MemberAccessNode>(expr, member);
      }
      else {
        break;
      }
    }
    return expr;
  }
  std::shared_ptr<ExprNode> parseMultiplicative() {
    auto left = parsePostfix();
    while (current().type == TokenType::Multiply || current().type == TokenType::Divide) {
      const auto op = current().type == TokenType::Multiply ?
        BinaryOpNode::OpType::Mul : BinaryOpNode::OpType::Div;
      advance();
      const auto right = parsePostfix();
      left = std::make_shared<BinaryOpNode>(op, left, right);
    }
    return left;
  }
  std::shared_ptr<ExprNode> parseAdditive() {
    auto left = parseMultiplicative();
    while (current().type == TokenType::Plus || current().type == TokenType::Minus) {
      const auto op = current().type == TokenType::Plus ?
        BinaryOpNode::OpType::Add : BinaryOpNode::OpType::Sub;
      advance();
      const auto right = parseMultiplicative();
      left = std::make_shared<BinaryOpNode>(op, left, right);
    }
    return left;
  }
  std::shared_ptr<ExprNode> parseComparison() {
    auto left = parseAdditive();
    while (current().type == TokenType::Less || current().type == TokenType::Greater
        || current().type == TokenType::LessEqual || current().type == TokenType::GreaterEqual
        || current().type == TokenType::Equal || current().type == TokenType::NotEqual) {
      BinaryOpNode::OpType op;
      switch (current().type) {
        case TokenType::Less: op = BinaryOpNode::OpType::LT; break;
        case TokenType::Greater: op = BinaryOpNode::OpType::GT; break;
        case TokenType::LessEqual: op = BinaryOpNode::OpType::LE; break;
        case TokenType::GreaterEqual: op = BinaryOpNode::OpType::GE; break;
        case TokenType::Equal: op = BinaryOpNode::OpType::Eq; break;
        case TokenType::NotEqual: op = BinaryOpNode::OpType::NEq; break;
        default: throw std::runtime_error("Unknown operator");
      }
      advance();
      const auto right = parseAdditive();
      left = std::make_shared<BinaryOpNode>(op, left, right);
    }
    return left;
  }
  std::shared_ptr<ExprNode> parseExpression() {
    return parseComparison();
  }
  std::shared_ptr<ASTNode> parseStatement() {
    const TokenType type = current().type;
    // 変数宣言
    if (type == TokenType::Entjera
     || type == TokenType::Reala
     || type == TokenType::Teksta
     || type == TokenType::Bulea
     || type == TokenType::Funkcia) {
      const auto typeVariable = parseType();
      std::string name = current().value;
      expect(TokenType::Identifier, "Expected variable name");
      std::shared_ptr<ExprNode> init = nullptr;
      if (match(TokenType::Assign)) {
        init = parseExpression();
      }
      expect(TokenType::Semicolon, "Expected ';'");
      return std::make_shared<VarDeclNode>(name, typeVariable, init);
    }
    // 関数宣言 funkcio name(Type param) RetType {}
    if (match(TokenType::Funkcio)) {
      const std::string name = current().value;
      expect(TokenType::Identifier, "Expected function name");
      expect(TokenType::LParen, "Expected '('");
      const auto paramType = parseType();
      const std::string paramName = current().value;
      expect(TokenType::Identifier, "Expected parameter name");
      expect(TokenType::RParen, "Expected ')'");
      const auto returnType = parseType();
      auto func = std::make_shared<FunctionDeclNode>(
        name, paramName, paramType, returnType
      );
      expect(TokenType::LBrace, "Expected '{'");
      while (!match(TokenType::RBrace)) {
        func->body.push_back(parseStatement());
      }
      return func;
    }
    // reveni文
    if (match(TokenType::Reveni)) {
      const auto value = parseExpression();
      expect(TokenType::Semicolon, "Expected ';'");
      return std::make_shared<ReturnNode>(value);
    }
    // se文
    if (match(TokenType::Se)) {
      expect(TokenType::LParen, "Expected '('");
      const auto condition = parseExpression();
      expect(TokenType::RParen, "Expected ')'");
      const auto ifNode = std::make_shared<IfNode>(condition);
      expect(TokenType::LBrace, "Expected '{'");
      while (!match(TokenType::RBrace)) {
        ifNode->thenBody.push_back(parseStatement());
      }
      if (match(TokenType::Alie)) {
        expect(TokenType::LBrace, "Expected '{'");
        while (!match(TokenType::RBrace)) {
          ifNode->elseBody.push_back(parseStatement());
        }
      }
      return ifNode;
    }
    // dum文
    if (match(TokenType::Dum)) {
      expect(TokenType::LParen, "Expected '('");
      const auto condition = parseExpression();
      expect(TokenType::RParen, "Expected ')'");
      const auto whileNode = std::make_shared<WhileNode>(condition);
      expect(TokenType::LBrace, "Expected '{");
      while (!match(TokenType::RBrace)) {
        whileNode->body.push_back(parseStatement());
      }
      return whileNode;
    }
    // 式と文
    const auto expr = parseExpression();
    // 代入
    if (auto varRef = std::dynamic_pointer_cast<VarRefNode>(expr)) {
      if (match(TokenType::Assign)) {
        const auto value = parseExpression();
        expect(TokenType::Semicolon, "Expected ';'");
        return std::make_shared<AssignNode>(varRef->name, value);
      }
    }
    expect(TokenType::Semicolon, "Expected ';'");
    return expr;
  }
};

