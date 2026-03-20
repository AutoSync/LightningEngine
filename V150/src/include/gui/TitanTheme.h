// TitanTheme.h — JSON-driven theme, icon and font loader for Titan UI.
//
// - Supports theme colors/layout/font from JSON.
// - Supports icons in PNG and SVG specifications.
// - Supports per-icon tint color (runtime editable).
// - Keeps backward compatibility with "icons": { "name": "path.png" }.
//
// SVG notes:
// - This file includes a pluggable SVG rasterizer callback.
// - If no rasterizer is provided, SVG entries fall back to "png"/"fallbackPng".
// - This keeps the project lightweight while allowing scalable SVG pipeline.
#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <SDL3/SDL.h>
#include "TitanStyle.h"
#include "TitanFont.h"
#include "../Texture.h"
#include "../Renderer.h"

namespace Titan {

struct IconAsset {
    LightningEngine::Texture texture;

    std::string pngPath;
    std::string svgPath;

    Color tint = { 255, 255, 255, 255 };
    int   baseSize = 24;
    bool  preferSvg = true;

    bool HasTexture() const { return texture.IsValid(); }

    void Release() {
        texture.Release();
        texture = LightningEngine::Texture{};
    }
};

struct IconCache {
    std::unordered_map<std::string, IconAsset> items;

    void Clear() {
        for (auto& kv : items) kv.second.Release();
        items.clear();
    }

    IconAsset* Find(const std::string& name) {
        auto it = items.find(name);
        return (it != items.end()) ? &it->second : nullptr;
    }

    const IconAsset* Find(const std::string& name) const {
        auto it = items.find(name);
        return (it != items.end()) ? &it->second : nullptr;
    }

    const LightningEngine::Texture* GetTexture(const std::string& name) const {
        const IconAsset* a = Find(name);
        return (a && a->texture.IsValid()) ? &a->texture : nullptr;
    }

    Color GetTint(const std::string& name) const {
        const IconAsset* a = Find(name);
        return a ? a->tint : Color{ 255,255,255,255 };
    }

    bool SetTint(const std::string& name, Color c) {
        IconAsset* a = Find(name);
        if (!a) return false;
        a->tint = c;
        return true;
    }

    void Upsert(const std::string& name, IconAsset a) {
        auto it = items.find(name);
        if (it != items.end()) it->second.Release();
        items[name] = std::move(a);
    }
};

class Theme {
public:
    // SVG -> RGBA rasterizer hook.
    // Return true and fill rgbaOut/w/hOut if rasterization succeeds.
    using SVGRasterizerFn = std::function<bool(const std::string& svgPath,
                                               int targetPx,
                                               std::vector<unsigned char>& rgbaOut,
                                               int& wOut,
                                               int& hOut)>;

    static void SetSVGRasterizer(SVGRasterizerFn fn) { svgRasterizer = std::move(fn); }

    static bool Load(LightningEngine::Renderer& r, TitanFont& font,
                     const char* jsonPath)
    {
        size_t sz  = 0;
        void*  raw = SDL_LoadFile(jsonPath, &sz);
        if (!raw) return false;

        std::string src(static_cast<const char*>(raw), sz);
        SDL_free(raw);

        SDL_PathInfo pi{};
        SDL_GetPathInfo(jsonPath, &pi);
        s_lastMod = static_cast<Uint64>(pi.modify_time);
        s_path    = jsonPath;

        Style       next          = gStyle;
        bool        lineHSet      = false;
        bool        titleHSet     = false;
        std::string newFontPath;
        int         newFontSize   = font.pixelSize;
        bool        fontChanged   = false;

        s_icons.Clear();
        ParseJSON(src, r, next, lineHSet, titleHSet,
                  newFontPath, newFontSize, fontChanged);

        if (fontChanged) {
            const char* fp = newFontPath.empty()
                             ? "assets/fonts/Roboto-Regular.ttf"
                             : newFontPath.c_str();
            font.Build(r, fp, newFontSize);
        }

        if (!lineHSet)  next.lineH  = font.lineHeight + 2.f;
        if (!titleHSet) next.titleH = next.lineH + 6.f;

        gStyle = next;
        if (onThemeChanged) onThemeChanged();
        return true;
    }

    static bool Reload(LightningEngine::Renderer& r, TitanFont& font) {
        if (s_path.empty()) return false;
        return Load(r, font, s_path.c_str());
    }

    static bool NeedsReload() {
        if (s_path.empty()) return false;
        SDL_PathInfo pi{};
        SDL_GetPathInfo(s_path.c_str(), &pi);
        return static_cast<Uint64>(pi.modify_time) != s_lastMod;
    }

    static IconCache&                          Icons() { return s_icons; }
    static bool                                IsLoaded() { return !s_path.empty(); }
    static const std::string&                  CurrentPath() { return s_path; }
    static inline std::function<void()>        onThemeChanged;

    static const LightningEngine::Texture* Icon(const std::string& name) {
        return s_icons.GetTexture(name);
    }

    static Color IconTint(const std::string& name) {
        return s_icons.GetTint(name);
    }

    static bool SetIconTint(const std::string& name, Color c) {
        return s_icons.SetTint(name, c);
    }

private:
    static inline std::string      s_path;
    static inline Uint64           s_lastMod = 0;
    static inline IconCache        s_icons;
    static inline SVGRasterizerFn  svgRasterizer;

    struct Scanner {
        const char* p;
        const char* end;

        explicit Scanner(const std::string& s)
            : p(s.data()), end(s.data() + s.size()) {}

        void skipWS() {
            while (p < end) {
                char c = *p;
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { ++p; continue; }
                if (c == '/' && p+1 < end && *(p+1) == '/') {
                    p += 2;
                    while (p < end && *p != '\n') ++p;
                    continue;
                }
                if (c == '/' && p+1 < end && *(p+1) == '*') {
                    p += 2;
                    while (p+1 < end && !(*p == '*' && *(p+1) == '/')) ++p;
                    if (p+1 < end) p += 2;
                    continue;
                }
                break;
            }
        }

        bool atEnd() const { return p >= end; }
        char peek()  const { return (p < end) ? *p : '\0'; }
        void advance()     { if (p < end) ++p; }

        bool expect(char c) {
            skipWS();
            if (peek() == c) { advance(); return true; }
            return false;
        }

        std::string readString() {
            skipWS();
            if (peek() != '"') return {};
            advance();
            std::string out;
            out.reserve(32);
            while (p < end && *p != '"') {
                if (*p == '\\') {
                    ++p;
                    if (p < end) {
                        switch (*p) {
                        case 'n':  out += '\n'; break;
                        case 't':  out += '\t'; break;
                        case 'r':  out += '\r'; break;
                        default:   out += *p;   break;
                        }
                        ++p;
                    }
                } else {
                    out += *p++;
                }
            }
            if (p < end) advance();
            return out;
        }

        float readFloat() {
            skipWS();
            char* ep = nullptr;
            float v  = strtof(p, &ep);
            if (ep && ep != p) p = ep;
            return v;
        }

        int readInt() { return static_cast<int>(readFloat()); }

        bool readColorArray(Uint8& cr, Uint8& cg, Uint8& cb, Uint8& ca) {
            skipWS();
            if (!expect('[')) return false;
            cr = static_cast<Uint8>(readInt()); expect(',');
            cg = static_cast<Uint8>(readInt()); expect(',');
            cb = static_cast<Uint8>(readInt());
            skipWS();
            if (peek() == ',') { advance(); ca = static_cast<Uint8>(readInt()); }
            expect(']');
            return true;
        }

        void skipValue() {
            skipWS();
            char c = peek();
            if (c == '"') { readString(); return; }
            if (c == '{' || c == '[') {
                advance();
                int depth = 1;
                while (!atEnd() && depth > 0) {
                    char ch = *p;
                    if (ch == '"') { readString(); continue; }
                    if (ch == '{' || ch == '[') { ++depth; ++p; }
                    else if (ch == '}' || ch == ']') { --depth; ++p; }
                    else ++p;
                }
                return;
            }
            while (!atEnd() && *p != ',' && *p != '}' && *p != ']' &&
                   *p != '\n' && *p != '\r') ++p;
        }
    };

    static bool endsWithNoCase(const std::string& s, const char* ext) {
        size_t n = s.size(), m = SDL_strlen(ext);
        if (n < m) return false;
        size_t off = n - m;
        for (size_t i = 0; i < m; ++i) {
            char a = static_cast<char>(std::tolower(static_cast<unsigned char>(s[off + i])));
            char b = static_cast<char>(std::tolower(static_cast<unsigned char>(ext[i])));
            if (a != b) return false;
        }
        return true;
    }

    static bool isSvgPath(const std::string& path) {
        return endsWithNoCase(path, ".svg");
    }

    static bool loadIconTextureFromSpec(LightningEngine::Renderer& r, IconAsset& a) {
        a.Release();

        if (a.preferSvg && !a.svgPath.empty() && svgRasterizer) {
            std::vector<unsigned char> rgba;
            int w = 0, h = 0;
            const int px = (a.baseSize > 0) ? a.baseSize : 24;
            if (svgRasterizer(a.svgPath, px, rgba, w, h) &&
                w > 0 && h > 0 && !rgba.empty())
            {
                a.texture = r.LoadTextureFromPixels(rgba.data(), w, h, false);
                if (a.texture.IsValid()) return true;
            }
        }

        if (!a.pngPath.empty()) {
            a.texture = r.LoadTexture(a.pngPath.c_str());
            if (a.texture.IsValid()) return true;
        }

        if (!a.preferSvg && !a.svgPath.empty() && svgRasterizer) {
            std::vector<unsigned char> rgba;
            int w = 0, h = 0;
            const int px = (a.baseSize > 0) ? a.baseSize : 24;
            if (svgRasterizer(a.svgPath, px, rgba, w, h) &&
                w > 0 && h > 0 && !rgba.empty())
            {
                a.texture = r.LoadTextureFromPixels(rgba.data(), w, h, false);
                if (a.texture.IsValid()) return true;
            }
        }

        return false;
    }

    static void applyColor(const std::string& key,
                           Uint8 r, Uint8 g, Uint8 b, Uint8 a,
                           Style& s)
    {
#define MAP(field) if (key == #field) { s.field = {r,g,b,a}; return; }
        MAP(winBg)        MAP(panelBg)     MAP(panelHeader)  MAP(panelBorder)
        MAP(btnNormal)    MAP(btnHover)    MAP(btnActive)    MAP(btnBorder)
        MAP(btnText)      MAP(textNormal)  MAP(textDim)      MAP(textBright)
        MAP(textAccent)   MAP(textGreen)   MAP(textRed)      MAP(selection)
        MAP(highlight)    MAP(sliderTrack) MAP(sliderFill)   MAP(sliderThumb)
        MAP(separator)    MAP(menuBg)      MAP(menuBorder)   MAP(statusBg)
        MAP(checkMark)
#undef MAP
    }

    static IconAsset parseIconValue(Scanner& s, const std::string& iconName) {
        IconAsset out;

        s.skipWS();
        if (s.peek() == '"') {
            std::string path = s.readString();
            if (isSvgPath(path)) {
                out.svgPath   = path;
                out.preferSvg = true;
            } else {
                out.pngPath   = path;
                out.preferSvg = false;
            }
            return out;
        }

        if (s.peek() != '{') {
            s.skipValue();
            return out;
        }

        s.expect('{');
        while (!s.atEnd()) {
            s.skipWS();
            if (s.peek() == '}') { s.advance(); break; }
            if (s.peek() == ',') { s.advance(); continue; }

            std::string k = s.readString();
            s.expect(':');
            s.skipWS();

            if (k == "path") {
                std::string path = s.readString();
                if (isSvgPath(path)) {
                    out.svgPath = path;
                    out.preferSvg = true;
                } else {
                    out.pngPath = path;
                    out.preferSvg = false;
                }
            } else if (k == "png" || k == "fallbackPng") {
                out.pngPath = s.readString();
            } else if (k == "svg") {
                out.svgPath = s.readString();
                out.preferSvg = true;
            } else if (k == "size") {
                out.baseSize = s.readInt();
            } else if (k == "prefer") {
                std::string pref = s.readString();
                out.preferSvg = (pref == "svg" || pref == "SVG");
            } else if (k == "tint") {
                Uint8 r=255,g=255,b=255,a=255;
                if (s.readColorArray(r,g,b,a)) out.tint = {r,g,b,a};
            } else {
                s.skipValue();
            }
        }

        if (!out.svgPath.empty() && out.pngPath.empty()) out.preferSvg = true;
        if (out.svgPath.empty() && !out.pngPath.empty()) out.preferSvg = false;

        if (out.baseSize <= 0) out.baseSize = 24;

        (void)iconName;
        return out;
    }

    static void ParseJSON(const std::string& src,
                          LightningEngine::Renderer& r,
                          Style& out,
                          bool& lineHSet,
                          bool& titleHSet,
                          std::string& fontPath,
                          int& fontSize,
                          bool& fontChanged)
    {
        Scanner s(src);
        s.expect('{');

        while (!s.atEnd()) {
            s.skipWS();
            if (s.peek() == '}') break;
            if (s.peek() == ',') { s.advance(); continue; }

            std::string topKey = s.readString();
            s.expect(':');
            s.skipWS();

            if (topKey == "name") {
                s.readString();

            } else if (topKey == "colors") {
                s.expect('{');
                while (!s.atEnd()) {
                    s.skipWS();
                    if (s.peek() == '}') { s.advance(); break; }
                    if (s.peek() == ',') { s.advance(); continue; }
                    std::string ckey = s.readString();
                    s.expect(':');
                    Uint8 cr = 0, cg = 0, cb = 0, ca = 255;
                    if (s.readColorArray(cr, cg, cb, ca))
                        applyColor(ckey, cr, cg, cb, ca, out);
                    else
                        s.skipValue();
                }

            } else if (topKey == "layout") {
                s.expect('{');
                while (!s.atEnd()) {
                    s.skipWS();
                    if (s.peek() == '}') { s.advance(); break; }
                    if (s.peek() == ',') { s.advance(); continue; }
                    std::string lk = s.readString();
                    s.expect(':');
                    float v = s.readFloat();
                    if      (lk == "padding") { out.padding = v; }
                    else if (lk == "lineH"  ) { out.lineH   = v; lineHSet  = (v > 0.f); }
                    else if (lk == "titleH" ) { out.titleH  = v; titleHSet = (v > 0.f); }
                }

            } else if (topKey == "font") {
                s.expect('{');
                while (!s.atEnd()) {
                    s.skipWS();
                    if (s.peek() == '}') { s.advance(); break; }
                    if (s.peek() == ',') { s.advance(); continue; }
                    std::string fk = s.readString();
                    s.expect(':');
                    s.skipWS();
                    if (fk == "path") {
                        fontPath    = s.readString();
                        fontChanged = true;
                    } else if (fk == "size") {
                        fontSize    = s.readInt();
                        fontChanged = true;
                    } else {
                        s.skipValue();
                    }
                }

            } else if (topKey == "icons") {
                s.expect('{');
                while (!s.atEnd()) {
                    s.skipWS();
                    if (s.peek() == '}') { s.advance(); break; }
                    if (s.peek() == ',') { s.advance(); continue; }

                    std::string iname = s.readString();
                    s.expect(':');

                    if (iname.empty()) { s.skipValue(); continue; }

                    IconAsset a = parseIconValue(s, iname);

                    if (a.pngPath.empty() && a.svgPath.empty()) {
                        continue;
                    }

                    loadIconTextureFromSpec(r, a);
                    s_icons.Upsert(iname, std::move(a));
                }

            } else {
                s.skipValue();
            }
        }
    }
};

} // namespace Titan
