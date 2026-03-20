#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "EditorTabSystem.h"
#include "../../include/gui/widgets/RichText.h"

namespace LightningEditor {

struct EditorDocumentOutlineItem {
    std::string label;
    std::uint8_t r = 185;
    std::uint8_t g = 185;
    std::uint8_t b = 195;
};

struct EditorDocumentContent {
    std::string kindLabel;
    std::string pathLabel;
    std::string statusLabel;
    std::string editorTitle;
    std::string bodyText;
    bool fileExists = false;
    bool textual = false;
    bool editable = false;
    Titan::RichText::SyntaxMode syntax = Titan::RichText::SyntaxMode::None;
    std::vector<EditorDocumentOutlineItem> outline;
};

inline std::string TrimCopy(const std::string& text)
{
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

inline std::string FileKindLabel(EditorTabKind kind)
{
    switch (kind) {
    case EditorTabKind::Scene: return "Cena";
    case EditorTabKind::Script: return "Script";
    case EditorTabKind::Texture: return "Textura";
    case EditorTabKind::Shader: return "Shader";
    case EditorTabKind::Material: return "Material";
    case EditorTabKind::Particle: return "Particula";
    case EditorTabKind::Prefab: return "Prefab";
    case EditorTabKind::StaticMesh: return "Malha Estatica";
    case EditorTabKind::SkeletalMesh: return "Malha Animada";
    case EditorTabKind::Config: return "Configuracao";
    default: return "Documento";
    }
}

inline bool ReadTextFile(const std::string& path, std::string& outText)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    std::ostringstream buffer;
    buffer << in.rdbuf();
    outText = buffer.str();

    const std::size_t checkLen = std::min<std::size_t>(outText.size(), 2048);
    for (std::size_t i = 0; i < checkLen; ++i) {
        if (outText[i] == '\0') return false;
    }
    return true;
}

inline bool WriteTextFile(const std::string& path, const std::vector<std::string>& lines)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;

    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i) out << '\n';
        out << lines[i];
    }
    return static_cast<bool>(out);
}

inline void AppendUniqueOutline(std::vector<EditorDocumentOutlineItem>& outline,
                                const std::string& label,
                                std::uint8_t r,
                                std::uint8_t g,
                                std::uint8_t b)
{
    if (label.empty()) return;
    for (const auto& item : outline) {
        if (item.label == label) return;
    }
    outline.push_back({ label, r, g, b });
}

inline std::vector<EditorDocumentOutlineItem> BuildOutline(const EditorTabDescriptor& tab,
                                                           const std::string& bodyText)
{
    std::vector<EditorDocumentOutlineItem> outline;
    std::istringstream input(bodyText);
    std::string line;

    while (std::getline(input, line)) {
        std::string trimmed = TrimCopy(line);
        if (trimmed.empty()) continue;

        if (tab.kind == EditorTabKind::Script) {
            if (trimmed.rfind("class ", 0) == 0 || trimmed.find(" class ") != std::string::npos) {
                AppendUniqueOutline(outline, trimmed, 255, 138, 28);
            } else if (trimmed.find('(') != std::string::npos && trimmed.find(')') != std::string::npos) {
                AppendUniqueOutline(outline, trimmed, 180, 220, 130);
            } else if (trimmed.find(';') != std::string::npos) {
                AppendUniqueOutline(outline, trimmed, 100, 160, 230);
            }
            continue;
        }

        if (tab.kind == EditorTabKind::Shader) {
            if (trimmed.rfind("uniform ", 0) == 0 || trimmed.rfind("layout", 0) == 0) {
                AppendUniqueOutline(outline, trimmed, 72, 190, 90);
            } else if (trimmed.rfind("#", 0) == 0) {
                AppendUniqueOutline(outline, trimmed, 140, 140, 150);
            } else if (trimmed.find("void ") != std::string::npos && trimmed.find('(') != std::string::npos) {
                AppendUniqueOutline(outline, trimmed, 180, 220, 130);
            }
            continue;
        }

        if (tab.kind == EditorTabKind::Config || tab.kind == EditorTabKind::Material) {
            if (trimmed.front() == '[' || trimmed.front() == '{') {
                AppendUniqueOutline(outline, trimmed, 140, 140, 150);
            } else if (trimmed.find('=') != std::string::npos || trimmed.find(':') != std::string::npos) {
                AppendUniqueOutline(outline, trimmed, 100, 160, 230);
            }
            continue;
        }

        if (outline.size() < 12) {
            AppendUniqueOutline(outline, trimmed, 185, 185, 195);
        }
    }

    return outline;
}

inline EditorDocumentContent BuildDocumentContent(const EditorTabDescriptor& tab)
{
    EditorDocumentContent content;
    content.kindLabel = FileKindLabel(tab.kind);
    content.pathLabel = tab.path.empty() ? "Arquivo sem caminho definido" : tab.path;
    content.editorTitle = tab.label.empty() ? content.kindLabel : tab.label;

    if (tab.path.empty()) {
        content.statusLabel = "Aguardando geracao do arquivo pelo editor ou template.";
        return content;
    }

    content.fileExists = fs::exists(tab.path);
    if (!content.fileExists) {
        content.statusLabel = "Arquivo ainda nao existe. O conteudo sera criado pelo editor, importacao ou template.";
        return content;
    }

    content.textual = IsTextDocument(tab);
    content.editable = content.textual;
    if (!content.textual) {
        content.statusLabel = "Asset importado. Este tipo usa fluxo dedicado de importacao e edicao contextual.";
        return content;
    }

    if (!ReadTextFile(tab.path, content.bodyText)) {
        content.textual = false;
        content.editable = false;
        content.bodyText.clear();
        content.statusLabel = "Nao foi possivel ler o arquivo como texto.";
        return content;
    }

    if (tab.kind == EditorTabKind::Script) {
        content.syntax = Titan::RichText::SyntaxMode::CSharp;
    }

    content.outline = BuildOutline(tab, content.bodyText);
    content.statusLabel = content.outline.empty()
        ? "Documento carregado. Estrutura ainda sera gerada conforme o editor evoluir."
        : "Documento carregado com estrutura derivada do proprio arquivo.";
    return content;
}

} // namespace LightningEditor