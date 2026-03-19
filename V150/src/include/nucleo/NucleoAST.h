// NucleoAST.h — Abstract Syntax Tree nodes for the Núcleo parser.
// Expressions and statements use the visitor pattern for traversal.
#pragma once
#include "NucleoToken.h"
#include <vector>
#include <memory>
#include <string>

namespace Nucleo {

// ── Forward declarations ──────────────────────────────────────────────────────
struct Expr;
struct Stmt;
struct Decl;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

// ─────────────────────────────────────────────────────────────────────────────
// Expressions
// ─────────────────────────────────────────────────────────────────────────────

struct Expr {
    int line = 0;
    virtual ~Expr() = default;
    enum class Kind {
        Literal, Ident, Binary, Unary, Assign, CompoundAssign,
        Call, Member, Index, New, Cast, Ternary, This
    };
    virtual Kind kind() const = 0;
};

struct LiteralExpr : Expr {
    Token tok;
    Kind kind() const override { return Kind::Literal; }
};

struct IdentExpr : Expr {
    std::string name;
    Kind kind() const override { return Kind::Ident; }
};

struct ThisExpr : Expr {
    Kind kind() const override { return Kind::This; }
};

struct BinaryExpr : Expr {
    TK       op;
    ExprPtr  left, right;
    Kind kind() const override { return Kind::Binary; }
};

struct UnaryExpr : Expr {
    TK      op;
    ExprPtr operand;
    bool    prefix = true;  // false = postfix (a++, a--)
    Kind kind() const override { return Kind::Unary; }
};

struct AssignExpr : Expr {
    ExprPtr target, value;
    Kind kind() const override { return Kind::Assign; }
};

struct CompoundAssignExpr : Expr {
    TK      op;    // PLUS_EQ, MINUS_EQ, STAR_EQ, SLASH_EQ
    ExprPtr target, value;
    Kind kind() const override { return Kind::CompoundAssign; }
};

struct CallExpr : Expr {
    ExprPtr                  callee;
    std::vector<ExprPtr>     args;
    Kind kind() const override { return Kind::Call; }
};

struct MemberExpr : Expr {
    ExprPtr     object;
    std::string member;
    Kind kind() const override { return Kind::Member; }
};

struct IndexExpr : Expr {
    ExprPtr object, index;
    Kind kind() const override { return Kind::Index; }
};

struct NewExpr : Expr {
    std::string          typeName;
    std::vector<ExprPtr> args;
    Kind kind() const override { return Kind::New; }
};

struct CastExpr : Expr {
    std::string typeName;
    ExprPtr     value;
    Kind kind() const override { return Kind::Cast; }
};

struct TernaryExpr : Expr {
    ExprPtr cond, thenExpr, elseExpr;
    Kind kind() const override { return Kind::Ternary; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Statements
// ─────────────────────────────────────────────────────────────────────────────

struct Stmt {
    int line = 0;
    virtual ~Stmt() = default;
    enum class Kind {
        Expr, VarDecl, Block, If, While, For, Foreach,
        Return, Break, Continue
    };
    virtual Kind kind() const = 0;
};

struct ExprStmt : Stmt {
    ExprPtr expr;
    Kind kind() const override { return Kind::Expr; }
};

struct VarDeclStmt : Stmt {
    std::string typeName;   // "int", "float", "bool", "string", or class name
    std::string varName;
    ExprPtr     init;       // may be null
    Kind kind() const override { return Kind::VarDecl; }
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> stmts;
    Kind kind() const override { return Kind::Block; }
};

struct IfStmt : Stmt {
    ExprPtr cond;
    StmtPtr thenBranch;
    StmtPtr elseBranch;  // may be null
    Kind kind() const override { return Kind::If; }
};

struct WhileStmt : Stmt {
    ExprPtr cond;
    StmtPtr body;
    Kind kind() const override { return Kind::While; }
};

struct ForStmt : Stmt {
    StmtPtr init;    // VarDeclStmt or ExprStmt or null
    ExprPtr cond;    // may be null
    ExprPtr step;    // may be null
    StmtPtr body;
    Kind kind() const override { return Kind::For; }
};

struct ForeachStmt : Stmt {
    std::string varType, varName;
    ExprPtr     collection;
    StmtPtr     body;
    Kind kind() const override { return Kind::Foreach; }
};

struct ReturnStmt : Stmt {
    ExprPtr value;   // may be null (void return)
    Kind kind() const override { return Kind::Return; }
};

struct BreakStmt    : Stmt { Kind kind() const override { return Kind::Break; } };
struct ContinueStmt : Stmt { Kind kind() const override { return Kind::Continue; } };

// ─────────────────────────────────────────────────────────────────────────────
// Declarations (class-level)
// ─────────────────────────────────────────────────────────────────────────────

struct Param {
    std::string typeName;
    std::string name;
};

struct FieldDecl {
    std::string  access;    // "public", "private", "protected"
    bool         isStatic = false;
    std::string  typeName;
    std::string  name;
    ExprPtr      init;
};

struct MethodDecl {
    std::string          access;
    bool                 isStatic   = false;
    bool                 isOverride = false;
    std::string          retType;
    std::string          name;
    std::vector<Param>   params;
    std::unique_ptr<BlockStmt> body;
};

struct ClassDecl {
    std::string               name;
    std::string               baseName;   // "" if no base
    std::vector<FieldDecl>    fields;
    std::vector<MethodDecl>   methods;
};

struct Program {
    std::vector<ClassDecl> classes;
};

} // namespace Nucleo
