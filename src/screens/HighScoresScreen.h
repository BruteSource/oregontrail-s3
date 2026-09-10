// The Oregon Top Five.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "SaveGame.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class HighScoresScreen : public Screen {
public:
    void onEnter() override { count_ = savegame::loadHighScores(rows_); }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        ui::drawCentered(g, "THE OREGON TOP FIVE", 160, 14, theme::ACCENT, 4);

        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::top_left);
        int16_t y = 56;
        for (int i = 0; i < count_; ++i) {
            const savegame::HighScore& r = rows_[i];
            g.setTextColor(r.arrived ? theme::INK : theme::INK_DIM);
            char line[48];
            snprintf(line, sizeof(line), "%d. %-13s %5ld", i + 1, r.name,
                     (long)r.score);
            g.drawString(line, theme::MARGIN + 4, y);
            g.setTextColor(theme::INK_DIM);
            char sub[32];
            snprintf(sub, sizeof(sub), "%s, %d days",
                     r.arrived ? "reached Oregon" : "lost on the trail", r.days);
            g.drawString(sub, theme::MARGIN + 24, y + 15);
            y += 34;
        }
        if (count_ == 0)
            ui::drawCentered(g, "no scores yet", 160, 110, theme::INK_DIM, 2);

        g.setTextDatum(textdatum_t::bottom_center);
        g.setTextColor(theme::INK_DIM);
        g.drawString("tap to go back", 160, 236);
    }

    void onTap(int16_t, int16_t) override { app::screens.pop(); }

private:
    savegame::HighScore rows_[savegame::kTopN]{};
    int count_ = 0;
};
