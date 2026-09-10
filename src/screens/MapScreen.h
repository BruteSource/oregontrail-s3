// The MECC route map at full resolution, scrolled to the party's position (the
// whole 640-wide map won't fit, so we show a window and let you pan it). A
// compact stop list sits below.
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
    static constexpr int MAP_W = 640, MAP_H = 200, VIEW_H = 132;

    void onEnter() override {
        // START (Independence) sits at the right edge of the map, FINISH at the
        // left; progress west moves the view leftward.
        pan_ = (int)((1.0f - progress()) * (MAP_W - 320));
        blink_ = 0;
    }

    // 0 at Independence .. 1 at Oregon City, by trail node.
    static float progress() {
        int n;
        game::trailNodes(&n);
        return n > 1 ? (float)game::sim.locIndex / (n - 1) : 0;
    }

    void tick(uint32_t dtMs) override { blink_ += dtMs; }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        if (pan_ < 0) pan_ = 0;
        if (pan_ > MAP_W - 320) pan_ = MAP_W - 320;

        // draw at (0,0), clip 320xVIEW_H, read starting from source column pan_
        g.drawPng(art::map_img, art::map_img_len, 0, 0, 320, VIEW_H, pan_, 0);

        // "you are here" marker — interpolate the stop's x along the map
        const game::Sim& s = game::sim;
        const int markX = (int)(600 - progress() * 560) - pan_;
        if ((blink_ / 400) % 2 == 0 && markX > -6 && markX < 326) {
            g.drawLine(markX - 6, 40, markX + 6, 40, theme::WARN);
            g.drawLine(markX, 34, markX, 46, theme::WARN);
            g.fillCircle(markX, 40, 2, theme::WARN);
        }

        // back affordance — top-left, over the map
        back_ = {4, 4, 62, 22};
        g.fillRoundRect(back_.x, back_.y, back_.w, back_.h, 4, theme::PANEL);
        g.drawRoundRect(back_.x, back_.y, back_.w, back_.h, 4, theme::FRAME);
        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::middle_center);
        g.setTextColor(theme::INK);
        g.drawString("< Back", back_.x + back_.w / 2, back_.y + back_.h / 2 + 1);

        // pan arrows
        left_  = {0, 40, 26, 52};
        right_ = {294, 40, 26, 52};
        if (pan_ > 0) {
            g.fillTriangle(6, 66, 20, 56, 20, 76, theme::ACCENT);
        }
        if (pan_ < MAP_W - 320) {
            g.fillTriangle(314, 66, 300, 56, 300, 76, theme::ACCENT);
        }

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

        char foot[44];
        snprintf(foot, sizeof(foot), "%d mi done   -   ~%d to Oregon", s.odometer(),
                 s.milesRemaining());
        g.setTextDatum(textdatum_t::bottom_center);
        g.setTextColor(theme::INK_DIM);
        g.drawString(foot, 160, 239);
    }

    void onTap(int16_t x, int16_t y) override {
        if (back_.contains(x, y))  { app::screens.pop(); return; }
        if (left_.contains(x, y))  { pan_ -= 90; return; }
        if (right_.contains(x, y)) { pan_ += 90; return; }
        app::screens.pop();
    }

private:
    int      pan_ = 0;
    uint32_t blink_ = 0;
    ui::Rect left_{}, right_{}, back_{};
};
