// TitanStyle.h — Titan GUI dark editor theme.
#pragma once
#include <SDL3/SDL.h>

namespace Titan {

struct Color {
    Uint8 r=0, g=0, b=0, a=255;
    Color() = default;
    Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a=255) : r(r), g(g), b(b), a(a) {}
};

struct Style {
    // Backgrounds
    Color winBg        = {  20,  20,  26, 255 };
    Color panelBg      = {  28,  28,  36, 255 };
    Color panelHeader  = {  22,  22,  30, 255 };
    Color panelBorder  = {  50,  50,  65, 255 };

    // Buttons
    Color btnNormal    = {  48,  48,  62, 255 };
    Color btnHover     = {  62,  66,  90, 255 };
    Color btnActive    = {  38,  84, 155, 255 };
    Color btnBorder    = {  70,  70,  90, 255 };
    Color btnText      = { 215, 215, 220, 255 };

    // Text
    Color textNormal   = { 210, 210, 215, 255 };
    Color textDim      = { 120, 120, 132, 255 };
    Color textBright   = { 255, 255, 255, 255 };
    Color textAccent   = {  90, 160, 255, 255 };
    Color textGreen    = {  90, 210, 110, 255 };
    Color textRed      = { 235,  75,  75, 255 };

    // Selection / highlight
    Color selection    = {  38,  88, 158, 200 };
    Color highlight    = {  50, 110, 195, 255 };

    // Slider
    Color sliderTrack  = {  40,  40,  52, 255 };
    Color sliderFill   = {  50, 105, 195, 255 };
    Color sliderThumb  = {  85, 148, 240, 255 };

    // Misc
    Color separator    = {  48,  48,  62, 255 };
    Color menuBg       = {  36,  36,  46, 255 };
    Color menuBorder   = {  60,  60,  76, 255 };
    Color statusBg     = {  22,  22,  28, 255 };
    Color checkMark    = {  70, 185, 110, 255 };

    // Layout (updated by EditorApp after font is built)
    float titleH   = 20.f;  // panel title bar height
    float lineH    = 17.f;  // row height in lists / between labels
    float padding  = 5.f;
};

inline Style gStyle;

} // namespace Titan
