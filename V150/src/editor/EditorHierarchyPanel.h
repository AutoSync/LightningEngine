// EditorHierarchyPanel.h — Scene hierarchy tree panel.
#pragma once
#include "EditorPanel.h"

class EditorHierarchyPanel : public EditorPanel
{
public:
    explicit EditorHierarchyPanel(EditorContext& ctx) : ctx(ctx) {}

    Panel*       pHierarchy = nullptr;
    TreeView*    pHierTree  = nullptr;
    ContextMenu* hierMenu   = nullptr;

    std::unique_ptr<Widget> Build() override
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, ctx.kHierW, 300.f, "");
        pHierarchy = panel.get();

        const float pad = gStyle.padding;
        const float lh  = gStyle.lineH;
        float ty = pad;

        pHierarchy->Add<Label>(pad, ty + (lh - ctx.ui.font.GlyphH()) * 0.5f, "Scene")
                  ->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);
        ty += lh + 2.f;

        auto* btnAdd = pHierarchy->Add<Button>(ctx.kHierW - pad - 60.f, pad, 60.f, lh, "+ Node");
        btnAdd->onClick = [this]{ addNodeToScene("Node"); };

        pHierarchy->Add<Button>(pad, ty, ctx.kHierW - pad * 2.f, lh + 2.f, "Search...")->onClick = []{};
        ty += lh + 6.f;

        float svH = pHierarchy->h - ty - pad;
        pHierTree = pHierarchy->Add<TreeView>(pad, ty, ctx.kHierW - pad * 2.f, svH);
        pHierTree->showRoot = false;
        pHierTree->onSelect = [this](TreeNode* tn) {
            if (!tn) { ctx.selectedNode = nullptr; if (ctx.onInspectorRefresh) ctx.onInspectorRefresh(); return; }
            ctx.selectedNode = reinterpret_cast<Node*>(tn->userData);
            if (ctx.onInspectorRefresh) ctx.onInspectorRefresh();
        };

        return panel;
    }

    void Refresh() override { RebuildTree(); }

    void RebuildTree()
    {
        if (!pHierTree) return;
        pHierTree->root.ClearChildren();
        pHierTree->selected = nullptr;

        for (auto& n : ctx.editorLevel.GetNodes())
            addNodeToTree(&pHierTree->root, n.get());
    }

    void ClearPtrs() override
    {
        pHierarchy = nullptr;
        pHierTree  = nullptr;
        hierMenu   = nullptr;
    }

    void addNodeToScene(const std::string& name)
    {
        auto node = std::make_unique<Node>(name);
        ctx.editorLevel.AddNode(std::move(node));
        RebuildTree();
        if (ctx.noteChange) ctx.noteChange("Node added: " + name);
    }

    void duplicateSelected()
    {
        if (!ctx.selectedNode) return;
        auto clone = ctx.selectedNode->Clone();
        if (!clone) return;
        clone->name += " (Copy)";
        ctx.editorLevel.AddNode(std::move(clone));
        RebuildTree();
        if (ctx.noteChange) ctx.noteChange("Node duplicated");
    }

    void deleteSelected()
    {
        if (!ctx.selectedNode) return;
        Node* target = ctx.selectedNode;
        ctx.selectedNode = nullptr;

        auto& nodes = ctx.editorLevel.GetNodes();
        for (auto it = nodes.begin(); it != nodes.end(); ++it) {
            if (it->get() == target) {
                std::string name = target->name;
                nodes.erase(it);
                RebuildTree();
                if (ctx.onInspectorRefresh) ctx.onInspectorRefresh();
                if (ctx.noteChange) ctx.noteChange("Node deleted: " + name);
                return;
            }
        }
    }

private:
    EditorContext& ctx;

    void addNodeToTree(TreeNode* parent, Node* node)
    {
        if (!node) return;
        std::string tag = node->tag.empty() ? node->name : node->tag;
        TreeNode* tn    = parent->AddChild(node->name, tag);
        tn->userData    = reinterpret_cast<void*>(node);
        tn->expanded    = true;
        if (node == ctx.selectedNode) pHierTree->selected = tn;
        for (const auto& child : node->GetChildren())
            addNodeToTree(tn, child.get());
    }
};
