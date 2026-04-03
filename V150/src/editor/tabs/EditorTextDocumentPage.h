#pragma once

#include "EditorDocumentPageContext.h"
#include "EditorDocumentContent.h"

namespace LightningEditor {

inline bool RenderTextDocumentPage(const EditorDocumentContent& content,
                                   EditorDocumentWorkspaceContext& context,
                                   float pad,
                                   float editorY)
{
    if (!content.textual || !context.editorPanel) return false;

    float richTextHeight = context.editorHeight - editorY - pad;
    auto* editor = context.editorPanel->Add<Titan::RichText>(
        pad, editorY, context.editorWidth - pad * 2.f, richTextHeight);
    editor->syntax = content.syntax;
    editor->SetText(content.bodyText);
    editor->onChanged = context.onTextChanged;

    if (context.activeTextEditor) *context.activeTextEditor = editor;
    return true;
}

} // namespace LightningEditor