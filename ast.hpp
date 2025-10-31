# pragma once
# include <memory>
# include <vector>
# include <string>
# include <sstream>
# include <concepts>

// 型
enum class TypeKind {
  Entjera,
  Reala,
  Teksta,
  Bulea,
  Funkcia,
  Klaso,
  Void,
};

namespace {
  std::map<TypeKind, std::string> TypeKind2String = {
    {TypeKind::Entjera, "entjera"},
    {TypeKind::Reala, "reala"},
    {TypeKind::Teksta, "teksta"},
    {TypeKind::Bulea, "bulea"},
    {TypeKind::Funkcia, "funkcia"},
    {TypeKind::Klaso, "klaso"},
    {TypeKind::Void, "void"},
  };
}

struct Type {
  TypeKind kind;
  std::shared_ptr<Type> returnType;
  std::shared_ptr<Type> paramType;
  std::string className;
  Type(TypeKind k) : kind(k) {}
  static std::shared_ptr<Type> createFunction(std::shared_ptr<Type> param, std::shared_ptr<Type> ret) {
    auto t = std::make_shared<Type>(TypeKind::Funkcia);
    t->paramType = param;
    t->returnType = ret;
    return t;
  }
  std::string toString() const {
    if (kind == TypeKind::Funkcia && paramType && returnType) {
      return std::format("({} -> {})", paramType->toString(), returnType->toString());
    }
    if (kind == TypeKind::Klaso) {
      return className;
    }
    return TypeKind2String.at(kind);
  }
};

// 基底ASTノード
struct ASTNode {
  virtual ~ASTNode() = default;
  virtual std::string toString(const int indent = 0) const = 0;
protected:
  std::string indentStr(const int indent) const {
    return std::string(indent*2, ' ');
  }
};

// 式ノード
struct ExprNode : public ASTNode {
  std::shared_ptr<Type> type;
};

// 数値リテラル
struct NumberLiteral : public ExprNode {
  double value;
  bool isInteger;
  NumberLiteral(const double v, const bool isInt)
    : value(v), isInteger(isInt) {}
  std::string toString(const int indent = 0) const override {
    if (isInteger) {
      return std::format("{}NumberLiteral({})", indentStr(indent), static_cast<int>(value));
    }
    return std::format("{}NumberLiteral({})", indentStr(indent), value);
  }
};

// 文字列リテラル
struct StringLiteral : public ExprNode {
  std::string value;
  StringLiteral(const std::string& v)
    : value(v) {}
  std::string toString(const int indent = 0) const override {
    return std::format("{}StringLiteral(\"{}\")", indentStr(indent), value);
  }
};

// 真偽値リテラル
struct BoolLiteral : public ExprNode {
  bool value;
  BoolLiteral(const bool v)
    : value(v) {}
  std::string toString(const int indent = 0) const override {
    return std::format("{}BoolLiteral({})", indentStr(indent), value ? "vero" : "malvero");
  }
};

// 変数参照
struct VarRefNode : public ExprNode {
  std::string name;
  VarRefNode(const std::string& n)
    : name(n) {}
  std::string toString(const int indent = 0) const override {
    return std::format("{}VarRef({})", indentStr(indent), name);
  }
};

// 二項演算
struct BinaryOpNode : public ExprNode {
  enum class OpType {
    Add, Sub, Mul, Div,
    Eq, NEq, LT, GT, LE, GE
  };
  OpType op;
  std::shared_ptr<ExprNode> left;
  std::shared_ptr<ExprNode> right;
  BinaryOpNode(const OpType& o, const std::shared_ptr<ExprNode>& l, const std::shared_ptr<ExprNode>& r)
    : op(o), left(l), right(r) {}
  std::string toString(const int indent = 0) const override {
    static const std::map<OpType, std::string> opNames{
      {OpType::Add, "+"}, {OpType::Sub, "-"}, {OpType::Mul, "*"}, {OpType::Div, "/"},
      {OpType::Eq, "=="}, {OpType::NEq, "!="}, {OpType::LT, "<"}, {OpType::GT, ">"}, {OpType::LE, "<="}, {OpType::GE, ">="}
    };
    std::ostringstream oss;
    oss << indentStr(indent) << "BinaryOp(" << opNames.at(op) << ")\n";
    oss << left->toString(indent+1) << "\n";
    oss << right->toString(indent+1);
    return oss.str();
  }
};

// 関数呼び出し
struct CallNode : public ExprNode {
  std::shared_ptr<ExprNode> function;
  std::shared_ptr<ExprNode> argument;
  CallNode(const std::shared_ptr<ExprNode>& f, const std::shared_ptr<ExprNode>& a)
    : function(f), argument(a) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "Call\n";
    oss << indentStr(indent+1) << "function:\n" << function->toString(indent+2) << "\n";
    oss << indentStr(indent+1) << "argument:\n" << argument->toString(indent+2);
    return oss.str();
  }
};

// アット関数
struct AtFunctionNode : public ExprNode {
  std::string paramName;
  std::shared_ptr<Type> paramType;
  std::shared_ptr<Type> returnType;
  std::vector<std::shared_ptr<ASTNode>> body;
  AtFunctionNode(const std::string& param, const std::shared_ptr<Type>& pType, const std::shared_ptr<Type>& rType)
    : paramName(param), paramType(pType), returnType(rType) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "AtFunction(@(" << paramType->toString() << " " << paramName << ")" << returnType->toString() << ")\n";
    oss << indentStr(indent+1) << "body:\n";
    for (size_t i = 0; i < body.size(); ++i) {
      oss << body[i]->toString(indent+2);
      if (i < body.size() - 1) oss << "\n";
    }
    return oss.str();
  }
};

// メンバーアクセス
struct MemberAccessNode : public ExprNode {
  std::shared_ptr<ExprNode> object;
  std::string member;
  MemberAccessNode(const std::shared_ptr<ExprNode>& obj, const std::string& m)
    : object(obj), member(m) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "MemberAccess(." << member << ")\n";
    oss << object->toString(indent+1);
    return oss.str();
  }
};

// 文ノード
struct StmtNode : public ASTNode {};

// 変数宣言
struct VarDeclNode : public StmtNode {
  std::string name;
  std::shared_ptr<Type> type;
  std::shared_ptr<ExprNode> initializer;
  VarDeclNode(const std::string& n, const std::shared_ptr<Type>& t, const std::shared_ptr<ExprNode>& init)
    : name(n), type(t), initializer(init) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "VarDecl(" << type->toString() << " " << name << ")";
    if (initializer) {
      oss << "\n" << indentStr(indent+1) << "initializer:\n";
      oss << initializer->toString(indent+2);
    }
    return oss.str();
  }
};

// 代入
struct AssignNode : public StmtNode {
  std::string name;
  std::shared_ptr<ExprNode> value;
  AssignNode(const std::string& n, const std::shared_ptr<ExprNode>& v)
    : name(n), value(v) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "Assign(" << name << ")\n";
    oss << value->toString(indent+1);
    return oss.str();
  }
};

// 関数宣言
struct FunctionDeclNode : public StmtNode {
  std::string name;
  std::string paramName;
  std::shared_ptr<Type> paramType;
  std::shared_ptr<Type> returnType;
  std::vector<std::shared_ptr<ASTNode>> body;
  FunctionDeclNode(const std::string& n, const std::string& param, const std::shared_ptr<Type>& pType, const std::shared_ptr<Type> rType)
    : name(n), paramName(param), paramType(pType), returnType(rType) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "FunctionDecl(" << name << "(" << paramType->toString() << " " << paramName << ")" << returnType->toString() << ")\n";
    oss << indentStr(indent+1) << "body:\n";
    for (size_t i = 0; i < body.size(); ++i) {
      oss << body[i]->toString(indent+2);
      if (i < body.size() - 1) oss << "\n";
    }
    return oss.str();
  }
};

// return文
struct ReturnNode : public StmtNode {
  std::shared_ptr<ExprNode> value;
  ReturnNode(const std::shared_ptr<ExprNode>& v)
    : value(v) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "Return\n";
    oss << value->toString(indent+1);
    return oss.str();
  }
};

// if文
struct IfNode : public StmtNode {
  std::shared_ptr<ExprNode> condition;
  std::vector<std::shared_ptr<ASTNode>> thenBody;
  std::vector<std::shared_ptr<ASTNode>> elseBody;
  IfNode(const std::shared_ptr<ExprNode>& cond)
    : condition(cond) {}
  std::string toString(const int indent = 0) const override {
    std::stringstream oss;
    oss << indentStr(indent) << "If\n";
    oss << indentStr(indent+1) << "condition:\n";
    oss << condition->toString(indent+2) << "\n";
    oss << indentStr(indent+1) << "then:\n";
    for (size_t i = 0; i < thenBody.size(); ++i) {
      oss << thenBody[i]->toString(indent+2);
      if (i < thenBody.size() - 1) oss << "\n";
    }
    if (!elseBody.empty()) {
      oss << "\n" << indentStr(indent+1) << "else:\n";
      for (size_t i = 0; i < elseBody.size(); ++i) {
        oss << elseBody[i]->toString(indent+2);
        if (i < elseBody.size() - 1) oss << "\n";
      }
    }
    return oss.str();
  }
};

// while文
struct WhileNode : public StmtNode {
  std::shared_ptr<ExprNode> condition;
  std::vector<std::shared_ptr<ASTNode>> body;
  WhileNode(const std::shared_ptr<ExprNode>& cond)
    : condition(cond) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "While\n";
    oss << indentStr(indent+1) << "condition:\n";
    oss << condition->toString(indent+2) << "\n";
    oss << indentStr(indent+1) << "body:\n";
    for (size_t i = 0; i < body.size(); ++i) {
      oss << body[i]->toString(indent+2);
      if (i < body.size() - 1) oss << "\n";
    }
    return oss.str();
  }
};

// クラス宣言
struct ClassDeclNode : public StmtNode {
  std::string name;
  std::vector<std::shared_ptr<VarDeclNode>> fields;
  std::vector<std::shared_ptr<FunctionDeclNode>> methods;
  ClassDeclNode(const std::string& n)
    : name(n) {}
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "ClassDecl(" << name << ")\n";
    if (!fields.empty()) {
      oss << indentStr(indent+1) << "fields:\n";
      for (size_t i = 0; i < fields.size(); ++i) {
        oss << fields[i]->toString(indent+2);
        if (i < fields.size() - 1) oss << "\n";
      }
    }
    if (!methods.empty()) {
      oss << "\n" << indentStr(indent+1) << "methods:\n";
      for (size_t i = 0; i < methods.size(); ++i) {
        oss << methods[i]->toString(indent+2);
        if (i < methods.size() - 1) oss << "\n";
      }
    }
    return oss.str();
  }
};

// プログラム全体
struct ProgramNode : public ASTNode {
  std::vector<std::shared_ptr<ASTNode>> statements;
  std::string toString(const int indent = 0) const override {
    std::ostringstream oss;
    oss << indentStr(indent) << "Program\n";
    for (size_t i = 0; i < statements.size(); ++i) {
      oss << statements[i]->toString(indent+2);
      if (i < statements.size() - 1) oss << "\n";
    }
    return oss.str();
  }
};

template<typename T>
requires std::derived_from<T, ASTNode>
struct std::formatter<std::shared_ptr<T>> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }
  auto format(const std::shared_ptr<T>& node, std::format_context& ctx) const {
    if (node) {
      return std::format_to(ctx.out(), "{}", node->toString());
    }
    return std::format_to(ctx.out(), "nullptr");
  }
};

