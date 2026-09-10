// Read-only look at the wagon inventory and the party's health.
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

class SuppliesScreen : public Screen {
public:
    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        hud::drawTrailStatus(g);
        const game::Vehicle& v = game::g.vehicle;

        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::top_left);

        int16_t y = 25;
        auto line = [&](const String& s, uint16_t c) {
            g.setTextColor(c);
            g.drawString(s, theme::MARGIN, y);
            y += 14;
        };
        g.setTextColor(theme::ACCENT);
        g.drawString("SUPPLIES", theme::MARGIN, y);
        y += 15;
        line(String("Food        ") + v.food + " lb", theme::INK);
        line(String("Oxen        ") + v.oxen, theme::INK);
        line(String("Clothing    ") + v.clothes + " sets", theme::INK);
        line(String("Ammunition  ") + v.bullets + " rounds", theme::INK);
        line(String("Spare parts ") + v.wheels + "w " + v.axles + "a " + v.tongues + "t", theme::INK);
        line(String("Medicine    ") + v.medicine + " kits", theme::INK);
        line(String("Cash        $") + v.cash, theme::INK);

        y += 4;
        g.setTextColor(theme::ACCENT);
        g.drawString("PARTY", theme::MARGIN, y);
        y += 15;
        for (int i = 0; i < game::g.party.count; ++i) {
            const game::Person& p = game::g.party.member[i];
            String s = String(p.name);
            uint16_t c = theme::INK;
            if (!p.alive) { s += " - dead"; c = theme::WARN; }
            else {
                s += String(" - ") + game::healthName(p.band());
                if (p.sick) { s += ", ill"; c = theme::WARN; }
            }
            line(s, c);
        }

        g.setTextDatum(textdatum_t::bottom_right);
        g.setTextColor(theme::INK_DIM);
        g.drawString("tap to go back", 314, 238);
    }
    void onTap(int16_t, int16_t) override { app::screens.pop(); }
};
