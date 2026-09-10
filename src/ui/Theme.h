// Visual constants. One look only (no light/dark) — a green-phosphor nod to the
// Apple II MECC original, warmed up slightly so it reads well on an IPS panel.
#pragma once
#include <stdint.h>

namespace theme {

// RGB565
constexpr uint16_t BG        = 0x0000;  // black
constexpr uint16_t PANEL     = 0x1082;  // near-black slate for framed panels
constexpr uint16_t INK       = 0x9FF3;  // primary phosphor green
constexpr uint16_t INK_DIM   = 0x5B0A;  // muted green for secondary text
constexpr uint16_t ACCENT    = 0xFD20;  // amber — selections, highlights
constexpr uint16_t ACCENT_INK= 0x0000;  // text on an amber fill
constexpr uint16_t WARN      = 0xF8A5;  // red-orange — danger, death
constexpr uint16_t FRAME     = 0x4B4C;  // panel borders

// Layout metrics (screen is 320x240 landscape).
constexpr int16_t MARGIN     = 8;
constexpr int16_t LINE_H     = 18;   // Font2 body line height
constexpr int16_t ROW_H      = 30;   // touch menu row height
constexpr int16_t STATUS_H   = 20;   // top status bar

}  // namespace theme
