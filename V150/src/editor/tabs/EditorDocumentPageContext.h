#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../../include/Renderer.h"
#include "../../include/Texture.h"
#include "../../include/gui/widgets/Panel.h"
#include "../../include/gui/widgets/TabStrip.h"
#include "../../include/gui/widgets/RichText.h"

namespace LightningEditor {

struct EditorDocumentWorkspaceContext {
    Titan::Panel* filesPanel = nullptr;
    Titan::Panel* outlinePanel = nullptr;
    Titan::Panel* editorPanel = nullptr;
    Titan::TabStrip* tabStrip = nullptr;
    Titan::RichText** activeTextEditor = nullptr;
    LightningEngine::Renderer* renderer = nullptr;
    LightningEngine::Texture* gridTexture = nullptr;
    LightningEngine::Texture* previewTexture = nullptr;
    std::string* previewTexturePath = nullptr;
    float editorWidth = 0.f;
    float editorHeight = 0.f;
    std::function<void(int)> onActivateTab;
    std::function<void(const std::vector<std::string>&)> onTextChanged;
};

} // namespace LightningEditor