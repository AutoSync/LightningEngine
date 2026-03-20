// TitanUI.h — Titan GUI manager.
#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include "Widget.h"
#include "TitanFont.h"
#include "TitanStyle.h"
#include "TitanTheme.h"
#include "widgets/Panel.h"
#include "widgets/Label.h"
#include "widgets/Button.h"
#include "widgets/Checkbox.h"
#include "widgets/Slider.h"
#include "widgets/TextField.h"
#include "widgets/Image.h"
#include "widgets/ProgressBar.h"
#include "widgets/Dropdown.h"
#include "widgets/ScrollView.h"
#include "widgets/VStack.h"
#include "widgets/HStack.h"
#include "widgets/Grid.h"
#include "widgets/Separator.h"
#include "widgets/Text.h"
#include "widgets/List.h"
#include "widgets/Border.h"
#include "widgets/Window.h"
#include "widgets/TabStrip.h"
#include "widgets/Viewport2D.h"
#include "widgets/Backdrop.h"
#include "widgets/ContextMenu.h"
#include "widgets/TreeView.h"
#include "widgets/NumericUpDown.h"
#include "widgets/Modal.h"
#include "widgets/MenuBar.h"
#include "widgets/Toolbar.h"
#include "widgets/ColorPicker.h"
#include "widgets/Splitter.h"
#include "widgets/RichText.h"
#include "widgets/DockSpace.h"
#include "widgets/StatusBar.h"
#include "widgets/Table.h"
#include "widgets/FileDialog.h"
#include "widgets/Chart.h"
#include "../Renderer.h"
#include "../InputManager.h"

namespace Titan {

class TitanUI {
public:
    TitanFont font;
    Widget*   captured  = nullptr;
    Widget*   focused   = nullptr;
    SDL_Window* sdlWin  = nullptr;
    float     scrollY   = 0.f;  // mouse wheel Y delta this frame

private:
    std::vector<std::unique_ptr<Widget>> roots;
    float mx=0, my=0;
    bool  ldown=false, lclick=false, lrelease=false;

    // Tooltip state
    float    tipMx    = -9999.f;
    float    tipMy    = -9999.f;
    Uint64   tipSince = 0;
    std::string tipText;
    float    tipPx    = 0.f;
    float    tipPy    = 0.f;

public:
    // Build font atlas. fontPath = TTF file, size = pixel height.
    bool Init(LightningEngine::Renderer& r,
              const char* fontPath = "assets/fonts/Roboto-Regular.ttf",
              int size = 13)
    {
        bool ok = font.Build(r, fontPath, size);
        sdlWin = r.GetWindow();
        if (ok) {
            // Update style layout metrics to match actual font
            gStyle.lineH  = font.lineHeight + 2.f;
            gStyle.titleH = gStyle.lineH + 6.f;
        }
        return ok;
    }

    void Release() { font.Release(); roots.clear(); }

    // ── Theme ─────────────────────────────────────────────────────────────
    /// Load a JSON theme file. Updates gStyle, rebuilds font if changed,
    /// populates the icon cache. Safe to call at any time (hot-reload too).
    bool LoadTheme(LightningEngine::Renderer& r, const char* jsonPath) {
        return Theme::Load(r, font, jsonPath);
    }

    /// Reload the last loaded theme (call in your update loop to hot-reload).
    bool ReloadTheme(LightningEngine::Renderer& r) {
        return Theme::Reload(r, font);
    }

    /// True if the theme JSON file changed on disk since last load.
    bool ThemeNeedsReload() const { return Theme::NeedsReload(); }

    /// Get a named icon texture (or nullptr if not loaded).
    const LightningEngine::Texture* ThemeIcon(const std::string& name) const {
        return Theme::Icon(name);
    }

    /// Get the default tint configured for this icon in theme JSON.
    Color ThemeIconTint(const std::string& name) const {
        return Theme::IconTint(name);
    }

    /// Runtime tint override for an icon (useful for white source icons).
    bool SetThemeIconTint(const std::string& name, Color c) {
        return Theme::SetIconTint(name, c);
    }

    /// Registers a callback capable of rasterizing SVG into RGBA pixels.
    void SetThemeSVGRasterizer(Theme::SVGRasterizerFn fn) {
        Theme::SetSVGRasterizer(std::move(fn));
    }

    template<typename T, typename... Args>
    T* AddRoot(Args&&... args)
    {
        auto w = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = w.get();
        roots.push_back(std::move(w));
        return ptr;
    }

    void ClearRoots() { roots.clear(); captured = nullptr; focused = nullptr; }

    // Bring widget to the front of the render stack (highest zOrder).
    void BringToFront(Widget* w)
    {
        int maxZ = 0;
        for (auto& r : roots) maxZ = std::max(maxZ, r->zOrder);
        w->zOrder = maxZ + 1;
    }

    // Push widget behind all others.
    void SendToBack(Widget* w) { w->zOrder = 0; }

    // ── Keyboard shortcut registry ────────────────────────────────────────
    // Shortcuts are global: they fire even when a widget has keyboard focus.
    // mods: bitmask of SDL_KMOD_CTRL / SDL_KMOD_SHIFT / SDL_KMOD_ALT, or 0.
    void AddShortcut(SDL_Scancode key, SDL_Keymod mods,
                     std::function<void()> cb, const char* desc = "")
    {
        shortcuts.push_back({ key, mods, desc ? desc : "", std::move(cb) });
    }

    void RemoveShortcut(SDL_Scancode key, SDL_Keymod mods)
    {
        shortcuts.erase(std::remove_if(shortcuts.begin(), shortcuts.end(),
            [key, mods](const Shortcut& s){ return s.key == key && s.mods == mods; }),
            shortcuts.end());
    }

    void ClearShortcuts() { shortcuts.clear(); }

    void ProcessInput(LightningEngine::InputManager& im)
    {
        mx       = im.GetMouseX();
        my       = im.GetMouseY();
        ldown    = im.IsMouseDown(1);
        lclick   = im.IsMousePressed(1);
        lrelease = im.IsMouseReleased(1);
        scrollY  = im.GetScrollWheelY();

        if (captured) {
            captured->ProcessInput(mx, my, ldown, lclick, lrelease, this, 0.f, 0.f);
            if (!ldown) captured = nullptr;
            return;
        }

        // Forward text + navigation keys to focused widget
        if (focused) {
            const std::string& txt = im.GetTextInput();
            bool bs = im.HasDeleteBack();
            if (!txt.empty() || bs)
                focused->ReceiveText(txt, bs);

            static constexpr SDL_Scancode kNavKeys[] = {
                SDL_SCANCODE_LEFT,  SDL_SCANCODE_RIGHT,
                SDL_SCANCODE_UP,    SDL_SCANCODE_DOWN,
                SDL_SCANCODE_HOME,  SDL_SCANCODE_END,
                SDL_SCANCODE_RETURN,SDL_SCANCODE_TAB,
                SDL_SCANCODE_PAGEUP,SDL_SCANCODE_PAGEDOWN,
                SDL_SCANCODE_DELETE,
                SDL_SCANCODE_A, SDL_SCANCODE_C,
                SDL_SCANCODE_V, SDL_SCANCODE_X,
                SDL_SCANCODE_Z
            };
            for (SDL_Scancode k : kNavKeys)
                if (im.IsKeyPressed(k)) focused->ReceiveKey(k);
        }

        // Input dispatched highest-zOrder first (front-most widget gets priority)
        sortedInputIndices();
        for (int i : inputOrder) {
            if (roots[i]->ProcessInput(mx, my, ldown, lclick, lrelease, this, 0.f, 0.f))
                break;
        }

        // Tooltip tracking
        if (mx != tipMx || my != tipMy) { tipMx=mx; tipMy=my; tipSince=SDL_GetTicks(); tipText.clear(); }
        else if (tipSince > 0 && (SDL_GetTicks() - tipSince) > 600 && tipText.empty()) {
            for (int i : inputOrder) {
                const std::string* t = roots[i]->FindTooltip(mx, my, 0.f, 0.f);
                if (t && !t->empty()) { tipText=*t; tipPx=mx+14.f; tipPy=my-2.f; break; }
            }
        }

        // Global keyboard shortcuts
        if (!shortcuts.empty()) {
            SDL_Keymod cur  = SDL_GetModState();
            bool hasCtrl  = (cur & SDL_KMOD_CTRL)  != 0;
            bool hasShift = (cur & SDL_KMOD_SHIFT) != 0;
            bool hasAlt   = (cur & SDL_KMOD_ALT)   != 0;
            for (auto& sc : shortcuts) {
                if (!im.IsKeyPressed(sc.key)) continue;
                bool wantCtrl  = (sc.mods & SDL_KMOD_CTRL)  != 0;
                bool wantShift = (sc.mods & SDL_KMOD_SHIFT) != 0;
                bool wantAlt   = (sc.mods & SDL_KMOD_ALT)   != 0;
                if (hasCtrl == wantCtrl && hasShift == wantShift && hasAlt == wantAlt)
                    sc.callback();
            }
        }
    }

    void Render(LightningEngine::Renderer& r)
    {
        if (!font.IsValid()) return;
        r.BeginScreenSpace();

        // Render lowest-zOrder first, highest last (painter's algorithm)
        sortedRenderIndices();
        for (int i : renderOrder) roots[i]->Render(r, font, 0.f, 0.f);

        // Draw tooltip always on top
        if (!tipText.empty()) {
            float tw = font.MeasureW(tipText.c_str()) + 10.f;
            float th = font.GlyphH() + 6.f;
            r.SetDrawColor(gStyle.menuBg.r, gStyle.menuBg.g, gStyle.menuBg.b);
            r.FillRect(tipPx, tipPy, tw, th);
            r.SetDrawColor(gStyle.menuBorder.r, gStyle.menuBorder.g, gStyle.menuBorder.b);
            r.DrawRect(tipPx, tipPy, tw, th);
            r.SetDrawColor(gStyle.textNormal.r, gStyle.textNormal.g, gStyle.textNormal.b);
            font.DrawText(r, tipText.c_str(), tipPx+5.f, tipPy+3.f);
        }

        renderToasts(r);
        r.EndScreenSpace();
    }

public:
    // Show a temporary notification toast at the bottom-right of the screen.
    // durationSec: how long the toast is displayed before auto-dismissal.
    void ShowToast(const char* msg, float durationSec = 3.f)
    {
        Uint64 now = SDL_GetTicks();
        toasts.push_back({ msg ? msg : "", now + (Uint64)(durationSec * 1000.f) });
    }

private:
    struct Shortcut {
        SDL_Scancode          key;
        SDL_Keymod            mods;
        std::string           description;
        std::function<void()> callback;
    };
    std::vector<Shortcut> shortcuts;

    struct ToastEntry { std::string msg; Uint64 expireAt; };
    std::vector<ToastEntry> toasts;

    void renderToasts(LightningEngine::Renderer& r)
    {
        Uint64 now = SDL_GetTicks();
        toasts.erase(std::remove_if(toasts.begin(), toasts.end(),
            [now](const ToastEntry& t){ return now >= t.expireAt; }), toasts.end());
        if (toasts.empty() || !font.IsValid()) return;

        float pad   = 10.f;
        float lineH = font.GlyphH() + 8.f;
        float tw_max = 0.f;
        for (const auto& t : toasts)
            tw_max = std::max(tw_max, font.MeasureW(t.msg.c_str()));
        float boxW = tw_max + pad * 2.f;

        int sw = 1280, sh = 720;
        if (sdlWin) SDL_GetWindowSize(sdlWin, &sw, &sh);

        float bx = sw - boxW - 12.f;
        float by = sh - (float)toasts.size() * (lineH + 4.f) - 12.f;

        for (const auto& t : toasts) {
            Uint64 remaining = t.expireAt > now ? t.expireAt - now : 0;
            Uint8  alpha     = (remaining < 500) ? (Uint8)(remaining * 255 / 500) : 220;
            r.SetDrawColor(30, 34, 50, alpha);
            r.FillRect(bx, by, boxW, lineH);
            r.SetDrawColor(60, 70, 110, alpha);
            r.DrawRect(bx, by, boxW, lineH);
            r.SetDrawColor(200, 200, 215, alpha);
            font.DrawText(r, t.msg.c_str(), bx + pad, by + 4.f);
            by += lineH + 4.f;
        }
    }

    // Sorted index arrays (rebuilt each frame — roots rarely change)
    mutable std::vector<int> renderOrder;   // ascending  zOrder
    mutable std::vector<int> inputOrder;    // descending zOrder

    void sortedRenderIndices() const
    {
        renderOrder.resize(roots.size());
        for (int i = 0; i < (int)roots.size(); i++) renderOrder[i] = i;
        std::stable_sort(renderOrder.begin(), renderOrder.end(),
            [this](int a, int b){ return roots[a]->zOrder < roots[b]->zOrder; });
    }

    void sortedInputIndices() const
    {
        inputOrder.resize(roots.size());
        for (int i = 0; i < (int)roots.size(); i++) inputOrder[i] = i;
        std::stable_sort(inputOrder.begin(), inputOrder.end(),
            [this](int a, int b){ return roots[a]->zOrder > roots[b]->zOrder; });
    }
};

// Defined here after TitanUI is complete — resolves forward declarations in Widget.h

// Safely clear focused/captured when a widget is destroyed.
inline void ClearUIFocusIfMatch(TitanUI* ui, Widget* w)
{
    if (!ui) return;
    if (ui->focused == w) {
        ui->focused = nullptr;
        if (ui->sdlWin) SDL_StopTextInput(ui->sdlWin);
    }
    if (ui->captured == w) ui->captured = nullptr;
}

// Widget destructor — defined here so ClearUIFocusIfMatch is available.
inline Widget::~Widget()
{
    if (uiContext) ClearUIFocusIfMatch(uiContext, this);
    // children unique_ptrs destroyed after this body, their dtors fire in turn
}

inline void SetUICapture(TitanUI* ui, Widget* w)
{
    if (ui) ui->captured = w;
}

inline float GetUIScrollY(TitanUI* ui)
{
    return ui ? ui->scrollY : 0.f;
}

inline void SetUIFocus(TitanUI* ui, Widget* w)
{
    if (!ui) return;
    if (ui->focused && ui->focused != w)
        ui->focused->OnLostFocus();  // proper lost-focus notification
    ui->focused = w;
    if (w) w->uiContext = ui;  // track TitanUI for destructor cleanup
    if (ui->sdlWin) {
        if (w) SDL_StartTextInput(ui->sdlWin);
        else   SDL_StopTextInput(ui->sdlWin);
    }
}

// Defined here after TitanUI is complete — MenuBar and other widgets call this
// instead of ui->BringToFront(this) directly (avoids incomplete-type error).
inline void Widget::DoBringToFront(TitanUI* ui)
{
    if (ui) ui->BringToFront(this);
}

} // namespace Titan
