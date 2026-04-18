// EditorScriptPanel.h — Document workspace / script panel.
#pragma once
#include "EditorPanel.h"
#include "tabs/EditorDocumentContent.h"
#include "tabs/EditorDocumentWorkspaceRenderer.h"

class EditorScriptPanel : public EditorPanel
{
public:
    explicit EditorScriptPanel(EditorContext& ctx) : ctx(ctx) {}

    // Bottom tray script preview
    Panel*    pScriptPanel = nullptr;
    RichText* pScriptEdit  = nullptr;

    // Document workspace (full dock)
    DockSpace* pScriptDock            = nullptr;
    Panel*     pDocumentFilesPanel    = nullptr;
    Panel*     pDocumentOutlinePanel  = nullptr;
    Panel*     pDocumentEditorPanel   = nullptr;
    RichText*  pScriptDockEdit        = nullptr;

    void BuildScriptDock(float kW, float kTopH, float kMainH, float kScriptH)
    {
        static constexpr float kScriptLeftW = 220.f;
        float fullH = kMainH + kScriptH;

        pScriptDock = ctx.ui.AddRoot<DockSpace>(0.f, kTopH, kW, fullH);
        pScriptDock->visible = false;

        DockNode* root = pScriptDock->Root();
        float leftFrac = kScriptLeftW / kW;
        auto [leftNode, editorNode] = root->Split(true, leftFrac);
        auto [filesNode, membersNode] = leftNode->Split(false, 0.5f);

        filesNode->Dock(buildFilesPanel(), "Open Documents");
        membersNode->Dock(buildMembersPanel(), "Document Outline");
        editorNode->Dock(buildEditorPanel(kW - kScriptLeftW, fullH), "Document Workspace");
    }

    std::unique_ptr<Widget> Build() override
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, 400.f, 180.f, "");
        panel->clipChildren = true;
        pScriptPanel = panel.get();
        RefreshScriptPreview();
        return panel;
    }

    void RefreshScriptPreview()
    {
        if (!pScriptPanel) return;
        pScriptPanel->Clear();
        pScriptEdit = nullptr;

        const float pad = gStyle.padding;
        const float spW = pScriptPanel->w;
        float ty = pad;

        std::string scriptPath;
        if (ctx.selectedNode) {
            if (auto* script = ctx.selectedNode->GetComponent<NucleoScriptComponent>()) {
                scriptPath = ctx.resolveFilePath ? ctx.resolveFilePath(script->scriptPath) : script->scriptPath;
            }
        }

        std::string scriptLabel = scriptPath.empty()
            ? "Nenhum script associado"
            : fs::path(scriptPath).filename().string();

        pScriptPanel->Add<Label>(pad, ty, scriptLabel.c_str())
                    ->SetColor(gStyle.textAccent.r, gStyle.textAccent.g, gStyle.textAccent.b);

        float rh = pScriptPanel->h - ty - gStyle.lineH - pad;
        pScriptEdit = pScriptPanel->Add<RichText>(pad, ty + gStyle.lineH, spW - pad * 2.f, rh);
        pScriptEdit->syntax = RichText::SyntaxMode::None;
        pScriptEdit->SetText("");
        if (ctx.applyTextPrefs) ctx.applyTextPrefs(pScriptEdit);

        if (!scriptPath.empty()) {
            auto scriptTab = LightningEditor::BuildAssetTab(scriptPath);
            auto content = LightningEditor::BuildDocumentContent(scriptTab);
            pScriptEdit->syntax = content.syntax;
            pScriptEdit->SetText(content.bodyText);
            if (ctx.applyTextPrefs) ctx.applyTextPrefs(pScriptEdit);
        }
    }

    void RefreshDocumentWorkspace(Texture& gridTex, Texture& texEditorTex, std::string& texEditorPath)
    {
        if (!pDocumentFilesPanel || !pDocumentOutlinePanel || !pDocumentEditorPanel) return;

        LightningEditor::EditorDocumentWorkspaceContext context;
        context.filesPanel = pDocumentFilesPanel;
        context.outlinePanel = pDocumentOutlinePanel;
        context.editorPanel = pDocumentEditorPanel;
        context.tabStrip = nullptr; // set by EditorApp
        context.activeTextEditor = &pScriptDockEdit;
        context.renderer = &ctx.renderer;
        context.gridTexture = &gridTex;
        context.previewTexture = &texEditorTex;
        context.previewTexturePath = &texEditorPath;
        context.editorWidth = (pDocumentEditorPanel->w > 0.f) ? pDocumentEditorPanel->w : 800.f;
        context.editorHeight = (pDocumentEditorPanel->h > 0.f) ? pDocumentEditorPanel->h : 600.f;
        context.onActivateTab = nullptr; // set by EditorApp
        context.onTextChanged = nullptr; // set by EditorApp

        LightningEditor::RenderDocumentWorkspace(ctx.tabManager, context);
        if (ctx.applyTextPrefs) ctx.applyTextPrefs(pScriptDockEdit);
    }

    void ClearPtrs() override
    {
        pScriptPanel = nullptr;
        pScriptEdit  = nullptr;
        pScriptDock  = nullptr;
        pDocumentFilesPanel = nullptr;
        pDocumentOutlinePanel = nullptr;
        pDocumentEditorPanel = nullptr;
        pScriptDockEdit = nullptr;
    }

private:
    EditorContext& ctx;

    std::unique_ptr<Widget> buildFilesPanel()
    {
        auto p = std::make_unique<Panel>(0.f, 0.f, 200.f, 200.f, "");
        pDocumentFilesPanel = p.get();
        return p;
    }

    std::unique_ptr<Widget> buildMembersPanel()
    {
        auto p = std::make_unique<Panel>(0.f, 0.f, 200.f, 200.f, "");
        pDocumentOutlinePanel = p.get();
        return p;
    }

    std::unique_ptr<Widget> buildEditorPanel(float w, float h)
    {
        auto p = std::make_unique<Panel>(0.f, 0.f, w, h, "");
        p->clipChildren = true;
        pDocumentEditorPanel = p.get();
        return p;
    }
};
