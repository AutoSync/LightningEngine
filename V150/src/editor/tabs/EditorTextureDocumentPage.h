#pragma once

#include <filesystem>

#include <SDL3/SDL.h>

#include "EditorDocumentPageContext.h"
#include "EditorDocumentContent.h"
#include "../../include/gui/widgets/Label.h"
#include "../../include/gui/widgets/TextureViewerWidget.h"

namespace LightningEditor {

namespace fs = std::filesystem;

inline bool RenderTextureDocumentPage(const EditorTabDescriptor& active,
                                      const EditorDocumentContent& content,
                                      EditorDocumentWorkspaceContext& context,
                                      float pad,
                                      float lh,
                                      float infoY,
                                      float editorY)
{
    if (active.kind != EditorTabKind::Texture || !content.fileExists ||
        !context.outlinePanel || !context.editorPanel || !context.renderer ||
        !context.previewTexture || !context.previewTexturePath) {
        return false;
    }

    if (*context.previewTexturePath != active.path) {
        if (context.previewTexture->IsValid()) context.previewTexture->Release();
        *context.previewTexture = context.renderer->LoadTexture(active.path.c_str());
        *context.previewTexturePath = active.path;
    }

    if (context.previewTexture->IsValid()) {
        auto addMeta = [&](const char* key, const std::string& value) {
            std::string line = std::string(key) + value;
            auto* label = context.outlinePanel->Add<Titan::Label>(pad, infoY, line.c_str());
            label->h = lh;
            label->SetColor(Titan::gStyle.textDim.r, Titan::gStyle.textDim.g, Titan::gStyle.textDim.b);
            infoY += lh + 2.f;
        };

        addMeta("Largura:  ", std::to_string(context.previewTexture->GetWidth()) + " px");
        addMeta("Altura:   ", std::to_string(context.previewTexture->GetHeight()) + " px");

        std::error_code ec;
        auto size = fs::file_size(active.path, ec);
        if (!ec) {
            char buf[32];
            if (size >= 1024 * 1024) {
                SDL_snprintf(buf, sizeof(buf), "%.1f MB", size / (1024.0 * 1024.0));
            } else if (size >= 1024) {
                SDL_snprintf(buf, sizeof(buf), "%.1f KB", size / 1024.0);
            } else {
                SDL_snprintf(buf, sizeof(buf), "%llu B", (unsigned long long)size);
            }
            addMeta("Tamanho: ", buf);
        }
        addMeta("Formato: ", active.extension);
    }

    float viewHeight = context.editorHeight - editorY - pad;
    float viewWidth = context.editorWidth - pad * 2.f;
    auto* viewer = context.editorPanel->Add<Titan::TextureViewerWidget>(
        pad, editorY, viewWidth, viewHeight);
    viewer->tex = context.previewTexture->IsValid() ? context.previewTexture : nullptr;
    viewer->gridTex = (context.gridTexture && context.gridTexture->IsValid()) ? context.gridTexture : nullptr;
    viewer->ResetView();
    return true;
}

} // namespace LightningEditor