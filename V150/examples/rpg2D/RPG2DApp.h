// RPG2DApp.h — Entry point for the RPG2D example.
// Shows how to wire GameInstance -> LoadLevel<GameLevel>().
//
// To run this instead of the Editor, change main.cpp to:
//   #include "examples/rpg2D/RPG2DApp.h"
//   int main() { RPG2D::RPG2DApp app; app.Run(); }

#pragma once
#include "../../src/include/GameInstance.h"
#include "GameLevel.h"

namespace RPG2D {

class RPG2DApp : public LightningEngine::GameInstance
{
public:
    void Initialize() override
    {
        renderer.SetClearColor(30, 30, 50);   // dark blue-grey sky
        LoadLevel<GameLevel>();
    }

    void Shutdown() override {}

    void Update(float dt) override
    {
        // Quit on Escape
        if (inputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE))
            Quit();

        UpdateLevel(dt);
    }

    void Render() override
    {
        RenderLevel();
    }
};

} // namespace RPG2D
