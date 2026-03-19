// GameInstance — Abstract base class for user games.
// Inherits the full engine lifecycle from Window.
// Owns an active Level and manages level transitions.
//
// CLASS HIERARCHY:
// WINDOW -> [ GAMEINSTANCE ] -> LEVEL -> NODES -> COMPONENTS
//
// Usage:
//   class MyGame : public GameInstance {
//   public:
//       void Initialize() override {
//           LoadLevel<MainLevel>();
//       }
//       void Shutdown()         override {}
//       void Update(float dt)   override { UpdateLevel(dt); }
//       void Render()           override { RenderLevel(); }
//   };

#pragma once
#include "Window.h"
#include "Level.h"
#include <memory>

namespace LightningEngine {

class GameInstance : public Window {
public:
    virtual ~GameInstance() = default;

    // Returns the currently active Level, or nullptr if none is loaded.
    Level* GetLevel() const { return activeLevel.get(); }

protected:
    // -----------------------------------------------------------------------
    // Level management
    // -----------------------------------------------------------------------

    // Construct and immediately activate a Level subclass.
    // Shuts down and releases any previously active level first.
    // Example: LoadLevel<MainMenuLevel>();
    template<typename T>
    T* LoadLevel()
    {
        static_assert(std::is_base_of<Level, T>::value,
            "T must derive from LightningEngine::Level");

        // Shut down the previous level cleanly
        if (activeLevel)
        {
            activeLevel->Shutdown();
            activeLevel.reset();
        }

        auto lvl = std::make_unique<T>();
        T* ptr   = lvl.get();
        lvl->SetContext(renderer, inputManager);
        lvl->Initialize();
        activeLevel = std::move(lvl);
        return ptr;
    }

    // Convenience: tick the active level (call from your Update override).
    void UpdateLevel(float dt)
    {
        if (activeLevel) activeLevel->Update(dt);
    }

    // Convenience: render the active level (call from your Render override).
    void RenderLevel()
    {
        if (activeLevel) activeLevel->Render();
    }

private:
    std::unique_ptr<Level> activeLevel;
};

} // namespace LightningEngine
