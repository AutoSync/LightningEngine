#pragma once

#include "EditorDocumentPageContext.h"
#include "EditorDocumentContent.h"
#include "../../include/Equinox.h"

namespace LightningEditor {

inline bool RenderEquinoxDocumentPage(const EditorTabDescriptor& active,
                                      const EditorDocumentContent& content,
                                      EditorDocumentWorkspaceContext& context,
                                      float pad,
                                      float lh,
                                      float outlineY,
                                      float editorY)
{
    if (active.kind != EditorTabKind::Equinox || !context.outlinePanel || !context.editorPanel) {
        return false;
    }

    const auto info = LightningEngine::EquinoxFileManager::InspectDocument(active.path, content.bodyText);

    auto addOutline = [&](const std::string& text, Uint8 r, Uint8 g, Uint8 b) {
        auto* label = context.outlinePanel->Add<Titan::Label>(pad, outlineY, text.c_str());
        label->h = lh;
        label->SetColor(r, g, b);
        outlineY += lh + 2.f;
    };

    addOutline("Resumo Equinox", Titan::gStyle.textBright.r, Titan::gStyle.textBright.g, Titan::gStyle.textBright.b);
    addOutline("Tipo: " + info.displayName, 60, 150, 80);
    addOutline("Stage: " + info.stage, 170, 210, 120);
    addOutline("Output: " + info.output, 100, 160, 230);
    addOutline("Nodes detectados: " + std::to_string(info.nodeCount), Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
    addOutline("Parametros detectados: " + std::to_string(info.parameterCount), Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);

    auto* hint = context.editorPanel->Add<Titan::Label>(
        pad, editorY, "Editor Equinox: ajuste o documento e salve pela barra ou com Ctrl+S.");
    hint->h = lh;
    hint->SetColor(Titan::gStyle.textBright.r, Titan::gStyle.textBright.g, Titan::gStyle.textBright.b);
    editorY += lh + 4.f;

    std::string meta = "Tipo " + info.displayName + "  |  Stage " + info.stage +
        "  |  Output " + info.output;
    auto* metaLabel = context.editorPanel->Add<Titan::Label>(pad, editorY, meta.c_str());
    metaLabel->h = lh;
    metaLabel->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
    editorY += lh + 6.f;

    if (!content.textual) {
        auto* fallback = context.editorPanel->Add<Titan::Label>(
            pad, editorY, "Nao foi possivel carregar o documento Equinox como texto.");
        fallback->h = lh;
        fallback->SetColor(220, 90, 90);
        return true;
    }

    float richTextHeight = context.editorHeight - editorY - pad;
    auto* editor = context.editorPanel->Add<Titan::RichText>(
        pad, editorY, context.editorWidth - pad * 2.f, richTextHeight);
    editor->syntax = Titan::RichText::SyntaxMode::None;
    editor->SetText(content.bodyText);
    editor->onChanged = context.onTextChanged;

    if (context.activeTextEditor) *context.activeTextEditor = editor;
    return true;
}

} // namespace LightningEditor