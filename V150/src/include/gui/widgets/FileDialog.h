// FileDialog.h — Floating filesystem browser dialog (open/save).
// Requires C++17 <filesystem>. Use as a root widget; call Open() to show.
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <SDL3/SDL.h>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {
namespace fs = std::filesystem;

class FileDialog : public Widget {
public:
    enum class Mode { Open, Save, Folder };

    Mode        mode       = Mode::Open;
    std::string filter;              // extension filter e.g. ".cs", "" = all files
    std::string selectedPath;        // set when user confirms

    std::function<void(const std::string&)> onConfirm;
    std::function<void()>                   onCancel;

    FileDialog() = default;
    FileDialog(float w, float h) { this->w=w; this->h=h; }

    void Open(float screenW, float screenH, Mode m = Mode::Open,
              const char* startDir = nullptr, const char* ext = nullptr)
    {
        mode      = m;
        filter    = ext ? ext : "";
        visible   = true;
        x         = (screenW - w) * 0.5f;
        y         = (screenH - h) * 0.5f;
        inputBuf[0] = '\0';
        errMsg.clear();

        fs::path start;
        if (startDir && fs::exists(startDir)) start = startDir;
        else                                   start = fs::current_path();
        navigate(start);
    }

    void Close() { visible = false; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible) return false;
        float ax = ox+x, ay = oy+y;

        // Always consume input when visible (modal-like)
        bool inside = Contains(mx, my, ox, oy);

        // Drag title bar
        if (ldown && my >= ay && my < ay + kTitleH && mx >= ax && mx < ax + w) {
            if (lclick) { dragOx = mx - (ox+x); dragOy = my - (oy+y); dragging = true; }
        }
        if (dragging) {
            if (ldown) { x = mx - dragOx - ox; y = my - dragOy - oy; }
            else        dragging = false;
        }

        // Close button (top-right X)
        float closeX = ax + w - kTitleH;
        bool overClose = mx >= closeX && mx < ax + w && my >= ay && my < ay + kTitleH;
        if (lclick && overClose) { Close(); if (onCancel) onCancel(); return true; }

        // Up button
        float upBtnX = ax + 4.f;
        float upBtnY = ay + kTitleH + 4.f;
        bool overUp = mx >= upBtnX && mx < upBtnX + 30.f &&
                      my >= upBtnY && my < upBtnY + kBtnH;
        if (lclick && overUp && curDir.has_parent_path())
            navigate(curDir.parent_path());

        // File list scroll (mouse wheel)
        float listY = ay + kTitleH + kNavH;
        float listH = h - kTitleH - kNavH - kFootH;
        bool overList = mx >= ax && mx < ax + w && my >= listY && my < listY + listH;
        if (overList) {
            float wheel = GetUIScrollY(ui);
            if (wheel != 0.f) {
                listScroll -= wheel * kItemH * 3.f;
                clampScroll();
            }
        }

        // File list click
        if (lclick && overList) {
            int idx = (int)((my - listY + listScroll) / kItemH);
            if (idx >= 0 && idx < (int)entries.size()) {
                const auto& e = entries[idx];
                if (fs::is_directory(e)) {
                    navigate(e);
                } else {
                    selectedIdx  = idx;
                    // Fill filename input
                    std::string fname = e.filename().string();
                    SDL_strlcpy(inputBuf, fname.c_str(), sizeof(inputBuf));
                }
            }
        }

        // OK button
        float okX  = ax + w - 88.f;
        float btnY = ay + h - kBtnH - 8.f;
        bool overOk = mx >= okX && mx < okX + 40.f && my >= btnY && my < btnY + kBtnH;
        if (lclick && overOk) confirmSelection();

        // Cancel button
        float cancelX = ax + w - 44.f;
        bool overCancel = mx >= cancelX && mx < cancelX + 40.f && my >= btnY && my < btnY + kBtnH;
        if (lclick && overCancel) { Close(); if (onCancel) onCancel(); }

        // Keyboard input for filename field (simple: store raw chars)
        // Focus is external; we handle it via uiContext focus
        return inside || true;  // always consume when visible
    }

    void ReceiveText(const std::string& txt, bool backspace) override
    {
        if (!visible) return;
        if (backspace) {
            size_t len = SDL_strlen(inputBuf);
            if (len > 0) inputBuf[len-1] = '\0';
        } else {
            SDL_strlcat(inputBuf, txt.c_str(), sizeof(inputBuf));
        }
    }

    void ReceiveKey(SDL_Scancode key) override
    {
        if (!visible) return;
        if (key == SDL_SCANCODE_RETURN) confirmSelection();
        if (key == SDL_SCANCODE_ESCAPE) { Close(); if (onCancel) onCancel(); }
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;
        float pad = s.padding;

        // Backdrop dim
        r.SetDrawColor(0, 0, 0, 140);
        r.FillRect(ox, oy, 4096.f, 4096.f);  // covers screen

        // Dialog background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, h);

        // Title bar
        r.SetDrawColor(s.panelHeader.r, s.panelHeader.g, s.panelHeader.b);
        r.FillRect(ax, ay, w, kTitleH);

        const char* title = (mode == Mode::Open)   ? "Open File"     :
                            (mode == Mode::Save)   ? "Save File"     :
                                                     "Select Folder";
        r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
        f.DrawText(r, title, ax + pad, ay + (kTitleH - f.GlyphH()) * 0.5f);

        // Close button
        float closeX = ax + w - kTitleH;
        r.SetDrawColor(s.btnNormal.r, s.btnNormal.g, s.btnNormal.b);
        r.FillRect(closeX, ay, kTitleH, kTitleH);
        r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
        float cw = f.MeasureW("x");
        f.DrawText(r, "x", closeX + (kTitleH - cw) * 0.5f, ay + (kTitleH - f.GlyphH()) * 0.5f);

        // Navigation bar (current path + Up button)
        float navY = ay + kTitleH;
        r.SetDrawColor(s.btnNormal.r, s.btnNormal.g, s.btnNormal.b);
        r.FillRect(ax, navY, w, kNavH);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.FillRect(ax, navY + kNavH - 1.f, w, 1.f);

        // Up button
        float upBtnX = ax + pad;
        float upBtnY2 = navY + (kNavH - kBtnH) * 0.5f;
        r.SetDrawColor(s.btnNormal.r + 10, s.btnNormal.g + 10, s.btnNormal.b + 10);
        r.FillRect(upBtnX, upBtnY2, 30.f, kBtnH);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(upBtnX, upBtnY2, 30.f, kBtnH);
        float upW = f.MeasureW("..");
        r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
        f.DrawText(r, "..", upBtnX + (30.f - upW) * 0.5f, upBtnY2 + (kBtnH - f.GlyphH()) * 0.5f);

        // Current path
        std::string pathStr = curDir.string();
        float pathX = upBtnX + 34.f;
        float pathMaxW = w - 40.f;
        while (!pathStr.empty() && f.MeasureW(pathStr.c_str()) > pathMaxW)
            pathStr.erase(0, 1);
        r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
        f.DrawText(r, pathStr.c_str(), pathX, navY + (kNavH - f.GlyphH()) * 0.5f);

        // File list
        float listY  = navY + kNavH;
        float listH  = h - kTitleH - kNavH - kFootH;
        r.SetDrawColor(22, 22, 30);
        r.FillRect(ax, listY, w, listH);

        int firstItem = (int)(listScroll / kItemH);
        int maxItems  = (int)(listH / kItemH) + 2;

        for (int i = firstItem; i < std::min(firstItem + maxItems, (int)entries.size()); i++) {
            float iy = listY + i * kItemH - listScroll;
            if (iy + kItemH < listY || iy >= listY + listH) continue;

            bool sel = (i == selectedIdx);
            if (sel) {
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b, 60);
                r.FillRect(ax, iy, w, kItemH);
            } else if (i % 2 == 1) {
                r.SetDrawColor(26, 26, 35);
                r.FillRect(ax, iy, w, kItemH);
            }

            bool isDir = fs::is_directory(entries[i]);
            std::string name = entries[i].filename().string();
            if (isDir) name = "[" + name + "]";

            if (isDir)
                r.SetDrawColor(s.textAccent.r, s.textAccent.g, s.textAccent.b);
            else
                r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
            f.DrawText(r, name.c_str(), ax + pad, iy + (kItemH - f.GlyphH()) * 0.5f);
        }

        // List border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, listY, w, listH);

        // Scrollbar
        float totalH = entries.size() * kItemH;
        if (totalH > listH) {
            float thumbH = std::max(16.f, listH * listH / totalH);
            float thumbY = listY + (listScroll / (totalH - listH)) * (listH - thumbH);
            r.SetDrawColor(s.panelBorder.r + 20, s.panelBorder.g + 20, s.panelBorder.b + 20);
            r.FillRect(ax + w - 6.f, thumbY, 4.f, thumbH);
        }

        // Footer (filename input + buttons)
        float footY = listY + listH;
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, footY, w, kFootH);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.FillRect(ax, footY, w, 1.f);

        float fnY   = footY + (kFootH - kBtnH) * 0.5f;
        float fnW   = w - 104.f;

        // Filename input box
        r.SetDrawColor(22, 22, 30);
        r.FillRect(ax + pad, fnY, fnW, kBtnH);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax + pad, fnY, fnW, kBtnH);
        r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
        f.DrawText(r, inputBuf, ax + pad + 4.f, fnY + (kBtnH - f.GlyphH()) * 0.5f);

        // OK button
        float okX2 = ax + w - 88.f;
        r.SetDrawColor(s.btnNormal.r + 10, s.btnNormal.g + 10, s.btnNormal.b + 10);
        r.FillRect(okX2, fnY, 40.f, kBtnH);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(okX2, fnY, 40.f, kBtnH);
        const char* okLbl = (mode == Mode::Open)   ? "Open"   :
                            (mode == Mode::Save)   ? "Save"   :
                                                     "Select";
        float okLblW = f.MeasureW(okLbl);
        r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
        f.DrawText(r, okLbl, okX2 + (40.f - okLblW) * 0.5f, fnY + (kBtnH - f.GlyphH()) * 0.5f);

        // Cancel button
        float cancelX2 = ax + w - 44.f;
        r.SetDrawColor(s.btnNormal.r, s.btnNormal.g, s.btnNormal.b);
        r.FillRect(cancelX2, fnY, 40.f, kBtnH);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(cancelX2, fnY, 40.f, kBtnH);
        float cancelW = f.MeasureW("Cancel");
        r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
        f.DrawText(r, "Cancel", cancelX2 + (40.f - cancelW) * 0.5f, fnY + (kBtnH - f.GlyphH()) * 0.5f);

        // Error message
        if (!errMsg.empty()) {
            r.SetDrawColor(s.textRed.r, s.textRed.g, s.textRed.b);
            f.DrawText(r, errMsg.c_str(), ax + pad, fnY + (kBtnH - f.GlyphH()) * 0.5f + kBtnH + 2.f);
        }

        // Outer border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);
    }

private:
    static constexpr float kTitleH = 24.f;
    static constexpr float kNavH   = 26.f;
    static constexpr float kFootH  = 38.f;
    static constexpr float kBtnH   = 22.f;
    static constexpr float kItemH  = 20.f;

    fs::path              curDir;
    std::vector<fs::path> entries;
    int                   selectedIdx = -1;
    float                 listScroll  = 0.f;
    char                  inputBuf[512] = {};
    std::string           errMsg;
    bool                  dragging = false;
    float                 dragOx = 0.f, dragOy = 0.f;

    void navigate(const fs::path& dir)
    {
        try {
            curDir      = fs::canonical(dir);
            entries.clear();
            selectedIdx = -1;
            listScroll  = 0.f;
            inputBuf[0] = '\0';

            std::vector<fs::path> dirs, files;
            for (const auto& e : fs::directory_iterator(curDir)) {
                if (fs::is_directory(e)) dirs.push_back(e);
                else if (filter.empty() || e.path().extension() == filter)
                    files.push_back(e);
            }
            std::sort(dirs.begin(),  dirs.end());
            std::sort(files.begin(), files.end());
            entries.insert(entries.end(), dirs.begin(),  dirs.end());
            entries.insert(entries.end(), files.begin(), files.end());
        } catch (...) { errMsg = "Cannot read directory."; }
    }

    void clampScroll()
    {
        float listH  = h - kTitleH - kNavH - kFootH;
        float totalH = entries.size() * kItemH;
        listScroll = std::clamp(listScroll, 0.f, std::max(0.f, totalH - listH));
    }

    void confirmSelection()
    {
        // Folder mode: always returns curDir (ignore inputBuf)
        if (mode == Mode::Folder) {
            selectedPath = curDir.string();
            Close();
            if (onConfirm) onConfirm(selectedPath);
            return;
        }
        if (inputBuf[0] == '\0') { errMsg = "Enter a filename."; return; }
        fs::path full = curDir / inputBuf;
        if (mode == Mode::Open && !fs::exists(full)) {
            errMsg = "File not found."; return;
        }
        selectedPath = full.string();
        Close();
        if (onConfirm) onConfirm(selectedPath);
    }
};

} // namespace Titan
