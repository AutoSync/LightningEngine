// NucleoCompiler.h — AST → Bytecode compiler.
#pragma once
#include "NucleoAST.h"
#include "NucleoBytecode.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace Nucleo {

class Compiler {
public:
    // Compile a parsed Program into a CompiledProgram.
    // Returns true on success. Error details in Error().
    bool Compile(const Program& prog, CompiledProgram& out);

    const std::string& Error() const { return error; }

private:
    CompiledProgram* program = nullptr;
    CompiledClass*   curClass  = nullptr;
    CompiledMethod*  curMethod = nullptr;
    Chunk*           chunk     = nullptr;
    std::string      error;

    // Local variable scope stack
    struct Local { std::string name; int depth; };
    std::vector<Local> locals;
    int                scopeDepth = 0;

    // Loop context for break/continue
    struct LoopCtx { std::vector<size_t> breaks; size_t continueTarget; };
    std::vector<LoopCtx> loopStack;

    void compileClass (const ClassDecl&  cls);
    void compileMethod(const MethodDecl& m);

    void compileStmt  (const Stmt& s);
    void compileBlock (const BlockStmt& b);
    void compileIf    (const IfStmt& s);
    void compileWhile (const WhileStmt& s);
    void compileFor   (const ForStmt& s);
    void compileForeach(const ForeachStmt& s);
    void compileReturn(const ReturnStmt& s);
    void compileVarDecl(const VarDeclStmt& s);

    void  compileExpr (const Expr& e);
    void  compileLiteral(const LiteralExpr& e);
    void  compileBinary (const BinaryExpr& e);
    void  compileUnary  (const UnaryExpr& e);
    void  compileAssign (const AssignExpr& e);
    void  compileCompoundAssign(const CompoundAssignExpr& e);
    void  compileCall   (const CallExpr& e);
    void  compileMember (const MemberExpr& e);
    void  compileNew    (const NewExpr& e);
    void  compileTernary(const TernaryExpr& e);

    // Emit an l-value load (returns true) or store (returns false)
    void emitLoad (const Expr& target);
    void emitStore(const Expr& target);

    // Locals management
    void  beginScope();
    void  endScope();
    int   addLocal(const std::string& name);
    int   resolveLocal(const std::string& name) const;

    void  err(int line, const std::string& msg);
};

} // namespace Nucleo
