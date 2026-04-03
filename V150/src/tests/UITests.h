#pragma once

#include "../include/gui/widgets/Button.h"
#include "../include/gui/widgets/Dropdown.h"
#include "../include/gui/widgets/TreeView.h"
#include "../include/gui/widgets/ScrollView.h"
#include <iostream>
#include <string>

namespace LightningTests {

class UITestLog {
public:
    int passed = 0;
    int failed = 0;

    void Expect(bool condition, const char* name, const std::string& detail = "")
    {
        if (condition) {
            passed++;
            std::cout << "[PASS] " << name;
            if (!detail.empty()) std::cout << " :: " << detail;
            std::cout << "\n";
            return;
        }

        failed++;
        std::cout << "[FAIL] " << name;
        if (!detail.empty()) std::cout << " :: " << detail;
        std::cout << "\n";
    }

    int ExitCode() const { return failed == 0 ? 0 : 1; }
};

inline int RunUITests()
{
    using namespace Titan;

    UITestLog t;

    // Button click inside / outside
    {
        bool clicked = false;
        Button btn(10.f, 10.f, 120.f, 24.f, "Atualizar");
        btn.onClick = [&clicked] { clicked = true; };

        bool consumed = btn.ProcessInput(20.f, 20.f, false, true, false, nullptr, 0.f, 0.f);
        t.Expect(consumed, "Button consome clique interno");
        t.Expect(clicked, "Button dispara onClick");

        clicked = false;
        consumed = btn.ProcessInput(200.f, 200.f, false, true, false, nullptr, 0.f, 0.f);
        t.Expect(!consumed, "Button ignora clique externo");
        t.Expect(!clicked, "Button nao dispara onClick fora");
    }

    // Dropdown open + item selection by click
    {
        Dropdown dd(0.f, 0.f, 120.f, 20.f, "Tipo");
        dd.AddItem("All");
        dd.AddItem("Images");
        dd.AddItem("Scripts");

        bool callbackFired = false;
        int callbackIdx = -1;
        dd.onSelect = [&callbackFired, &callbackIdx](int idx, const std::string&) {
            callbackFired = true;
            callbackIdx = idx;
        };

        // Open header
        bool consumed = dd.ProcessInput(4.f, 4.f, false, true, false, nullptr, 0.f, 0.f);
        t.Expect(consumed && dd.isOpen, "Dropdown abre com clique");

        // Select second item (Images)
        consumed = dd.ProcessInput(10.f, dd.h + Dropdown::kItemH + 2.f, false, true, false, nullptr, 0.f, 0.f);
        t.Expect(consumed, "Dropdown consome clique em item");
        t.Expect(dd.selectedIndex == 1, "Dropdown seleciona item correto");
        t.Expect(callbackFired && callbackIdx == 1, "Dropdown dispara callback de selecao");
    }

    // TreeView expand + select child by click
    {
        TreeView tv(0.f, 0.f, 220.f, 120.f);
        tv.itemH = 18.f;
        tv.showRoot = false;

        TreeNode* assets = tv.root.AddChild("Assets", "folder");
        assets->expanded = false;
        assets->AddChild("logo.png", "png");

        bool gotSelect = false;
        std::string selectedLabel;
        tv.onSelect = [&gotSelect, &selectedLabel](TreeNode* n) {
            gotSelect = (n != nullptr);
            selectedLabel = n ? n->label : "";
        };

        // Click arrow zone to expand first row
        bool consumed = tv.ProcessInput(2.f, 5.f, false, true, false, nullptr, 0.f, 0.f);
        t.Expect(consumed && assets->expanded, "TreeView expande no clique da seta");

        // Click second row (child label zone)
        consumed = tv.ProcessInput(22.f, tv.itemH + 4.f, false, true, false, nullptr, 0.f, 0.f);
        t.Expect(consumed, "TreeView consome clique em item");
        t.Expect(gotSelect && selectedLabel == "logo.png", "TreeView seleciona filho correto");
    }

    // ScrollView forwards click to visible child after scroll
    {
        ScrollView sv(0.f, 0.f, 220.f, 60.f);
        sv.autoContent = true;

        bool clicked = false;
        auto* btn = sv.Add<Button>(0.f, 70.f, 100.f, 20.f, "Importar");
        btn->onClick = [&clicked] { clicked = true; };

        sv.scrollOffset = 40.f; // brings button into visible area
        bool consumed = sv.ProcessInput(10.f, 35.f, false, true, false, nullptr, 0.f, 0.f);
        t.Expect(consumed, "ScrollView consome clique em filho visivel");
        t.Expect(clicked, "ScrollView encaminha clique ao botao filho");
    }

    std::cout << "[SUMMARY][UI] passed=" << t.passed << " failed=" << t.failed << "\n";
    return t.ExitCode();
}

} // namespace LightningTests
