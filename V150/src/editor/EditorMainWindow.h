// EditorMainWindow.h - C#-style main window shell for the editor.
//
// EditorApp owns application/runtime concerns. EditorMainWindow owns the UI
// runtime and window-level presentation concerns. The current implementation is
// intentionally thin so the large EditorApp can be migrated panel by panel.
#pragma once

#include "EditorUI.h"
#include "../include/InputManager.h"
#include "../include/Renderer.h"
#include <functional>

namespace LightningEditor {

class EditorMainWindow {
public:
    UI::Runtime& UIRuntime() { return ui; }
    const UI::Runtime& UIRuntime() const { return ui; }

    bool Initialize(LightningEngine::Renderer& renderer,
                    const char* fontPath = "assets/fonts/Roboto-Regular.ttf",
                    int fontSize = 13)
    {
        this->renderer = &renderer;
        return ui.Init(renderer, fontPath, fontSize);
    }

    void Shutdown()
    {
        ui.Release();
        renderer = nullptr;
    }

    void Resize(float width, float height)
    {
        this->width = width;
        this->height = height;
    }

    void ProcessInput(LightningEngine::InputManager& input)
    {
        ui.ProcessInput(input);
    }

    void Render()
    {
        if (renderer) ui.Render(*renderer);
    }

    UI::Panel* CreateModalPanel(const char* title,
                                float modalWidth,
                                float modalHeight,
                                const std::function<void(UI::Panel*)>& populate,
                                int zOrder = 200)
    {
        float mx = (width - modalWidth) * 0.5f;
        float my = (height - modalHeight) * 0.5f;
        auto* panel = ui.AddRoot<UI::Panel>(mx, my, modalWidth, modalHeight, title);
        panel->visible = false;
        panel->zOrder = zOrder;
        ui.BringToFront(panel);
        if (populate) populate(panel);
        return panel;
    }

    void ShowDialog(UI::Widget* dialog)
    {
        if (!dialog) return;
        dialog->visible = true;
        ui.BringToFront(dialog);
    }

    void HideDialog(UI::Widget* dialog)
    {
        if (dialog) dialog->visible = false;
    }

    float Width() const { return width; }
    float Height() const { return height; }

private:
    UI::Runtime ui;
    LightningEngine::Renderer* renderer = nullptr;
    float width = 0.f;
    float height = 0.f;
};

} // namespace LightningEditor
