// EditorElementRegistry.h — Runtime registry for editor-extensible elements.
// Phase 1: registry-driven data model with flat menu output (category/subcategory prefixes).
#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace LightningEngine {

enum class EditorElementSource {
    Core,
    CodeExpansion,
    PluginGlobal,
    PluginProject
};

enum class EditorElementKind {
    NodeFactory,
    ComponentFactory,
    AssetType,
    MenuAction,
    PanelFactory
};

struct EditorElementDescriptor {
    std::string id;
    std::string label;
    std::string category;
    std::string subcategory;
    EditorElementKind kind = EditorElementKind::MenuAction;
    EditorElementSource source = EditorElementSource::Core;
    int sortOrder = 0;
    bool enabled = true;
    std::function<void()> action;
};

class EditorElementRegistry {
public:
    using Bucket = std::vector<EditorElementDescriptor>;
    using SubcategoryMap = std::map<std::string, Bucket>;
    using CategoryMap = std::map<std::string, SubcategoryMap>;

    static EditorElementRegistry& Instance()
    {
        static EditorElementRegistry instance;
        return instance;
    }

    void Clear()
    {
        elements.clear();
    }

    bool Register(EditorElementDescriptor descriptor)
    {
        if (descriptor.id.empty()) return false;
        auto it = std::find_if(elements.begin(), elements.end(),
            [&descriptor](const EditorElementDescriptor& e) { return e.id == descriptor.id; });
        if (it != elements.end()) return false;
        elements.push_back(std::move(descriptor));
        return true;
    }

    const std::vector<EditorElementDescriptor>& All() const
    {
        return elements;
    }

    CategoryMap GroupByCategory(EditorElementKind kind) const
    {
        CategoryMap grouped;
        for (const auto& e : elements) {
            if (!e.enabled || e.kind != kind) continue;
            grouped[e.category][e.subcategory].push_back(e);
        }

        for (auto& [category, subMap] : grouped) {
            for (auto& [subcategory, bucket] : subMap) {
                std::sort(bucket.begin(), bucket.end(),
                    [](const EditorElementDescriptor& a, const EditorElementDescriptor& b) {
                        if (a.sortOrder != b.sortOrder) return a.sortOrder < b.sortOrder;
                        return a.label < b.label;
                    });
            }
        }
        return grouped;
    }

private:
    std::vector<EditorElementDescriptor> elements;
};

} // namespace LightningEngine
