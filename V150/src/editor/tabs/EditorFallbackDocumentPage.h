#pragma once

#include "EditorDocumentPageContext.h"

namespace LightningEditor {

inline void RenderFallbackDocumentPage(EditorDocumentWorkspaceContext& context,
                                       float pad,
                                       float lh,
                                       float editorY)
{
    auto* info1 = context.editorPanel->Add<Titan::Label>(
        pad, editorY, "Este tipo de arquivo usa fluxo de importacao ou editor dedicado.");
    info1->h = lh;
    info1->SetColor(Titan::gStyle.textBright.r, Titan::gStyle.textBright.g, Titan::gStyle.textBright.b);
    editorY += lh + 4.f;

    auto* info2 = context.editorPanel->Add<Titan::Label>(
        pad, editorY, "A aba organiza o contexto pelo arquivo aberto, sem conteudo de exemplo embutido.");
    info2->h = lh;
    info2->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
}

} // namespace LightningEditor