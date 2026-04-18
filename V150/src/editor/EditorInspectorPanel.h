// EditorInspectorPanel.h — Node property inspector panel.
#pragma once
#include "EditorPanel.h"
#include "../include/components/SpriteRenderer.h"
#include "../include/Nucleo.h"

class EditorInspectorPanel : public EditorPanel
{
public:
    explicit EditorInspectorPanel(EditorContext& ctx) : ctx(ctx) {}

    Panel* pInspector = nullptr;
    float  inspLastW  = 0.f;

    std::unique_ptr<Widget> Build() override
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, ctx.kHierW, 300.f, "");
        pInspector = panel.get();
        Refresh();
        return panel;
    }

    void Refresh() override
    {
        if (!pInspector) return;
        pInspector->Clear();

        const float pad    = gStyle.padding;
        const float lh     = gStyle.lineH;
        const float panelW = (pInspector->w > 0.f) ? pInspector->w : ctx.kHierW;
        const float innerW = panelW - pad * 2.f;
        auto* stack = pInspector->Add<VStack>(pad, pad, innerW, 3.f);

        if (!ctx.selectedNode) {
            auto* lbl = stack->Add<Label>(0.f, 0.f, "No selection");
            lbl->h = lh;
            lbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            return;
        }

        Node* node = ctx.selectedNode;

        // Name
        stack->Add<Label>(0.f, 0.f, "Name")->h = lh;
        auto* tfName = stack->Add<TextField>(0.f, 0.f, innerW, lh + 2.f);
        tfName->SetText(node->name);
        tfName->onChanged = [this, node](const std::string& v) {
            node->name = v;
            if (node->level) node->level->MarkNodeLookupDirty();
            if (ctx.onHierarchyChanged) ctx.onHierarchyChanged();
        };
        stack->Add<Separator>(innerW);

        // Tag
        stack->Add<Label>(0.f, 0.f, "Tag")->h = lh;
        auto* tfTag = stack->Add<TextField>(0.f, 0.f, innerW, lh + 2.f);
        tfTag->SetText(node->tag);
        tfTag->onChanged = [node](const std::string& v) {
            node->tag = v;
            if (node->level) node->level->MarkNodeLookupDirty();
        };
        stack->Add<Separator>(innerW);

        // Active
        auto* chk = stack->Add<Checkbox>(0.f, 0.f, "Active", node->active);
        chk->h = lh;
        chk->onChange = [node](bool v) { node->active = v; };
        stack->Add<Separator>(innerW);

        // Transform — Position
        buildTransformSection(stack, "Position", innerW, lh, node,
            { {"X", 220,80,80, &node->transform.Position.x},
              {"Y", 80,200,80, &node->transform.Position.y},
              {"Z", 80,120,220,&node->transform.Position.z} },
            -99999.f, 99999.f, 1.f, 1, "Move");
        stack->Add<Separator>(innerW);

        // Rotation
        buildTransformSection(stack, "Rotation", innerW, lh, node,
            { {"X", 220,80,80, &node->transform.Rotation.x},
              {"Y", 80,200,80, &node->transform.Rotation.y},
              {"Z", 80,120,220,&node->transform.Rotation.z} },
            -360.f, 360.f, 1.f, 1, "Rotate");
        stack->Add<Separator>(innerW);

        // Scale
        buildTransformSection(stack, "Scale", innerW, lh, node,
            { {"X", 220,80,80, &node->transform.Scale.x},
              {"Y", 80,200,80, &node->transform.Scale.y},
              {"Z", 80,120,220,&node->transform.Scale.z} },
            0.001f, 9999.f, 0.1f, 2, "Scale");
        stack->Add<Separator>(innerW);

        // Script (NucleoScript)
        if (auto* sc = node->GetComponent<NucleoScriptComponent>()) {
            auto* lbl = stack->Add<Label>(0.f, 0.f, "Script");
            lbl->h = lh;
            lbl->SetColor(255, 138, 28);

            auto* tfScript = stack->Add<TextField>(0.f, 0.f, innerW, lh + 2.f);
            tfScript->SetText(sc->scriptPath);
            tfScript->onChanged = [sc](const std::string& v) {
                sc->scriptPath = v;
                sc->Reload();
            };
            if (!sc->lastError.empty()) {
                auto* err = stack->Add<Label>(0.f, 0.f, sc->lastError.c_str());
                err->h = lh;
                err->SetColor(220, 80, 80);
            }
        }

        // Add Script button
        {
            auto* btn = stack->Add<Button>(0.f, 0.f, innerW, lh + 2.f, "+ Add Script Component");
            btn->onClick = [this, node]{
                if (!node->HasComponent<NucleoScriptComponent>())
                    node->AddComponent<NucleoScriptComponent>();
                Refresh();
            };
        }
    }

    void ClearPtrs() override
    {
        pInspector = nullptr;
    }

private:
    EditorContext& ctx;

    struct AxisInfo { const char* name; Uint8 r,g,b; float* ptr; };

    void buildTransformSection(VStack* stack, const char* title, float innerW, float lh,
                               Node* node, std::initializer_list<AxisInfo> axes,
                               float minV, float maxV, float step, int decimals,
                               const char* undoLabel)
    {
        auto* lbl = stack->Add<Label>(0.f, 0.f, title);
        lbl->h = lh;
        lbl->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

        for (const auto& a : axes) {
            auto* row = stack->Add<HStack>(0.f, 0.f, innerW, lh, 3.f);
            row->stretchH = true;
            auto* al = row->Add<Label>(0.f, 0.f, a.name);
            al->w = 14.f; al->h = lh;
            al->SetColor(a.r, a.g, a.b);
            float* ptr = a.ptr;
            auto* num = row->Add<NumericUpDown>(0.f, 0.f, innerW - 14.f, lh,
                                                *ptr, minV, maxV, step, decimals);
            num->onChanged = [this, node, ptr, undoLabel](float v) {
                float old = *ptr;
                ctx.undoStack.Do(
                    [node, ptr, v]{ *ptr = v; node->MarkTransformDirty(); },
                    [node, ptr, old]{ *ptr = old; node->MarkTransformDirty(); },
                    undoLabel);
            };
        }
    }
};
