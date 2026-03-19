#include "../../include/nucleo/NucleoParser.h"
#include <cassert>

namespace Nucleo {

// ─────────────────────────────────────────────────────────────────────────────
// Token navigation
// ─────────────────────────────────────────────────────────────────────────────

static const Token kEOF = { TK::END_OF_FILE, "", 0 };

const Token& Parser::peek(int offset) const
{
    size_t i = cur + offset;
    return i < toks.size() ? toks[i] : kEOF;
}

const Token& Parser::advance()
{
    if (!atEnd()) ++cur;
    return toks[cur - 1];
}

bool Parser::check(TK t, int offset) const { return peek(offset).type == t; }

bool Parser::match(TK t)
{
    if (check(t)) { advance(); return true; }
    return false;
}

bool Parser::matchAny(std::initializer_list<TK> ts)
{
    for (TK t : ts) if (match(t)) return true;
    return false;
}

const Token& Parser::expect(TK t, const char* msg)
{
    if (!check(t)) { err(std::string(msg) + " (got \"" + peek().lexeme + "\")"); }
    return advance();
}

bool Parser::atEnd() const { return cur >= toks.size() || toks[cur].is(TK::END_OF_FILE); }

void Parser::err(const std::string& msg)
{
    if (error.empty())
        error = "Line " + std::to_string(peek().line) + ": " + msg;
}

// ─────────────────────────────────────────────────────────────────────────────
// Top-level parse
// ─────────────────────────────────────────────────────────────────────────────

Program Parser::Parse(const std::vector<Token>& tokens)
{
    toks  = tokens;
    cur   = 0;
    error = "";
    Program prog;
    while (!atEnd() && error.empty()) {
        // Skip stray semicolons
        if (match(TK::SEMICOLON)) continue;
        if (check(TK::KW_CLASS)) {
            prog.classes.push_back(parseClass());
        } else {
            err("Expected class declaration");
            break;
        }
    }
    return prog;
}

ClassDecl Parser::parseClass()
{
    ClassDecl cls;
    expect(TK::KW_CLASS, "Expected 'class'");
    cls.name = expect(TK::IDENT, "Expected class name").lexeme;
    if (match(TK::COLON))
        cls.baseName = expect(TK::IDENT, "Expected base class name").lexeme;
    expect(TK::LBRACE, "Expected '{' after class declaration");

    while (!check(TK::RBRACE) && !atEnd() && error.empty()) {
        // Modifiers
        std::string access   = "private";
        bool        isStatic = false, isOverride = false;
        if (check(TK::KW_PUBLIC))    { access = "public";    advance(); }
        else if (check(TK::KW_PRIVATE))   { access = "private";   advance(); }
        else if (check(TK::KW_PROTECTED)) { access = "protected"; advance(); }
        if (check(TK::KW_STATIC))   { isStatic   = true; advance(); }
        if (check(TK::KW_OVERRIDE))  { isOverride = true; advance(); }

        // Return/type and name
        std::string typeOrRet = peek().lexeme; advance();
        std::string memberName = expect(TK::IDENT, "Expected member name").lexeme;
        int ln = peek().line;

        if (check(TK::LPAREN)) {
            cls.methods.push_back(
                parseMethod(access, isStatic, isOverride, typeOrRet, memberName, ln));
        } else {
            cls.fields.push_back(
                parseField(access, isStatic, typeOrRet, memberName));
        }
    }
    expect(TK::RBRACE, "Expected '}' to close class");
    return cls;
}

FieldDecl Parser::parseField(const std::string& access, bool isStatic,
                              const std::string& type, const std::string& name)
{
    FieldDecl f;
    f.access = access; f.isStatic = isStatic;
    f.typeName = type; f.name = name;
    if (match(TK::EQ)) f.init = parseExpr();
    expect(TK::SEMICOLON, "Expected ';' after field declaration");
    return f;
}

MethodDecl Parser::parseMethod(const std::string& access, bool isStatic,
                                bool isOverride, const std::string& ret,
                                const std::string& name, int ln)
{
    MethodDecl m;
    m.access = access; m.isStatic = isStatic; m.isOverride = isOverride;
    m.retType = ret;   m.name = name;
    m.params  = parseParams();
    auto body = parseBlock();
    m.body.reset(static_cast<BlockStmt*>(body.release()));
    return m;
}

std::vector<Param> Parser::parseParams()
{
    std::vector<Param> params;
    expect(TK::LPAREN, "Expected '(' for parameter list");
    if (!check(TK::RPAREN)) {
        do {
            Param p;
            p.typeName = peek().lexeme; advance();
            p.name     = expect(TK::IDENT, "Expected parameter name").lexeme;
            params.push_back(std::move(p));
        } while (match(TK::COMMA));
    }
    expect(TK::RPAREN, "Expected ')' to close parameter list");
    return params;
}

// ─────────────────────────────────────────────────────────────────────────────
// Statements
// ─────────────────────────────────────────────────────────────────────────────

bool Parser::isTypeName(const std::string& s) const
{
    static const std::vector<std::string> kBuiltin = {
        "int","float","bool","string","void","var"
    };
    for (auto& b : kBuiltin) if (s == b) return true;
    // Heuristic: starts with uppercase = class name
    return !s.empty() && std::isupper((unsigned char)s[0]);
}

bool Parser::isVarDeclStart() const
{
    // type IDENT [= | ;]
    if (!check(TK::IDENT, 0) && !check(TK::KW_INT, 0) && !check(TK::KW_FLOAT, 0) &&
        !check(TK::KW_BOOL, 0) && !check(TK::KW_STRING, 0) && !check(TK::KW_VAR, 0))
        return false;
    if (!check(TK::IDENT, 1)) return false;
    TK next2 = peek(2).type;
    return next2 == TK::EQ || next2 == TK::SEMICOLON || next2 == TK::COMMA;
}

StmtPtr Parser::parseStmt()
{
    if (check(TK::LBRACE))     return parseBlock();
    if (check(TK::KW_IF))      return parseIf();
    if (check(TK::KW_WHILE))   return parseWhile();
    if (check(TK::KW_FOR))     return parseFor();
    if (check(TK::KW_FOREACH)) return parseForeach();
    if (check(TK::KW_RETURN))  return parseReturn();
    if (check(TK::KW_BREAK))   { advance(); expect(TK::SEMICOLON, "Expected ';'"); return std::make_unique<BreakStmt>(); }
    if (check(TK::KW_CONTINUE)){ advance(); expect(TK::SEMICOLON, "Expected ';'"); return std::make_unique<ContinueStmt>(); }

    // Variable declaration?
    if (isVarDeclStart()) {
        std::string type = peek().lexeme; advance();
        std::string name = peek().lexeme; advance();
        return parseVarDecl(type, name);
    }

    // Expression statement
    auto s = std::make_unique<ExprStmt>();
    s->line = peek().line;
    s->expr = parseExpr();
    expect(TK::SEMICOLON, "Expected ';' after expression");
    return s;
}

StmtPtr Parser::parseBlock()
{
    expect(TK::LBRACE, "Expected '{'");
    auto b = std::make_unique<BlockStmt>();
    while (!check(TK::RBRACE) && !atEnd() && error.empty())
        b->stmts.push_back(parseStmt());
    expect(TK::RBRACE, "Expected '}'");
    return b;
}

StmtPtr Parser::parseIf()
{
    advance(); // 'if'
    auto s = std::make_unique<IfStmt>();
    s->line = peek().line;
    expect(TK::LPAREN, "Expected '('");
    s->cond = parseExpr();
    expect(TK::RPAREN, "Expected ')'");
    s->thenBranch = parseStmt();
    if (match(TK::KW_ELSE)) s->elseBranch = parseStmt();
    return s;
}

StmtPtr Parser::parseWhile()
{
    advance(); // 'while'
    auto s = std::make_unique<WhileStmt>();
    s->line = peek().line;
    expect(TK::LPAREN, "Expected '('");
    s->cond = parseExpr();
    expect(TK::RPAREN, "Expected ')'");
    s->body = parseStmt();
    return s;
}

StmtPtr Parser::parseFor()
{
    advance(); // 'for'
    auto s = std::make_unique<ForStmt>();
    s->line = peek().line;
    expect(TK::LPAREN, "Expected '('");

    // Init
    if (!check(TK::SEMICOLON)) {
        if (isVarDeclStart()) {
            std::string type = peek().lexeme; advance();
            std::string name = peek().lexeme; advance();
            s->init = parseVarDecl(type, name);
        } else {
            auto es = std::make_unique<ExprStmt>();
            es->expr = parseExpr();
            expect(TK::SEMICOLON, "Expected ';'");
            s->init = std::move(es);
        }
    } else advance(); // ';'

    if (!check(TK::SEMICOLON)) s->cond = parseExpr();
    expect(TK::SEMICOLON, "Expected ';'");

    if (!check(TK::RPAREN)) s->step = parseExpr();
    expect(TK::RPAREN, "Expected ')'");

    s->body = parseStmt();
    return s;
}

StmtPtr Parser::parseForeach()
{
    advance(); // 'foreach'
    auto s = std::make_unique<ForeachStmt>();
    s->line = peek().line;
    expect(TK::LPAREN, "Expected '('");
    s->varType = peek().lexeme; advance();
    s->varName = expect(TK::IDENT, "Expected variable name").lexeme;
    expect(TK::KW_IN, "Expected 'in'");
    s->collection = parseExpr();
    expect(TK::RPAREN, "Expected ')'");
    s->body = parseStmt();
    return s;
}

StmtPtr Parser::parseReturn()
{
    advance(); // 'return'
    auto s = std::make_unique<ReturnStmt>();
    s->line = peek().line;
    if (!check(TK::SEMICOLON)) s->value = parseExpr();
    expect(TK::SEMICOLON, "Expected ';'");
    return s;
}

StmtPtr Parser::parseVarDecl(const std::string& type, const std::string& name)
{
    auto s = std::make_unique<VarDeclStmt>();
    s->typeName = type;
    s->varName  = name;
    if (match(TK::EQ)) s->init = parseExpr();
    expect(TK::SEMICOLON, "Expected ';' after variable declaration");
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// Expressions (precedence climbing)
// ─────────────────────────────────────────────────────────────────────────────

ExprPtr Parser::parseExpr()
{
    auto left = parseTernary();
    // Assignment
    if (check(TK::EQ)) {
        advance();
        auto a = std::make_unique<AssignExpr>();
        a->line = peek().line;
        a->target = std::move(left);
        a->value  = parseExpr();
        return a;
    }
    // Compound assignment
    if (check(TK::PLUS_EQ) || check(TK::MINUS_EQ) ||
        check(TK::STAR_EQ) || check(TK::SLASH_EQ)) {
        TK op = peek().type; advance();
        auto a = std::make_unique<CompoundAssignExpr>();
        a->line   = peek().line;
        a->op     = op;
        a->target = std::move(left);
        a->value  = parseExpr();
        return a;
    }
    return left;
}

ExprPtr Parser::parseTernary()
{
    auto cond = parseOr();
    if (match(TK::QUESTION)) {
        auto t = std::make_unique<TernaryExpr>();
        t->cond     = std::move(cond);
        t->thenExpr = parseExpr();
        expect(TK::COLON, "Expected ':' in ternary");
        t->elseExpr = parseExpr();
        return t;
    }
    return cond;
}

ExprPtr Parser::parseOr()
{
    auto left = parseAnd();
    while (check(TK::PIPE_PIPE)) {
        TK op = peek().type; advance();
        auto b = std::make_unique<BinaryExpr>();
        b->op = op; b->left = std::move(left); b->right = parseAnd();
        left  = std::move(b);
    }
    return left;
}

ExprPtr Parser::parseAnd()
{
    auto left = parseEquality();
    while (check(TK::AMP_AMP)) {
        TK op = peek().type; advance();
        auto b = std::make_unique<BinaryExpr>();
        b->op = op; b->left = std::move(left); b->right = parseEquality();
        left  = std::move(b);
    }
    return left;
}

ExprPtr Parser::parseEquality()
{
    auto left = parseRelational();
    while (check(TK::EQ_EQ) || check(TK::BANG_EQ)) {
        TK op = peek().type; advance();
        auto b = std::make_unique<BinaryExpr>();
        b->op = op; b->left = std::move(left); b->right = parseRelational();
        left  = std::move(b);
    }
    return left;
}

ExprPtr Parser::parseRelational()
{
    auto left = parseAddSub();
    while (check(TK::LT) || check(TK::LT_EQ) ||
           check(TK::GT) || check(TK::GT_EQ)) {
        TK op = peek().type; advance();
        auto b = std::make_unique<BinaryExpr>();
        b->op = op; b->left = std::move(left); b->right = parseAddSub();
        left  = std::move(b);
    }
    return left;
}

ExprPtr Parser::parseAddSub()
{
    auto left = parseMulDiv();
    while (check(TK::PLUS) || check(TK::MINUS)) {
        TK op = peek().type; advance();
        auto b = std::make_unique<BinaryExpr>();
        b->op = op; b->left = std::move(left); b->right = parseMulDiv();
        left  = std::move(b);
    }
    return left;
}

ExprPtr Parser::parseMulDiv()
{
    auto left = parseUnary();
    while (check(TK::STAR) || check(TK::SLASH) || check(TK::PERCENT)) {
        TK op = peek().type; advance();
        auto b = std::make_unique<BinaryExpr>();
        b->op = op; b->left = std::move(left); b->right = parseUnary();
        left  = std::move(b);
    }
    return left;
}

ExprPtr Parser::parseUnary()
{
    if (check(TK::BANG) || check(TK::MINUS) ||
        check(TK::PLUS_PLUS) || check(TK::MINUS_MINUS)) {
        TK op = peek().type; advance();
        auto u = std::make_unique<UnaryExpr>();
        u->op = op; u->operand = parseUnary(); u->prefix = true;
        return u;
    }
    auto expr = parsePrimary();
    return parsePostfix(std::move(expr));
}

ExprPtr Parser::parsePostfix(ExprPtr expr)
{
    while (true) {
        if (check(TK::DOT)) {
            advance();
            std::string member = expect(TK::IDENT, "Expected member name").lexeme;
            auto m = std::make_unique<MemberExpr>();
            m->object = std::move(expr); m->member = member;
            expr = std::move(m);
        } else if (check(TK::LPAREN)) {
            expr = parseCall(std::move(expr));
        } else if (check(TK::LBRACKET)) {
            advance();
            auto ix = std::make_unique<IndexExpr>();
            ix->object = std::move(expr); ix->index = parseExpr();
            expect(TK::RBRACKET, "Expected ']'");
            expr = std::move(ix);
        } else if (check(TK::PLUS_PLUS) || check(TK::MINUS_MINUS)) {
            TK op = peek().type; advance();
            auto u = std::make_unique<UnaryExpr>();
            u->op = op; u->operand = std::move(expr); u->prefix = false;
            expr = std::move(u);
        } else break;
    }
    return expr;
}

ExprPtr Parser::parseCall(ExprPtr callee)
{
    expect(TK::LPAREN, "Expected '('");
    auto call = std::make_unique<CallExpr>();
    call->line   = peek().line;
    call->callee = std::move(callee);
    if (!check(TK::RPAREN)) {
        do { call->args.push_back(parseExpr()); } while (match(TK::COMMA));
    }
    expect(TK::RPAREN, "Expected ')'");
    return call;
}

ExprPtr Parser::parsePrimary()
{
    int ln = peek().line;

    // Literals
    if (peek().isLiteral()) {
        auto lit = std::make_unique<LiteralExpr>();
        lit->line = ln; lit->tok = advance();
        return lit;
    }

    // this
    if (match(TK::KW_THIS)) {
        auto t = std::make_unique<ThisExpr>(); t->line = ln; return t;
    }

    // new
    if (match(TK::KW_NEW)) {
        auto n = std::make_unique<NewExpr>();
        n->line = ln;
        n->typeName = expect(TK::IDENT, "Expected type name after 'new'").lexeme;
        expect(TK::LPAREN, "Expected '('");
        if (!check(TK::RPAREN))
            do { n->args.push_back(parseExpr()); } while (match(TK::COMMA));
        expect(TK::RPAREN, "Expected ')'");
        return n;
    }

    // Parenthesised expression or cast
    if (match(TK::LPAREN)) {
        // Cast: (TypeName) expr  — heuristic: (IDENT)
        if (check(TK::IDENT) && check(TK::RPAREN, 1)) {
            std::string typeName = peek().lexeme; advance(); advance();
            auto c = std::make_unique<CastExpr>();
            c->line = ln; c->typeName = typeName; c->value = parseUnary();
            return c;
        }
        auto inner = parseExpr();
        expect(TK::RPAREN, "Expected ')'");
        return inner;
    }

    // Identifier
    if (check(TK::IDENT)) {
        auto id = std::make_unique<IdentExpr>();
        id->line = ln; id->name = advance().lexeme;
        return id;
    }

    // Keyword identifiers used as names (e.g. type names in expressions)
    if (!atEnd()) {
        auto id = std::make_unique<IdentExpr>();
        id->line = ln; id->name = advance().lexeme;
        return id;
    }

    err("Unexpected token '" + peek().lexeme + "' in expression");
    return std::make_unique<LiteralExpr>(); // stub
}

} // namespace Nucleo
