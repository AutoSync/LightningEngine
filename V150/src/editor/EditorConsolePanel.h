// EditorConsolePanel.h — Console/log output panel.
#pragma once
#include "EditorPanel.h"
#include "../include/Logger.h"

class EditorConsolePanel : public EditorPanel
{
public:
    explicit EditorConsolePanel(EditorContext& ctx) : ctx(ctx) {}

    Panel*      pConsolePanel = nullptr;
    ScrollView* pConsoleSV    = nullptr;
    bool        consoleVisible = true;

    std::unique_ptr<Widget> Build() override
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, 420.f, 180.f, "");
        panel->clipChildren = true;
        pConsolePanel = panel.get();
        Refresh();
        return panel;
    }

    void Refresh() override
    {
        if (!pConsolePanel) return;
        pConsolePanel->Clear();

        const float pad = gStyle.padding;
        const float lh  = gStyle.lineH;
        float ty = pad;

        auto* hdr = pConsolePanel->Add<Label>(pad, ty, "Console Output");
        hdr->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

        auto* btnClear = pConsolePanel->Add<Button>(pConsolePanel->w - pad - 56.f, ty - 1.f, 56.f, lh + 2.f, "Clear");
        btnClear->onClick = [this]{
            Logger::ClearMessages();
            Refresh();
        };
        ty += lh + 4.f;

        float bodyH = pConsolePanel->h - ty - pad;
        pConsoleSV = pConsolePanel->Add<ScrollView>(pad, ty, pConsolePanel->w - pad * 2.f, bodyH);
        pConsoleSV->autoContent = true;

        const auto& msgs = Logger::GetMessages();
        for (const auto& m : msgs) {
            auto* lbl = pConsoleSV->Add<Label>(0.f, 0.f, m.text.c_str());
            lbl->h = lh;
            switch (m.level) {
                case Logger::MsgLevel::Warning: lbl->SetColor(220, 180, 60); break;
                case Logger::MsgLevel::Error:   lbl->SetColor(220, 80, 80); break;
                default: lbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b); break;
            }
        }
    }

    void Toggle()
    {
        consoleVisible = !consoleVisible;
        if (pConsolePanel) pConsolePanel->visible = consoleVisible;
    }

    void ClearPtrs() override
    {
        pConsolePanel = nullptr;
        pConsoleSV = nullptr;
    }

private:
    EditorContext& ctx;
};
