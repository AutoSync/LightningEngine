// UndoStack.h — Simple command-pattern undo/redo for the editor.
//
// Usage:
//   struct SetNameCmd : public Command { ... };
//   undoStack.Do(std::make_unique<SetNameCmd>(...));
//   undoStack.Undo();
//   undoStack.Redo();
//
// Each Command must implement Execute() and Undo().
// Execute() is called immediately by Do().
// The stack is capped at kMaxDepth (100) operations.
#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <string>

// ── Command interface ─────────────────────────────────────────────────────────
struct Command {
    virtual ~Command() = default;
    virtual void Execute() = 0;
    virtual void Undo()    = 0;
    virtual const char* Label() const { return "Action"; }
};

// ── Lambda-based command (quick one-liners) ───────────────────────────────────
struct LambdaCommand : Command {
    std::function<void()> exec;
    std::function<void()> undo;
    std::string           label;

    LambdaCommand(std::function<void()> e, std::function<void()> u,
                  const char* lbl = "Action")
        : exec(std::move(e)), undo(std::move(u)), label(lbl) {}

    void Execute() override { if (exec) exec(); }
    void Undo()    override { if (undo) undo(); }
    const char* Label() const override { return label.c_str(); }
};

// ── Stack ─────────────────────────────────────────────────────────────────────
class UndoStack {
public:
    static constexpr int kMaxDepth = 100;

    // Execute cmd and push it onto the undo stack.
    // Discards any redo history.
    void Do(std::unique_ptr<Command> cmd)
    {
        cmd->Execute();
        history.push_back(std::move(cmd));
        if ((int)history.size() > kMaxDepth)
            history.erase(history.begin());
        redoStack.clear();
    }

    // Convenience: Lambda version.
    void Do(std::function<void()> exec, std::function<void()> undo,
            const char* label = "Action")
    {
        Do(std::make_unique<LambdaCommand>(std::move(exec), std::move(undo), label));
    }

    // Record a command that has ALREADY been executed (no re-execution).
    // `exec` is the redo function; `undo` is the undo function.
    void Push(std::function<void()> exec, std::function<void()> undo,
              const char* label = "Action")
    {
        history.push_back(std::make_unique<LambdaCommand>(
            std::move(exec), std::move(undo), label));
        if ((int)history.size() > kMaxDepth)
            history.erase(history.begin());
        redoStack.clear();
    }

    bool CanUndo() const { return !history.empty(); }
    bool CanRedo() const { return !redoStack.empty(); }

    void Undo()
    {
        if (history.empty()) return;
        auto& cmd = history.back();
        cmd->Undo();
        redoStack.push_back(std::move(cmd));
        history.pop_back();
    }

    void Redo()
    {
        if (redoStack.empty()) return;
        auto& cmd = redoStack.back();
        cmd->Execute();
        history.push_back(std::move(cmd));
        redoStack.pop_back();
    }

    void Clear() { history.clear(); redoStack.clear(); }

    const char* UndoLabel() const
    { return history.empty()   ? nullptr : history.back()->Label(); }
    const char* RedoLabel() const
    { return redoStack.empty() ? nullptr : redoStack.back()->Label(); }

private:
    std::vector<std::unique_ptr<Command>> history;
    std::vector<std::unique_ptr<Command>> redoStack;
};
