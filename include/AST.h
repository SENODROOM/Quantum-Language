#pragma once
#include <memory>
#include <string>
#include <vector>
#include <variant>

// Forward declarations
struct ASTNode;
using ASTNodePtr = std::unique_ptr<ASTNode>;

// ─── Expression Types ───────────────────────────────────────────────────────

struct NumberLiteral
{
    double value;
};
struct StringLiteral
{
    std::string value;
};
struct BoolLiteral
{
    bool value;
};
struct NilLiteral
{
};

struct Identifier
{
    std::string name;
};

struct BinaryExpr
{
    std::string op;
    ASTNodePtr left, right;
};

struct UnaryExpr
{
    std::string op;
    ASTNodePtr operand;
};

struct AssignExpr
{
    std::string op; // = += -= *= /=
    ASTNodePtr target;
    ASTNodePtr value;
};

struct CallExpr
{
    ASTNodePtr callee;
    std::vector<ASTNodePtr> args;
};

struct IndexExpr
{
    ASTNodePtr object;
    ASTNodePtr index;
};

// Python slice: obj[start:stop:step]  — any part may be null (omitted)
struct SliceExpr
{
    ASTNodePtr object;
    ASTNodePtr start; // may be null → 0
    ASTNodePtr stop;  // may be null → end
    ASTNodePtr step;  // may be null → 1
};

struct MemberExpr
{
    ASTNodePtr object;
    std::string member;
};

struct ArrayLiteral
{
    std::vector<ASTNodePtr> elements;
};

struct DictLiteral
{
    std::vector<std::pair<ASTNodePtr, ASTNodePtr>> pairs;
};

struct LambdaExpr
{
    std::vector<std::string> params;
    ASTNodePtr body;
};

struct TernaryExpr
{
    ASTNodePtr condition;
    ASTNodePtr thenExpr;
    ASTNodePtr elseExpr;
};

// ─── Statement Types ─────────────────────────────────────────────────────────

struct VarDecl
{
    bool isConst;
    std::string name;
    ASTNodePtr initializer; // may be null
    std::string typeHint;   // e.g. "int", "float", "char", "" = none
};

struct FunctionDecl
{
    std::string name;
    std::vector<std::string> params;
    ASTNodePtr body; // BlockStmt
};

struct ReturnStmt
{
    ASTNodePtr value; // may be null
};

struct IfStmt
{
    ASTNodePtr condition;
    ASTNodePtr thenBranch;
    // elif chains stored as else-if
    ASTNodePtr elseBranch; // may be null
};

struct WhileStmt
{
    ASTNodePtr condition;
    ASTNodePtr body;
};

struct ForStmt
{
    std::string var;
    std::string var2; // optional second variable for tuple unpacking: for k, v in ...
    ASTNodePtr iterable;
    ASTNodePtr body;
};

// List comprehension: [expr for var in iterable (if cond)?]
struct ListComp
{
    ASTNodePtr expr;               // the value expression
    std::vector<std::string> vars; // loop variable(s) — supports tuple unpacking
    ASTNodePtr iterable;
    ASTNodePtr condition; // optional if-filter (may be null)
};

struct TupleLiteral
{
    std::vector<ASTNodePtr> elements;
};

struct BlockStmt
{
    std::vector<ASTNodePtr> statements;
};

struct ExprStmt
{
    ASTNodePtr expr;
};

struct PrintStmt
{
    std::vector<ASTNodePtr> args;
    bool newline;
};

struct InputStmt
{
    std::string target;
    ASTNodePtr prompt;
};

struct BreakStmt
{
};
struct ContinueStmt
{
};
struct RaiseStmt
{
    ASTNodePtr value; // the exception value/message
};

struct ExceptClause
{
    std::string errorType; // e.g. "ValueError" — empty = bare except
    std::string alias;     // "as e" — empty if none
    std::shared_ptr<ASTNode> body;
};

struct TryStmt
{
    std::shared_ptr<ASTNode> body;
    std::vector<ExceptClause> handlers;
    std::shared_ptr<ASTNode> finallyBody; // may be null
};

struct ImportStmt
{
    std::string module;
    std::string alias; // optional
};

struct ClassDecl
{
    std::string name;
    std::string base; // optional
    std::vector<ASTNodePtr> methods;
    std::vector<ASTNodePtr> fields;
};

// ─── ASTNode variant ─────────────────────────────────────────────────────────

using NodeVariant = std::variant<
    NumberLiteral, StringLiteral, BoolLiteral, NilLiteral,
    Identifier,
    BinaryExpr, UnaryExpr, AssignExpr,
    CallExpr, IndexExpr, SliceExpr, MemberExpr,
    ArrayLiteral, DictLiteral, LambdaExpr, ListComp, TupleLiteral,
    VarDecl, FunctionDecl, ReturnStmt,
    IfStmt, WhileStmt, ForStmt,
    BlockStmt, ExprStmt,
    PrintStmt, InputStmt,
    BreakStmt, ContinueStmt,
    RaiseStmt, TryStmt,
    ImportStmt, ClassDecl,
    TernaryExpr>;

struct ASTNode
{
    NodeVariant node;
    int line = 0;

    template <typename T>
    ASTNode(T &&n, int ln = 0) : node(std::forward<T>(n)), line(ln) {}

    template <typename T>
    T &as() { return std::get<T>(node); }

    template <typename T>
    const T &as() const { return std::get<T>(node); }

    template <typename T>
    bool is() const { return std::holds_alternative<T>(node); }
};