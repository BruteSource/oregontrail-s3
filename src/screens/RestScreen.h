// Rest in place for a few days. The party recovers faster stopped than moving,
// but the calendar (and the weather) still run. (Resting.cs / RestAmount.cs)
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "game/Session.h"
#include "game/Sim.h"
#include "screens/Hud.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class RestScreen : public Screen {
public:
    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        hud::drawTrailStatus(g);
        ui::drawCentered(g, "REST", 160, 28, theme::ACCENT, 4);

        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::top_center);
        g.setTextColor(theme::INK_DIM);
        g.drawString("The party stops to recover.", 160, 60);

        // party health
        g.setTextDatum(textdatum_t::top_left);
        int16_t y = 84;
        for (int i = 0; i < game::g.party.count; ++i) {
            const game::Person& p = game::g.party.member[i];
            g.setTextColor(p.alive ? (p.sick ? theme::WARN : theme::INK) : theme::INK_DIM);
            String s = String(p.name) + " - " +
                       (p.alive ? game::healthName(p.band()) : "dead");
            g.drawString(s, theme::MARGIN + 8, y);
            y += 15;
        }

        // day stepper
        minus_ = {70, 168, 34, 30};
        plus_  = {216, 168, 34, 30};
        ui::drawButton(g, minus_, "-");
        ui::drawButton(g, plus_, "+");
        g.setFont(&fonts::Font4);
        g.setTextColor(theme::INK);
        g.setTextDatum(textdatum_t::middle_center);
        g.drawString(String(days_) + (days_ == 1 ? " day" : " days"), 160, 183);

        rest_ = {(int16_t)(160 - 70), 206, 140, 28};
        ui::drawButton(g, rest_, "Rest", true);
    }

    void onTap(int16_t x, int16_t y) override {
        if (minus_.contains(x, y)) { if (days_ > 1) --days_; }
        else if (plus_.contains(x, y)) { if (days_ < 14) ++days_; }
        else if (rest_.contains(x, y)) {
            game::sim.rest(days_);
            app::screens.pop();
        }
    }

private:
    int days_ = 3;
    ui::Rect minus_{}, plus_{}, rest_{};
};
