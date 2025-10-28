# pragma once
# include <string>
# include <vector>
# include <memory>
# include <map>
# include <format>

enum class TokenType {
  // リテラル
  Number, String, Identifier,
  // キーワード
  Funkcio, Klaso, Se, Alie, Dum, Reveni, Tiu, Vero, Malvero,
  // 型
  Entjera, Reala, Teksta, Bulea, Funkcia,
  // 演算子
  Plus, Minus, Multiply, Divide, Assign, Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
  // 区切り
  LParen, RParen, LBrace, RBrace, Semicolon, Comma, At, Dot,
  // その他
  EndOfFile, Unknown,  
};

namespace {
  std::map<std::string, TokenType> Id2TokenType = {
    {"funkcio", TokenType::Funkcio},
    {"klaso", TokenType::Klaso},
    {"se", TokenType::Se},
    {"alie", TokenType::Alie},
    {"dum", TokenType::Dum},
    {"reveni", TokenType::Reveni},
    {"tiu", TokenType::Tiu},
    {"vero", TokenType::Vero},
    {"malvero", TokenType::Malvero},
    {"entjera", TokenType::Entjera},
    {"reala", TokenType::Reala},
    {"teksta", TokenType::Teksta},
    {"bulea", TokenType::Bulea},
    {"funkcia", TokenType::Funkcia},
  };
  std::map<TokenType, std::string> TokenType2Stirng = {
    {TokenType::Number, "Number"},
    {TokenType::String, "String"},
    {TokenType::Identifier, "Identifier"},
    {TokenType::Funkcio, "Funkcio"},
    {TokenType::Klaso, "Klaso"},
    {TokenType::Se, "Se"},
    {TokenType::Alie, "Alie"},
    {TokenType::Dum, "Dum"},
    {TokenType::Reveni, "Reveni"},
    {TokenType::Tiu, "Tiu"},
    {TokenType::Vero, "Vero"},
    {TokenType::Malvero, "Malvero"},
    {TokenType::Entjera, "Entjera"},
    {TokenType::Reala, "Reala"},
    {TokenType::Teksta, "Teksta"},
    {TokenType::Bulea, "Bulea"},
    {TokenType::Funkcia, "Funkcia"},
    {TokenType::Plus, "Plus"},
    {TokenType::Minus, "Minus"},
    {TokenType::Multiply, "Multiply"},
    {TokenType::Divide, "Divide"},
    {TokenType::Assign, "Assign"},
    {TokenType::Equal, "Equal"},
    {TokenType::NotEqual, "NotEqual"},
    {TokenType::Less, "Less"},
    {TokenType::Greater, "Greater"},
    {TokenType::LessEqual, "LessEqual"},
    {TokenType::GreaterEqual, "GreaterEqual"},
    {TokenType::LParen, "LParen"},
    {TokenType::RParen, "RParen"},
    {TokenType::LBrace, "LBrace"},
    {TokenType::RBrace, "RBrace"},
    {TokenType::Semicolon, "Semicolon"},
    {TokenType::Comma, "Comma"},
    {TokenType::At, "At"},
    {TokenType::Dot, "Dot"},
    {TokenType::EndOfFile, "EndOfFile"},
    {TokenType::Unknown, "Unknown"},
  };
}

struct Token {
  TokenType type;
  std::string value;
  int line;
  int column;
  Token(TokenType t, const std::string& v, int l, int c)
    : type(t), value(v), line(l), column(c) {}
};

template<>
struct std::formatter<Token> {
  char type = 'd';
  constexpr auto parse(std::format_parse_context& ctx) {
    auto iter = ctx.begin();
    if (iter != ctx.end() && *iter == '%') {
      ++iter;
      if (iter != ctx.end()) {
        type = *iter;
        ++iter;
      }
    }
    return iter;
  }
  auto format(const Token& token, std::format_context& ctx) const {
    switch (type) {
      case 't': return std::format_to(ctx.out(), "{}", TokenType2Stirng.at(token.type));
      case 'v': return std::format_to(ctx.out(), "{}", token.value);
      case 'l': return std::format_to(ctx.out(), "{}", token.line);
      case 'c': return std::format_to(ctx.out(), "{}", token.column);
    }
    return std::format_to(ctx.out(), "Token(l:{:04}, c:{:04}, {:>12}, \"{}\")",
      token.line, token.column, TokenType2Stirng.at(token.type), token.value
    );
  }
};

class Tokenizer {
public:
  Tokenizer(const std::string& src)
    : m_source(src), m_position(0), m_line(1), m_column(0) {}

  std::vector<Token> tokenize() {
    std::vector<Token> tokens;
    // コードの末尾に来たら終了
    while (current() != '\0') {
      // 空白やコメントはスキップ
      if(skipWhitespace()) continue;
      if(skipComment()) continue;
      // 始めの列数を記憶
      const int startColumn = m_column;
      // トークン化
      if (std::isdigit(current())) tokens.push_back(readNumber());
      else if (current() == '"') tokens.push_back(readString());
      else if (std::isalpha(current()) || current() == '_') tokens.push_back(readIdentifier());
      else {
        TokenType type = TokenType::Unknown;
        std::string value(1, current());
        switch (current()) {
          case '+': type = TokenType::Plus; break;
          case '-': type = TokenType::Minus; break;
          case '*': type = TokenType::Multiply; break;
          case '/': type = TokenType::Divide; break;
          case '(': type = TokenType::LParen; break;
          case ')': type = TokenType::RParen; break;
          case '{': type = TokenType::LBrace; break;
          case '}': type = TokenType::RBrace; break;
          case ';': type = TokenType::Semicolon; break;
          case ',': type = TokenType::Comma; break;
          case '@': type = TokenType::At; break;
          case '.': type = TokenType::Dot; break;
          case '=':
            if (peek() == '=') {
              type = TokenType::Equal;
              value = "==";
              advance();
            } else {
              type = TokenType::Assign;
            }
            break;
          case '!':
            if (peek() == '=') {
              type = TokenType::NotEqual;
              value = "!=";
              advance();
            }
            break;
          case '<':
            if (peek() == '=') {
              type = TokenType::LessEqual;
              value = "<=";
              advance();
            } else {
              type = TokenType::Less;
            }
            break;
          case '>':
            if (peek() == '=') {
              type = TokenType::GreaterEqual;
              value = ">=";
              advance();
            } else {
              type = TokenType::Greater;
            }
            break;
        }
        tokens.push_back(Token{type, value, m_line, startColumn});
        advance();
      }
    }
    // ファイル末尾のトークンを追加
    tokens.push_back(Token{TokenType::EndOfFile, "", m_line, m_column});
    return tokens;
  }

private:
  std::string m_source;
  size_t m_position;
  int m_line;
  int m_column;
  // 現在の文字を取得
  char current() const {
    return m_position < m_source.length() ? m_source[m_position] : '\0';
  }
  // 1つ先の文字を取得
  char peek(int offset = 1) const {
    const size_t pos = m_position + offset;
    return pos < m_source.length() ? m_source[pos] : '\0';
  }
  // 現在位置を1進める
  void advance() {
    if (current() == '\n') {
      m_line++;
      m_column = 0;
    } else {
      m_column++;
    }
    m_position++;
  }
  // 空白文字をスキップ
  bool skipWhitespace() {
    bool isWhitespace = false;
    while (std::isspace(current())) {
      isWhitespace = true;
      advance();
    }
    return isWhitespace;
  }
  // コメント行をスキップ
  bool skipComment() {
    bool isComment = false;
    if (current() == '/' && peek() == '/') {
      while (!(current() == '\n' || current() == '\0')) {
        isComment = true;
        advance();
      }
    }
    return isComment;
  }
  // 整数実数を読み込む
  Token readNumber() {
    const int startColumn = m_column;
    std::string number;
    bool isFloat = false;
    while (std::isdigit(current()) || current() == '.') {
      if (current() == '.') {
        if (isFloat) break;
        isFloat = true;
      }
      number += current();
      advance();
    }
    return Token{TokenType::Number, number, m_line, startColumn};
  }
  // 文字列を読み込む
  Token readString() {
    const int startColumn = m_column;
    std::string str;
    advance();
    while (!(current() == '"' || current() == '\0')) {
      if (current() == '\\') {
        advance();
        switch (current()) {
          case 'n': str += '\n'; break;
          case 't': str += '\t'; break;
          case '\\': str += '\\'; break;
          case '"': str += '"'; break;
          default: str += current();
        }
      } else {
        str += current();
      }
      advance();
    }
    if (current() == '"') advance();
    return Token{TokenType::String, str, m_line, startColumn};
  }
  // 識別子を読み込む
  Token readIdentifier() {
    const int startColumn = m_column;
    std::string id;
    while(std::isalnum(current()) || current() == '_') {
      id += current();
      advance();
    }
    TokenType type = Id2TokenType.contains(id) ? Id2TokenType.at(id) : TokenType::Identifier;
    return Token{type, id, m_line, startColumn};
  }
};