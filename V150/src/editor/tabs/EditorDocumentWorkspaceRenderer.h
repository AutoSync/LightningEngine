#pragma once

#include "EditorDocumentContent.h"
#include "EditorDocumentPageContext.h"
#include "EditorFallbackDocumentPage.h"
#include "EditorTextDocumentPage.h"
#include "EditorTextureDocumentPage.h"
#include "../../include/gui/widgets/Button.h"
#include "../../include/gui/widgets/Label.h"

namespace LightningEditor {

inline void RenderDocumentWorkspace(const EditorTabManager& tabManager,
                                    EditorDocumentWorkspaceContext& context)
{
    if (!context.filesPanel || !context.outlinePanel || !context.editorPanel) return;

    const float pad = Titan::gStyle.padding;
    const float lh = Titan::gStyle.lineH;

    if (context.activeTextEditor) *context.activeTextEditor = nullptr;

    context.filesPanel->Clear();
    context.outlinePanel->Clear();
    context.editorPanel->Clear();

    float filesWidth = (context.filesPanel->w > 0.f) ? context.filesPanel->w : 200.f;
    float ty = pad;
    context.filesPanel->Add<Titan::Label>(pad, ty, "Abas abertas")
        ->SetColor(Titan::gStyle.textBright.r, Titan::gStyle.textBright.g, Titan::gStyle.textBright.b);
    ty += lh + 6.f;

    int openDocs = 0;
    const auto& tabs = tabManager.Tabs();
    for (int i = 0; i < (int)tabs.size(); ++i) {
        const auto& tab = tabs[i];
        if (!tab.IsDocumentWorkspace()) continue;
        ++openDocs;
        auto* button = context.filesPanel->Add<Titan::Button>(
            pad, ty, filesWidth - pad * 2.f, lh + 2.f, tab.label.c_str());
        button->SetColor(tab.accent.r, tab.accent.g, tab.accent.b);
        button->onClick = [cb = context.onActivateTab, i] {
            if (cb) cb(i);
        };
        ty += lh + 4.f;
    }

    if (openDocs == 0) {
        auto* empty = context.filesPanel->Add<Titan::Label>(pad, ty, "Nenhum documento aberto.");
        empty->h = lh;
        empty->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
    }

    const auto* active = tabManager.ActiveTab();
    if (!active || !active->IsDocumentWorkspace()) {
        auto* idleOutline = context.outlinePanel->Add<Titan::Label>(
            pad, pad, "Abra um asset para gerar o workspace contextual.");
        idleOutline->h = lh;
        idleOutline->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);

        auto* idleEditor = context.editorPanel->Add<Titan::Label>(
            pad, pad, "O conteudo do documento aparece aqui.");
        idleEditor->h = lh;
        idleEditor->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
        return;
    }

    EditorDocumentContent content = BuildDocumentContent(*active);

    float infoY = pad;
    context.outlinePanel->Add<Titan::Label>(pad, infoY, content.kindLabel.c_str())
        ->SetColor(active->accent.r, active->accent.g, active->accent.b);
    infoY += lh + 4.f;

    auto* pathLabel = context.outlinePanel->Add<Titan::Label>(pad, infoY, content.editorTitle.c_str());
    pathLabel->h = lh;
    pathLabel->SetColor(Titan::gStyle.textBright.r, Titan::gStyle.textBright.g, Titan::gStyle.textBright.b);
    infoY += lh + 2.f;

    auto* statusLabel = context.outlinePanel->Add<Titan::Label>(pad, infoY, content.statusLabel.c_str());
    statusLabel->h = lh * 2.f;
    statusLabel->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
    infoY += lh + 8.f;

    auto* outlineTitle = context.outlinePanel->Add<Titan::Label>(pad, infoY, "Estrutura derivada");
    outlineTitle->h = lh;
    outlineTitle->SetColor(Titan::gStyle.textBright.r, Titan::gStyle.textBright.g, Titan::gStyle.textBright.b);
    infoY += lh + 4.f;

    if (content.outline.empty()) {
        auto* empty = context.outlinePanel->Add<Titan::Label>(pad, infoY, "Sem estrutura derivada disponivel.");
        empty->h = lh;
        empty->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
    } else {
        for (const auto& item : content.outline) {
            auto* label = context.outlinePanel->Add<Titan::Label>(pad, infoY, item.label.c_str());
            label->h = lh;
            label->SetColor(item.r, item.g, item.b);
            infoY += lh + 2.f;
        }
    }

    float editorY = pad;
    context.editorPanel->Add<Titan::Label>(pad, editorY, content.editorTitle.c_str())
        ->SetColor(active->accent.r, active->accent.g, active->accent.b);
    editorY += lh;

    auto* fileLabel = context.editorPanel->Add<Titan::Label>(pad, editorY, content.pathLabel.c_str());
    fileLabel->h = lh;
    fileLabel->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
    editorY += lh + 4.f;

    if (RenderTextDocumentPage(content, context, pad, editorY)) return;
    if (RenderTextureDocumentPage(*active, content, context, pad, lh, infoY, editorY)) return;
    RenderFallbackDocumentPage(context, pad, lh, editorY);
}

} // namespace LightningEditor