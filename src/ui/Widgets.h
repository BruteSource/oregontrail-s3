// Small immediate-mode-ish drawing helpers shared by every screen. Device-only
// (they draw through LovyanGFX). Screens keep their own hit-test rects.
#pragma once
#include <Arduino.h>
#include <stdint.h>

#include <LovyanGFX.hpp>

namespace ui {

// Plain aggregate — brace-init as {x, y, w, h}; declare as `Rect r{};` to zero.
struct Rect {
    int16_t x, y, w, h;
    bool contains(int16_t px, int16_t py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

// Word-wrapped text in [x, x+w). Returns the y coordinate just past the last
// line drawn, so callers can flow content.
int16_t drawText(LGFX_Sprite& g, const String& text, int16_t x, int16_t y,
                 int16_t w, uint16_t color, uint8_t font = 2,
                 int16_t lineH = 18, bool centered = false);

// Centered single line.
void drawCentered(LGFX_Sprite& g, const String& text, int16_t cx, int16_t y,
                  uint16_t color, uint8_t font = 2);

// Framed slate panel.
void drawPanel(LGFX_Sprite& g, const Rect& r);

// A labelled button. `highlight` fills it with the accent color.
void drawButton(LGFX_Sprite& g, const Rect& r, const String& label,
                bool highlight = false, bool enabled = true);

// Vertical stack of tappable rows — the replacement for the original's numbered
// command menus. Owns up to kMax short labels.
class MenuList {
public:
    static constexpr int kMax = 10;

    void clear() { n_ = 0; }
    void add(const String& label, bool enabled = true);
    int  count() const { return n_; }

    void setBounds(int16_t x, int16_t y, int16_t w, int16_t rowH);

    // Draws numbered rows. `selected` row (if >= 0) is highlighted.
    void render(LGFX_Sprite& g, int selected = -1) const;

    // Row index at (px, py), or -1. Disabled rows never hit.
    int hitTest(int16_t px, int16_t py) const;

private:
    String  labels_[kMax];
    bool    enabled_[kMax] = {};
    int     n_ = 0;
    int16_t x_ = 0, y_ = 0, w_ = 0, rowH_ = 30;
};

// On-screen QWERTY. Fixed to the bottom of a 320-wide screen. hitTest returns
// the produced character: an uppercase letter, ' ' (space), '\b' (backspace),
// '\n' (done), or 0 for a miss.
namespace keyboard {
// Sits in y [kTop, kTop+120). Kept clear of the bottom ~26 px, which is outside
// the touch calibration span and unreliable.
constexpr int16_t kTop = 78;
constexpr int16_t kBottom = 198;
void render(LGFX_Sprite& g);
char hitTest(int16_t px, int16_t py);
}  // namespace keyboard

}  // namespace ui
