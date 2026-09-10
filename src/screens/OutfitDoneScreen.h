// End of the new-game flow (Phase 2). Shows the wagon's starting loadout.
// Phase 3 replaces the "the trail begins" button with the travel menu.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "game/Session.h"
#include "game/Sim.h"
#include "screens/TrailMenuScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class OutfitDoneScreen : public Screen {
public:
    void render(LGFX_Sprite& g) override {
        const game::Vehicle& v = game::g.vehicle;
        const game::Party& p = game::g.party;

        g.fillScreen(theme::BG);
        ui::drawCentered(g, "READY TO ROLL", 160, 10, theme::ACCENT, 4);

        String who = String(p.leader().name) + " leads " +
                     (p.count - 1) + " others west from Independence, " +
                     game::monthName(game::g.startMonth) + " 1848.";
        int16_t y = ui::drawText(g, who, theme::MARGIN, 42,
                                 320 - 2 * theme::MARGIN, theme::INK);

        y += 6;
        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::top_left);
        auto row = [&](const String& s) {
            g.setTextColor(theme::INK);
            g.drawString(s, theme::MARGIN + 4, y);
            y += 16;
        };
        row(String("Cash left:  $") + v.cash);
        row(String("Oxen:       ") + v.oxen);
        row(String("Food:       ") + v.food + " lb");
        row(String("Clothing:   ") + v.clothes + " sets");
        row(String("Ammunition: ") + v.bullets + " rounds");
        row(String("Spare parts: ") + v.wheels + "w " + v.axles + "a " +
            v.tongues + "t   Medicine: " + v.medicine);

        btn_ = {(int16_t)(160 - 90), 202, 180, 30};
        ui::drawButton(g, btn_, "The trail begins", true);
    }

    void onTap(int16_t x, int16_t y) override {
        if (btn_.contains(x, y)) {
            game::sim.begin();
            app::screens.reset(new TrailMenuScreen());
        }
    }

private:
    ui::Rect btn_{};
};
