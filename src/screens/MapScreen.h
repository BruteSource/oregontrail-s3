// The MECC route map, zoomed in and kept centred on the party. The little red
// cross marks "you are here" at the landmark's real pixel spot on the DOS
// map.png (coords fitted by the reference project in OriginalTrail.cs). A
// compact stop list sits below-left; the < > arrows pan the map to look
// ahead/back; a tap anywhere else returns to the trail.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "game/Trail.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class MapScreen : public Screen {
public:
    static constexpr int   MAP_W = 640, MAP_H = 200, VIEW_H = 132;
    static constexpr float Z = 2.0f;                 // map zoom
    static constexpr int   WIN_W = (int)(320 / Z);   // source px shown across
    static constexpr int   WIN_H = (int)(VIEW_H / Z);

    // Landmark positions on the 640x200 map.png, node-for-node with game/Trail
    // (from the reference project's OriginalTrail.cs MapX/MapY table).
    static void nodeXY(int node, int* mx, int* my) {
        static const uint16_t X[20] = {578, 565, 546, 503, 462, 415, 372, 338,
                                       305, 319, 292, 257, 217, 195, 166, 161,
                                       140, 126, 128, 108};
        static const uint16_t Y[20] = {148, 153, 137, 134, 130, 123, 111, 117,
                                       136, 117, 116, 108, 111, 86,  72,  58,
                                       63,  65,  60,  57};
        const int i = node < 0 ? 0 : node > 19 ? 19 : node;
        *mx = X[i];
        *my = Y[i];
    }

    void onEnter() override {
        panNudge_ = 0;
        blink_ = 0;
    }

    void tick(uint32_t dtMs) override { blink_ += dtMs; }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        const game::Sim& s = game::sim;

        int mx, my;
        nodeXY(s.locIndex, &mx, &my);

        // source window, centred on the party, clamped to the image
        int srcX = mx - WIN_W / 2 + panNudge_;
        int srcY = my - WIN_H / 2;
        if (srcX < 0) srcX = 0;
        if (srcX > MAP_W - WIN_W) srcX = MAP_W - WIN_W;
        if (srcY < 0) srcY = 0;
        if (srcY > MAP_H - WIN_H) srcY = MAP_H - WIN_H;

        // NB: drawPng's offX/offY are in *scaled* (destination) pixels, not
        // source pixels — so the pan offset is srcX * Z.
        g.drawPng(art::map_img, art::map_img_len, 0, 0, 320, VIEW_H,
                  (int)(srcX * Z), (int)(srcY * Z), Z, Z);

        // "you are here" — solid red cross at the landmark's spot, with a ring
        // that pulses so it catches the eye without ever vanishing.
        const int markX = (int)((mx - srcX) * Z);
        const int markY = (int)((my - srcY) * Z);
        g.drawLine(markX - 8, markY, markX + 8, markY, theme::WARN);
        g.drawLine(markX - 8, markY + 1, markX + 8, markY + 1, theme::WARN);
        g.drawLine(markX, markY - 8, markX, markY + 8, theme::WARN);
        g.drawLine(markX + 1, markY - 8, markX + 1, markY + 8, theme::WARN);
        g.drawCircle(markX, markY, (blink_ / 350) % 2 ? 4 : 6, theme::WARN);

        // pan arrows (look west / east along the trail)
        left_  = {0, VIEW_H / 2 - 26, 30, 52};
        right_ = {290, VIEW_H / 2 - 26, 30, 52};
        const int cy = VIEW_H / 2;
        if (srcX > 0)
            g.fillTriangle(8, cy, 22, cy - 10, 22, cy + 10, theme::ACCENT);
        if (srcX < MAP_W - WIN_W)
            g.fillTriangle(312, cy, 298, cy - 10, 298, cy + 10, theme::ACCENT);

        // compact stop list
        g.drawFastHLine(0, VIEW_H, 320, theme::FRAME);
        int n;
        const game::Node* t = game::trailNodes(&n);
        int from = s.locIndex - 2;
        if (from < 0) from = 0;
        int listTo = from + 6;
        if (listTo > n) { listTo = n; from = listTo - 6 < 0 ? 0 : listTo - 6; }

        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::middle_left);
        const int16_t x0 = 24, top = VIEW_H + 14, rowH = 14;
        for (int i = from; i < listTo; ++i) {
            const int16_t y = top + (i - from) * rowH;
            const bool passed = i < s.locIndex;
            const bool cur = i == s.locIndex;
            uint16_t c = passed ? theme::INK_DIM : cur ? theme::ACCENT : theme::INK;
            if (i < listTo - 1) g.drawFastVLine(x0, y, rowH, theme::FRAME);
            g.fillCircle(x0, y, cur ? 4 : 2, c);
            g.setTextColor(c);
            g.drawString(t[i].name, x0 + 12, y);
            if (cur) g.fillTriangle(x0 - 11, y, x0 - 5, y - 4, x0 - 5, y + 4,
                                    theme::ACCENT);
        }

        // back hint — in the empty space to the right of the stop list
        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::middle_right);
        g.setTextColor(theme::ACCENT);
        g.drawString("tap here", 312, VIEW_H + 46);
        g.setTextColor(theme::INK_DIM);
        g.drawString("to go back", 312, VIEW_H + 62);

        char foot[44];
        snprintf(foot, sizeof(foot), "%d mi done   -   ~%d to Oregon", s.odometer(),
                 s.milesRemaining());
        g.setTextDatum(textdatum_t::bottom_center);
        g.setTextColor(theme::INK_DIM);
        g.drawString(foot, 160, 239);
    }

    void onTap(int16_t x, int16_t y) override {
        if (left_.contains(x, y))  { panNudge_ -= 60; return; }
        if (right_.contains(x, y)) { panNudge_ += 60; return; }
        app::screens.pop();
    }

private:
    int      panNudge_ = 0;   // source-px offset from the auto-centred view
    uint32_t blink_ = 0;
    ui::Rect left_{}, right_{};
};
