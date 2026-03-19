// NucleoParser.h — Recursive descent parser. Tokens → AST.
#pragma once
#include "NucleoToken.h"
#include "NucleoAST.h"
#include <vector>
#include <string>

namespace Nucleo {

class Parser {
public:
    // Parse a token stream produced by Lexer::Tokenise().
    // Returns the root Program, or an empty one on error.
    Program Parse(const std::vector<Token>& tokens);

    const std::string& Error() const { return error; }
    bool HasError()            const { return !error.empty(); }

private:
    std::vector<Token> toks;
    size_t             cur   = 0;
    std::string        error;

    // ── Token navigation ─────────────────────────────────────────────────────
    const Token& peek(int offset = 0) const;
    const Token& advance();
    bool         check(TK t, int offset = 0) const;
    bool         match(TK t);
    bool         matchAny(std::initializer_list<TK> ts);
    const Token& expect(TK t, const char* msg);
    bool         atEnd() const;

    // ── Top-level ────────────────────────────────────────────────────────────
    ClassDecl  parseClass();
    FieldDecl  parseField(const std::string& access, bool isStatic,
                          const std::string& type, const std::string& name);
    MethodDecl parseMethod(const std::string& access, bool isStatic,
                           bool isOverride, const std::string& ret,
                           const std::string& name, int ln);
    std::vector<Param> parseParams();

    // ── Statements ───────────────────────────────────────────────────────────
    StmtPtr parseStmt();
    StmtPtr parseBlock();
    StmtPtr parseIf();
    StmtPtr parseWhile();
    StmtPtr parseFor();
    StmtPtr parseForeach();
    StmtPtr parseReturn();
    StmtPtr parseVarDecl(const std::string& type, const std::string& name);

    bool isTypeName(const std::string& s) const;
    bool isVarDeclStart() const;

    // ── Expressions (precedence climbing) ────────────────────────────────────
    ExprPtr parseExpr();
    ExprPtr parseTernary();
    ExprPtr parseOr();
    ExprPtr parseAnd();
    ExprPtr parseEquality();
    ExprPtr parseRelational();
    ExprPtr parseAddSub();
    ExprPtr parseMulDiv();
    ExprPtr parseUnary();
    ExprPtr parsePostfix(ExprPtr expr);
    ExprPtr parsePrimary();
    ExprPtr parseCall(ExprPtr callee);

    void err(const std::string& msg);
};

} // namespace Nucleo
