#include "ui/Widgets.h"

#include <LovyanGFX.hpp>

#include "ui/Theme.h"

namespace ui {

namespace {

// Only Font2 (body) and Font4 (headings) are used — Font6/7/8 are digit-only
// seven-segment faces and would silently drop letters.
const lgfx::IFont* fontFor(uint8_t f) {
    switch (f) {
        case 4:  return &fonts::Font4;
        case 2:
        default: return &fonts::Font2;
    }
}

}  // namespace

int16_t drawText(LGFX_Sprite& g, const String& text, int16_t x, int16_t y,
                 int16_t w, uint16_t color, uint8_t font, int16_t lineH,
                 bool centered) {
    g.setFont(fontFor(font));
    g.setTextColor(color);
    g.setTextDatum(centered ? textdatum_t::top_center : textdatum_t::top_left);
    const int16_t anchorX = centered ? (int16_t)(x + w / 2) : x;

    String line;
    int16_t cy = y;

    auto flush = [&]() {
        if (line.length()) g.drawString(line, anchorX, cy);
        cy += lineH;
        line = "";
    };

    int start = 0;
    const int len = text.length();
    while (start <= len) {
        int nl = text.indexOf('\n', start);
        int sp = text.indexOf(' ', start);
        int brk = -1;
        bool hardBreak = false;
        if (nl >= 0 && (sp < 0 || nl < sp)) {
            brk = nl;
            hardBreak = true;
        } else {
            brk = sp;
        }

        String word = (brk >= 0) ? text.substring(start, brk) : text.substring(start);
        String trial = line.length() ? line + " " + word : word;
        if (g.textWidth(trial) <= w) {
            line = trial;
        } else {
            flush();
            line = word;
        }

        if (hardBreak) flush();
        if (brk < 0) break;
        start = brk + 1;
    }
    flush();
    return cy;
}

void drawCentered(LGFX_Sprite& g, const String& text, int16_t cx, int16_t y,
                  uint16_t color, uint8_t font) {
    g.setFont(fontFor(font));
    g.setTextColor(color);
    g.setTextDatum(textdatum_t::top_center);
    g.drawString(text, cx, y);
}

void drawPanel(LGFX_Sprite& g, const Rect& r) {
    g.fillRoundRect(r.x, r.y, r.w, r.h, 4, theme::PANEL);
    g.drawRoundRect(r.x, r.y, r.w, r.h, 4, theme::FRAME);
}

void drawButton(LGFX_Sprite& g, const Rect& r, const String& label,
                bool highlight, bool enabled) {
    const uint16_t fill = highlight ? theme::ACCENT : theme::PANEL;
    const uint16_t edge = enabled ? theme::FRAME : theme::INK_DIM;
    uint16_t ink = highlight ? theme::ACCENT_INK : theme::INK;
    if (!enabled) ink = theme::INK_DIM;

    g.fillRoundRect(r.x, r.y, r.w, r.h, 4, fill);
    g.drawRoundRect(r.x, r.y, r.w, r.h, 4, edge);
    g.setFont(&fonts::Font2);
    g.setTextColor(ink);
    g.setTextDatum(textdatum_t::middle_center);
    g.drawString(label, r.x + r.w / 2, r.y + r.h / 2);
}

void MenuList::add(const String& label, bool enabled) {
    if (n_ >= kMax) return;
    labels_[n_] = label;
    enabled_[n_] = enabled;
    ++n_;
}

void MenuList::setBounds(int16_t x, int16_t y, int16_t w, int16_t rowH) {
    x_ = x;
    y_ = y;
    w_ = w;
    rowH_ = rowH;
}

void MenuList::render(LGFX_Sprite& g, int selected) const {
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::middle_left);
    for (int i = 0; i < n_; ++i) {
        const int16_t ry = y_ + i * rowH_;
        const bool sel = (i == selected);
        const uint16_t fill = sel ? theme::ACCENT : theme::PANEL;
        uint16_t ink = sel ? theme::ACCENT_INK : theme::INK;
        if (!enabled_[i]) ink = theme::INK_DIM;

        g.fillRoundRect(x_, ry, w_, rowH_ - 4, 4, fill);
        g.drawRoundRect(x_, ry, w_, rowH_ - 4, 4, theme::FRAME);

        g.setTextColor(ink);
        String row = String(i + 1) + ".  " + labels_[i];
        g.drawString(row, x_ + 10, ry + (rowH_ - 4) / 2);
    }
}

int MenuList::hitTest(int16_t px, int16_t py) const {
    for (int i = 0; i < n_; ++i) {
        const int16_t ry = y_ + i * rowH_;
        if (!enabled_[i]) continue;
        if (px >= x_ && px < x_ + w_ && py >= ry && py < ry + rowH_ - 4) return i;
    }
    return -1;
}

// ---- on-screen keyboard --------------------------------------------------
namespace keyboard {

namespace {
constexpr int16_t ROW_H = 30;
constexpr int16_t GAP = 2;
const char* const ROW0 = "QWERTYUIOP";   // 10 keys, full width
const char* const ROW1 = "ASDFGHJKL";    // 9 keys, inset
const char* const ROW2 = "ZXCVBNM";      // 7 keys + backspace
constexpr int16_t SPACE_W = 180;         // row 3: space | DONE
}  // namespace

static void drawKey(LGFX_Sprite& g, int16_t x, int16_t y, int16_t w, int16_t h,
                    const char* label, bool wide) {
    g.fillRoundRect(x + GAP, y + GAP, w - 2 * GAP, h - 2 * GAP, 3, theme::PANEL);
    g.drawRoundRect(x + GAP, y + GAP, w - 2 * GAP, h - 2 * GAP, 3, theme::FRAME);
    // letters use a mid-size proportional face so they don't crowd the border
    if (wide) g.setFont(&fonts::Font2);
    else      g.setFont(&fonts::FreeSansBold9pt7b);
    g.setTextColor(theme::INK);
    g.setTextDatum(textdatum_t::middle_center);
    g.drawString(label, x + w / 2, y + h / 2 + (wide ? 0 : 1));
}

void render(LGFX_Sprite& g) {
    const int16_t top = kTop;
    g.fillRect(0, top, 320, kBottom - top, theme::BG);

    for (int i = 0; i < 10; ++i) {
        char s[2] = {ROW0[i], 0};
        drawKey(g, i * 32, top, 32, ROW_H, s, false);
    }
    for (int i = 0; i < 9; ++i) {
        char s[2] = {ROW1[i], 0};
        drawKey(g, 16 + i * 32, top + ROW_H, 32, ROW_H, s, false);
    }
    for (int i = 0; i < 7; ++i) {
        char s[2] = {ROW2[i], 0};
        drawKey(g, 16 + i * 32, top + 2 * ROW_H, 32, ROW_H, s, false);
    }
    drawKey(g, 16 + 7 * 32, top + 2 * ROW_H, 320 - (16 + 7 * 32), ROW_H, "DEL", true);
    drawKey(g, 0, top + 3 * ROW_H, SPACE_W, ROW_H, "space", true);
    drawKey(g, SPACE_W, top + 3 * ROW_H, 320 - SPACE_W, ROW_H, "DONE", true);
}

char hitTest(int16_t px, int16_t py) {
    const int16_t top = kTop;
    if (py < top || py >= kBottom) return 0;
    int row = (py - top) / ROW_H;
    if (row > 3) row = 3;                       // clamp the sliver at the bottom

    if (row == 0) {
        int i = px / 32;
        if (i >= 0 && i < 10) return ROW0[i];
    } else if (row == 1) {
        if (px < 16) return 0;
        int i = (px - 16) / 32;
        if (i >= 0 && i < 9) return ROW1[i];
    } else if (row == 2) {
        if (px < 16) return 0;
        if (px >= 16 + 7 * 32) return '\b';
        int i = (px - 16) / 32;
        if (i >= 0 && i < 7) return ROW2[i];
    } else {  // row 3
        return (px < SPACE_W) ? ' ' : '\n';
    }
    return 0;
}

}  // namespace keyboard

}  // namespace ui
