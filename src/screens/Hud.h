// Shared top status strip for the on-trail screens: date, weather, odometer.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "game/Sim.h"
#include "hw/Battery.h"
#include "ui/Theme.h"

namespace hud {

// Small battery pill, right-anchored, with its right edge at (rx, cy).
inline void drawBattery(LGFX_Sprite& g, int16_t rx, int16_t cy) {
    const int16_t w = 18, h = 9;
    const int16_t x = rx - w, y = cy - h / 2;
    const int pc = battery::percent();
    const int chg = battery::charging();
    uint16_t c = chg == 1 ? theme::INK : pc <= 15 ? theme::WARN : theme::INK_DIM;
    g.drawRect(x, y, w, h, c);
    g.fillRect(x + w, y + 2, 2, h - 4, c);
    const int fill = (w - 4) * pc / 100;
    if (fill > 0) g.fillRect(x + 2, y + 2, fill, h - 4, c);
    if (chg == 1) {  // little bolt
        g.drawLine(x + w / 2 + 1, y + 1, x + w / 2 - 2, y + h / 2, theme::ACCENT);
        g.drawLine(x + w / 2 - 2, y + h / 2, x + w / 2 + 1, y + h - 1, theme::ACCENT);
    }
}

inline const char* skyWord(game::Sky s) {
    switch (s) {
        case game::Sky::Rain: return "Rain";
        case game::Sky::Snow: return "Snow";
        default:              return "Clear";
    }
}

// Draws the strip at y 0..STATUS_H and returns the y below it.
inline int16_t drawTrailStatus(LGFX_Sprite& g) {
    const game::Sim& s = game::sim;
    g.fillRect(0, 0, 320, theme::STATUS_H, theme::PANEL);
    g.drawFastHLine(0, theme::STATUS_H, 320, theme::FRAME);

    g.setFont(&fonts::Font2);
    g.setTextColor(theme::INK);
    g.setTextDatum(textdatum_t::middle_left);
    static const char* kM[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char left[24];
    snprintf(left, sizeof(left), "%s %d", kM[(s.monthNum - 1) % 12], s.day);
    g.drawString(left, 6, theme::STATUS_H / 2);

    char mid[20];
    snprintf(mid, sizeof(mid), "%dF  %s", s.weather.tempF, skyWord(s.weather.sky));
    g.setTextDatum(textdatum_t::middle_center);
    g.setTextColor(theme::INK_DIM);
    g.drawString(mid, 168, theme::STATUS_H / 2);

    char right[20];
    snprintf(right, sizeof(right), "%d mi", s.odometer());
    g.setTextDatum(textdatum_t::middle_right);
    g.setTextColor(theme::INK);
    g.drawString(right, 288, theme::STATUS_H / 2);

    drawBattery(g, 314, theme::STATUS_H / 2);

    return theme::STATUS_H + 1;
}

}  // namespace hud
