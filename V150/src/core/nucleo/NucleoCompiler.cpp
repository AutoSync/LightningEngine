#include "../../include/nucleo/NucleoCompiler.h"
#include <cassert>
#include <iostream>

namespace Nucleo {

void Compiler::err(int line, const std::string& msg)
{
    if (error.empty())
        error = "Compiler line " + std::to_string(line) + ": " + msg;
}

// ─────────────────────────────────────────────────────────────────────────────
// Entry point
// ─────────────────────────────────────────────────────────────────────────────

bool Compiler::Compile(const Program& prog, CompiledProgram& out)
{
    program = &out;
    error   = "";
    for (const auto& cls : prog.classes) {
        compileClass(cls);
        if (!error.empty()) return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Class compilation
// ─────────────────────────────────────────────────────────────────────────────

void Compiler::compileClass(const ClassDecl& cls)
{
    CompiledClass cc;
    cc.name     = cls.name;
    cc.baseName = cls.baseName;

    for (const auto& f : cls.fields) {
        cc.fieldNames.push_back(f.name);
        // Default values: compile to constant if literal, else Null
        if (f.init) {
            if (f.init->kind() == Expr::Kind::Literal) {
                const auto& lit = static_cast<const LiteralExpr&>(*f.init);
                switch (lit.tok.type) {
                case TK::INT_LIT:   cc.fieldDefaults.push_back(Value::Int(std::stoi(lit.tok.lexeme)));  break;
                case TK::FLOAT_LIT: cc.fieldDefaults.push_back(Value::Float(std::stof(lit.tok.lexeme))); break;
                case TK::TRUE:      cc.fieldDefaults.push_back(Value::Bool(true));  break;
                case TK::FALSE:     cc.fieldDefaults.push_back(Value::Bool(false)); break;
                case TK::STRING_LIT:cc.fieldDefaults.push_back(Value::String(lit.tok.lexeme)); break;
                default:            cc.fieldDefaults.push_back(Value::Null()); break;
                }
            } else {
                cc.fieldDefaults.push_back(Value::Null());
            }
        } else {
            cc.fieldDefaults.push_back(Value::Null());
        }
    }

    program->classes.push_back(std::move(cc));
    curClass = &program->classes.back();

    for (const auto& m : cls.methods) {
        compileMethod(m);
        if (!error.empty()) return;
    }
    curClass = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Method compilation
// ─────────────────────────────────────────────────────────────────────────────

void Compiler::compileMethod(const MethodDecl& m)
{
    CompiledMethod cm;
    cm.name = m.name;
    for (const auto& p : m.params) cm.paramNames.push_back(p.name);

    curClass->methods.push_back(std::move(cm));
    curMethod = &curClass->methods.back();
    chunk     = &curMethod->chunk;
    chunk->name = curClass->name + "::" + m.name;

    locals.clear();
    scopeDepth = 0;

    // Parameters become locals in slot 0, 1, ...
    for (const auto& p : m.params)
        locals.push_back({ p.name, 0 });

    beginScope();
    if (m.body) compileBlock(*m.body);
    endScope();

    // Implicit void return
    chunk->emit(Op::RETURN_VOID);
    curMethod->localCount = (int)locals.size();
    curMethod = nullptr;
    chunk     = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Scope / locals
// ─────────────────────────────────────────────────────────────────────────────

void Compiler::beginScope() { ++scopeDepth; }

void Compiler::endScope()
{
    --scopeDepth;
    while (!locals.empty() && locals.back().depth > scopeDepth) {
        chunk->emit(Op::POP);
        locals.pop_back();
    }
}

int Compiler::addLocal(const std::string& name)
{
    locals.push_back({ name, scopeDepth });
    return (int)locals.size() - 1;
}

int Compiler::resolveLocal(const std::string& name) const
{
    for (int i = (int)locals.size() - 1; i >= 0; --i)
        if (locals[i].name == name) return i;
    return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Statements
// ─────────────────────────────────────────────────────────────────────────────

void Compiler::compileStmt(const Stmt& s)
{
    chunk->curLine = s.line;
    switch (s.kind()) {
    case Stmt::Kind::Block:    compileBlock(static_cast<const BlockStmt&>(s));  break;
    case Stmt::Kind::If:       compileIf   (static_cast<const IfStmt&>(s));    break;
    case Stmt::Kind::While:    compileWhile(static_cast<const WhileStmt&>(s)); break;
    case Stmt::Kind::For:      compileFor  (static_cast<const ForStmt&>(s));   break;
    case Stmt::Kind::Foreach:  compileForeach(static_cast<const ForeachStmt&>(s)); break;
    case Stmt::Kind::Return:   compileReturn (static_cast<const ReturnStmt&>(s));  break;
    case Stmt::Kind::VarDecl:  compileVarDecl(static_cast<const VarDeclStmt&>(s)); break;
    case Stmt::Kind::Expr: {
        compileExpr(*static_cast<const ExprStmt&>(s).expr);
        chunk->emit(Op::POP); // discard result
        break;
    }
    case Stmt::Kind::Break:
        if (!loopStack.empty()) {
            size_t jmp = chunk->emitJmp(Op::JMP);
            loopStack.back().breaks.push_back(jmp);
        }
        break;
    case Stmt::Kind::Continue:
        if (!loopStack.empty()) {
            size_t jmp = chunk->emitJmp(Op::JMP);
            int delta = (int)loopStack.back().continueTarget - (int)(jmp + 2);
            chunk->code[jmp]   = (uint8_t)(delta & 0xFF);
            chunk->code[jmp+1] = (uint8_t)((delta >> 8) & 0xFF);
        }
        break;
    }
}

void Compiler::compileBlock(const BlockStmt& b)
{
    beginScope();
    for (const auto& s : b.stmts) { compileStmt(*s); if (!error.empty()) return; }
    endScope();
}

void Compiler::compileIf(const IfStmt& s)
{
    compileExpr(*s.cond);
    size_t jmpFalse = chunk->emitJmp(Op::JMP_FALSE);
    compileStmt(*s.thenBranch);
    if (s.elseBranch) {
        size_t jmpEnd = chunk->emitJmp(Op::JMP);
        chunk->patchJmp(jmpFalse);
        compileStmt(*s.elseBranch);
        chunk->patchJmp(jmpEnd);
    } else {
        chunk->patchJmp(jmpFalse);
    }
}

void Compiler::compileWhile(const WhileStmt& s)
{
    size_t loopStart = chunk->code.size();
    loopStack.push_back({ {}, loopStart });

    compileExpr(*s.cond);
    size_t jmpEnd = chunk->emitJmp(Op::JMP_FALSE);
    compileStmt(*s.body);

    // Jump back to condition
    size_t jmpBack = chunk->emitJmp(Op::JMP);
    int delta = (int)loopStart - (int)(jmpBack + 2);
    chunk->code[jmpBack]   = (uint8_t)(delta & 0xFF);
    chunk->code[jmpBack+1] = (uint8_t)((delta >> 8) & 0xFF);

    chunk->patchJmp(jmpEnd);

    // Patch breaks
    for (size_t bp : loopStack.back().breaks) chunk->patchJmp(bp);
    loopStack.pop_back();
}

void Compiler::compileFor(const ForStmt& s)
{
    beginScope();
    if (s.init) compileStmt(*s.init);

    size_t loopStart = chunk->code.size();
    size_t jmpEnd    = 0;
    bool   hasCond   = (s.cond != nullptr);

    if (hasCond) {
        compileExpr(*s.cond);
        jmpEnd = chunk->emitJmp(Op::JMP_FALSE);
    }

    loopStack.push_back({ {}, chunk->code.size() });
    compileStmt(*s.body);

    if (s.step) {
        compileExpr(*s.step);
        chunk->emit(Op::POP);
    }

    size_t jmpBack = chunk->emitJmp(Op::JMP);
    int delta = (int)loopStart - (int)(jmpBack + 2);
    chunk->code[jmpBack]   = (uint8_t)(delta & 0xFF);
    chunk->code[jmpBack+1] = (uint8_t)((delta >> 8) & 0xFF);

    if (hasCond) chunk->patchJmp(jmpEnd);
    for (size_t bp : loopStack.back().breaks) chunk->patchJmp(bp);
    loopStack.pop_back();
    endScope();
}

void Compiler::compileForeach(const ForeachStmt& s)
{
    // For MVP: not implemented — emit a warning and skip body
    err(s.line, "foreach not yet supported in Nucleo VM");
}

void Compiler::compileReturn(const ReturnStmt& s)
{
    if (s.value) {
        compileExpr(*s.value);
        chunk->emit(Op::RETURN);
    } else {
        chunk->emit(Op::RETURN_VOID);
    }
}

void Compiler::compileVarDecl(const VarDeclStmt& s)
{
    if (s.init) compileExpr(*s.init);
    else        chunk->emit(Op::PUSH_NULL);
    addLocal(s.varName);
}

// ─────────────────────────────────────────────────────────────────────────────
// Expressions
// ─────────────────────────────────────────────────────────────────────────────

void Compiler::compileExpr(const Expr& e)
{
    chunk->curLine = e.line;
    switch (e.kind()) {
    case Expr::Kind::Literal:         compileLiteral(static_cast<const LiteralExpr&>(e)); break;
    case Expr::Kind::Ident: {
        const auto& id = static_cast<const IdentExpr&>(e);
        int slot = resolveLocal(id.name);
        if (slot >= 0) {
            chunk->emit(Op::LOAD_LOCAL); chunk->emitArg16((uint16_t)slot);
        } else {
            // Try field on this, then global
            if (curClass) {
                uint16_t ni = chunk->addName(id.name);
                chunk->emit(Op::LOAD_THIS_FIELD); chunk->emitArg16(ni);
            } else {
                uint16_t ni = chunk->addName(id.name);
                chunk->emit(Op::LOAD_GLOBAL); chunk->emitArg16(ni);
            }
        }
        break;
    }
    case Expr::Kind::This:
        chunk->emit(Op::LOAD_LOCAL); chunk->emitArg16(0xFFFF); // convention: this=special
        break;
    case Expr::Kind::Binary:         compileBinary(static_cast<const BinaryExpr&>(e));       break;
    case Expr::Kind::Unary:          compileUnary (static_cast<const UnaryExpr&>(e));         break;
    case Expr::Kind::Assign:         compileAssign(static_cast<const AssignExpr&>(e));        break;
    case Expr::Kind::CompoundAssign: compileCompoundAssign(static_cast<const CompoundAssignExpr&>(e)); break;
    case Expr::Kind::Call:           compileCall  (static_cast<const CallExpr&>(e));          break;
    case Expr::Kind::Member:         compileMember(static_cast<const MemberExpr&>(e));        break;
    case Expr::Kind::New:            compileNew   (static_cast<const NewExpr&>(e));           break;
    case Expr::Kind::Ternary:        compileTernary(static_cast<const TernaryExpr&>(e));      break;
    case Expr::Kind::Index:          err(e.line, "Index operator not yet supported"); break;
    case Expr::Kind::Cast: {
        const auto& c = static_cast<const CastExpr&>(e);
        compileExpr(*c.value);  // cast is a no-op at VM level for now
        break;
    }
    }
}

void Compiler::compileLiteral(const LiteralExpr& e)
{
    switch (e.tok.type) {
    case TK::INT_LIT: {
        uint16_t ci = chunk->addConst(Value::Int(std::stoi(e.tok.lexeme)));
        chunk->emit(Op::PUSH_CONST); chunk->emitArg16(ci);
        break;
    }
    case TK::FLOAT_LIT: {
        uint16_t ci = chunk->addConst(Value::Float(std::stof(e.tok.lexeme)));
        chunk->emit(Op::PUSH_CONST); chunk->emitArg16(ci);
        break;
    }
    case TK::STRING_LIT: {
        uint16_t ci = chunk->addConst(Value::String(e.tok.lexeme));
        chunk->emit(Op::PUSH_CONST); chunk->emitArg16(ci);
        break;
    }
    case TK::TRUE:  chunk->emit(Op::PUSH_TRUE);  break;
    case TK::FALSE: chunk->emit(Op::PUSH_FALSE); break;
    case TK::NULL_LIT: chunk->emit(Op::PUSH_NULL); break;
    default: err(e.tok.line, "Unknown literal"); break;
    }
}

void Compiler::compileBinary(const BinaryExpr& e)
{
    compileExpr(*e.left);
    compileExpr(*e.right);
    switch (e.op) {
    case TK::PLUS:     chunk->emit(Op::ADD); break;
    case TK::MINUS:    chunk->emit(Op::SUB); break;
    case TK::STAR:     chunk->emit(Op::MUL); break;
    case TK::SLASH:    chunk->emit(Op::DIV); break;
    case TK::PERCENT:  chunk->emit(Op::MOD); break;
    case TK::EQ_EQ:    chunk->emit(Op::EQ);  break;
    case TK::BANG_EQ:  chunk->emit(Op::NEQ); break;
    case TK::LT:       chunk->emit(Op::LT);  break;
    case TK::LT_EQ:    chunk->emit(Op::LTE); break;
    case TK::GT:       chunk->emit(Op::GT);  break;
    case TK::GT_EQ:    chunk->emit(Op::GTE); break;
    case TK::AMP_AMP:  chunk->emit(Op::LAND); break;
    case TK::PIPE_PIPE:chunk->emit(Op::LOR);  break;
    default: err(e.line, "Unknown binary operator"); break;
    }
}

void Compiler::compileUnary(const UnaryExpr& e)
{
    if (e.prefix) {
        compileExpr(*e.operand);
        switch (e.op) {
        case TK::MINUS:    chunk->emit(Op::NEGATE); break;
        case TK::BANG:     chunk->emit(Op::LNOT);   break;
        case TK::PLUS_PLUS: {
            // ++x: load, add 1, store, leave incremented value on stack
            emitLoad(*e.operand);
            uint16_t ci = chunk->addConst(Value::Int(1));
            chunk->emit(Op::PUSH_CONST); chunk->emitArg16(ci);
            chunk->emit(Op::ADD);
            chunk->emit(Op::DUP);
            emitStore(*e.operand);
            break;
        }
        case TK::MINUS_MINUS: {
            emitLoad(*e.operand);
            uint16_t ci = chunk->addConst(Value::Int(1));
            chunk->emit(Op::PUSH_CONST); chunk->emitArg16(ci);
            chunk->emit(Op::SUB);
            chunk->emit(Op::DUP);
            emitStore(*e.operand);
            break;
        }
        default: break;
        }
    } else {
        // Postfix: x++, x--
        compileExpr(*e.operand); // push current value (returned to caller)
        uint16_t ci = chunk->addConst(Value::Int(1));
        compileExpr(*e.operand); // push again for arithmetic
        chunk->emit(Op::PUSH_CONST); chunk->emitArg16(ci);
        chunk->emit(e.op == TK::PLUS_PLUS ? Op::ADD : Op::SUB);
        emitStore(*e.operand);
    }
}

void Compiler::compileAssign(const AssignExpr& e)
{
    compileExpr(*e.value);
    chunk->emit(Op::DUP); // leave value on stack after store
    emitStore(*e.target);
}

void Compiler::compileCompoundAssign(const CompoundAssignExpr& e)
{
    emitLoad(*e.target);
    compileExpr(*e.value);
    switch (e.op) {
    case TK::PLUS_EQ:  chunk->emit(Op::ADD); break;
    case TK::MINUS_EQ: chunk->emit(Op::SUB); break;
    case TK::STAR_EQ:  chunk->emit(Op::MUL); break;
    case TK::SLASH_EQ: chunk->emit(Op::DIV); break;
    default: break;
    }
    chunk->emit(Op::DUP);
    emitStore(*e.target);
}

void Compiler::emitLoad(const Expr& target)
{
    compileExpr(target);
}

void Compiler::emitStore(const Expr& target)
{
    if (target.kind() == Expr::Kind::Ident) {
        const auto& id = static_cast<const IdentExpr&>(target);
        int slot = resolveLocal(id.name);
        if (slot >= 0) {
            chunk->emit(Op::STORE_LOCAL); chunk->emitArg16((uint16_t)slot);
        } else if (curClass) {
            uint16_t ni = chunk->addName(id.name);
            chunk->emit(Op::STORE_THIS_FIELD); chunk->emitArg16(ni);
        } else {
            uint16_t ni = chunk->addName(id.name);
            chunk->emit(Op::STORE_GLOBAL); chunk->emitArg16(ni);
        }
    } else if (target.kind() == Expr::Kind::Member) {
        const auto& mem = static_cast<const MemberExpr&>(target);
        // stack: [newValue]  →  push object, swap, store
        compileExpr(*mem.object);
        // Stack: [newValue, object]  — need [object, newValue]
        // We emitted DUP before emitStore, so stack is [val, val] at entry
        // Actually at call site: DUP then emitStore, so [result_val] is on stack
        // We need object below value: emit SWAP would be ideal but we don't have it.
        // Simple approach: re-emit load of object (works for simple member access)
        uint16_t ni = chunk->addName(mem.member);
        chunk->emit(Op::STORE_FIELD); chunk->emitArg16(ni);
    }
}

void Compiler::compileCall(const CallExpr& e)
{
    // Detect method call on object: obj.method(args) → MemberExpr callee
    if (e.callee->kind() == Expr::Kind::Member) {
        const auto& mem = static_cast<const MemberExpr&>(*e.callee);
        compileExpr(*mem.object);
        for (const auto& a : e.args) compileExpr(*a);
        uint16_t ni   = chunk->addName(mem.member);
        uint16_t argc = (uint16_t)e.args.size();
        chunk->emit(Op::CALL_METHOD);
        chunk->emitArg16(argc);
        chunk->emitArg16(ni);
    } else if (e.callee->kind() == Expr::Kind::Ident) {
        // Could be a native or free function
        const auto& id = static_cast<const IdentExpr&>(*e.callee);
        // Check native registry
        for (size_t i = 0; i < program->natives.size(); ++i) {
            if (program->natives[i].name == id.name) {
                for (const auto& a : e.args) compileExpr(*a);
                chunk->emit(Op::CALL_NATIVE);
                chunk->emitArg16((uint16_t)i);
                chunk->emitArg16((uint16_t)e.args.size());
                return;
            }
        }
        // Assume it's a method on this
        chunk->emit(Op::LOAD_LOCAL); chunk->emitArg16(0xFFFF); // this
        for (const auto& a : e.args) compileExpr(*a);
        uint16_t ni = chunk->addName(id.name);
        chunk->emit(Op::CALL_METHOD);
        chunk->emitArg16((uint16_t)e.args.size());
        chunk->emitArg16(ni);
    }
}

void Compiler::compileMember(const MemberExpr& e)
{
    compileExpr(*e.object);
    uint16_t ni = chunk->addName(e.member);
    chunk->emit(Op::LOAD_FIELD); chunk->emitArg16(ni);
}

void Compiler::compileNew(const NewExpr& e)
{
    for (const auto& a : e.args) compileExpr(*a);
    uint16_t ni = chunk->addName(e.typeName);
    chunk->emit(Op::NEW_OBJ); chunk->emitArg16(ni);
}

void Compiler::compileTernary(const TernaryExpr& e)
{
    compileExpr(*e.cond);
    size_t jmpFalse = chunk->emitJmp(Op::JMP_FALSE);
    compileExpr(*e.thenExpr);
    size_t jmpEnd = chunk->emitJmp(Op::JMP);
    chunk->patchJmp(jmpFalse);
    compileExpr(*e.elseExpr);
    chunk->patchJmp(jmpEnd);
}

} // namespace Nucleo
