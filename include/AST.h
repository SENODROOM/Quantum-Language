#pragma once
#include <memory>
#include <string>
#include <vector>
#include <variant>

// Forward declarations
struct ASTNode;
using ASTNodePtr = std::unique_ptr<ASTNode>;

// ─── Expression Types ───────────────────────────────────────────────────────

struct NumberLiteral   { double value; };
struct StringLiteral   { std::string value; };
struct BoolLiteral     { bool value; };
struct NilLiteral      {};

struct Identifier      { std::string name; };

struct BinaryExpr {
    std::string op;
    ASTNodePtr left, right;
};

struct UnaryExpr {
    std::string op;
    ASTNodePtr operand;
};

struct AssignExpr {
    std::string op;   // = += -= *= /=
    ASTNodePtr target;
    ASTNodePtr value;
};

struct CallExpr {
    ASTNodePtr callee;
    std::vector<ASTNodePtr> args;
};

struct IndexExpr {
    ASTNodePtr object;
    ASTNodePtr index;
};

struct MemberExpr {
    ASTNodePtr object;
    std::string member;
};

struct ArrayLiteral {
    std::vector<ASTNodePtr> elements;
};

struct DictLiteral {
    std::vector<std::pair<ASTNodePtr, ASTNodePtr>> pairs;
};

struct LambdaExpr {
    std::vector<std::string> params;
    ASTNodePtr body;
};

// ─── Statement Types ─────────────────────────────────────────────────────────

struct VarDecl {
    bool isConst;
    std::string name;
    ASTNodePtr initializer; // may be null
};

struct FunctionDecl {
    std::string name;
    std::vector<std::string> params;
    ASTNodePtr body; // BlockStmt
};

struct ReturnStmt {
    ASTNodePtr value; // may be null
};

struct IfStmt {
    ASTNodePtr condition;
    ASTNodePtr thenBranch;
    // elif chains stored as else-if
    ASTNodePtr elseBranch; // may be null
};

struct WhileStmt {
    ASTNodePtr condition;
    ASTNodePtr body;
};

struct ForStmt {
    std::string var;
    ASTNodePtr iterable;
    ASTNodePtr body;
};

struct BlockStmt {
    std::vector<ASTNodePtr> statements;
};

struct ExprStmt {
    ASTNodePtr expr;
};

struct PrintStmt {
    std::vector<ASTNodePtr> args;
    bool newline;
};

struct InputStmt {
    std::string target;
    ASTNodePtr prompt;
};

struct BreakStmt  {};
struct ContinueStmt {};

struct ImportStmt {
    std::string module;
    std::string alias; // optional
};

struct ClassDecl {
    std::string name;
    std::string base;  // optional
    std::vector<ASTNodePtr> methods;
    std::vector<ASTNodePtr> fields;
};

// ─── ASTNode variant ─────────────────────────────────────────────────────────

using NodeVariant = std::variant<
    NumberLiteral, StringLiteral, BoolLiteral, NilLiteral,
    Identifier,
    BinaryExpr, UnaryExpr, AssignExpr,
    CallExpr, IndexExpr, MemberExpr,
    ArrayLiteral, DictLiteral, LambdaExpr,
    VarDecl, FunctionDecl, ReturnStmt,
    IfStmt, WhileStmt, ForStmt,
    BlockStmt, ExprStmt,
    PrintStmt, InputStmt,
    BreakStmt, ContinueStmt,
    ImportStmt, ClassDecl
>;

struct ASTNode {
    NodeVariant node;
    int line = 0;

    template<typename T>
    ASTNode(T&& n, int ln = 0) : node(std::forward<T>(n)), line(ln) {}

    template<typename T>
    T& as() { return std::get<T>(node); }

    template<typename T>
    const T& as() const { return std::get<T>(node); }

    template<typename T>
    bool is() const { return std::holds_alternative<T>(node); }
};
