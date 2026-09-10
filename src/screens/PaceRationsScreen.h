// Set the travelling pace and the food ration in one place. (ChangePace.cs /
// ChangeRations.cs)
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "game/Session.h"
#include "screens/Hud.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class PaceRationsScreen : public Screen {
public:
    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        hud::drawTrailStatus(g);

        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::top_left);

        static const char* pace[3] = {"Steady - normal pace",
                                      "Strenuous - 50% more a day",
                                      "Grueling - double, hard on all"};
        static const char* rat[3] = {"Filling - 3 lb each",
                                     "Meager - 2 lb each",
                                     "Bare bones - 1 lb each"};

        g.setTextColor(theme::ACCENT);
        g.drawString("TRAVELLING PACE", theme::MARGIN, 26);
        const int curP = static_cast<int>(game::g.vehicle.pace);
        for (int i = 0; i < 3; ++i) {
            paceR_[i] = {theme::MARGIN, (int16_t)(42 + i * 30),
                         320 - 2 * theme::MARGIN, 26};
            ui::drawButton(g, paceR_[i], pace[i], i == curP);
        }

        g.setTextColor(theme::ACCENT);
        g.setTextDatum(textdatum_t::top_left);
        g.drawString("FOOD RATIONS", theme::MARGIN, 136);
        const int curR = static_cast<int>(game::g.vehicle.rations);
        for (int i = 0; i < 3; ++i) {
            ratR_[i] = {theme::MARGIN, (int16_t)(152 + i * 26),
                        320 - 2 * theme::MARGIN, 22};
            ui::drawButton(g, ratR_[i], rat[i], i == curR);
        }

        g.setTextDatum(textdatum_t::bottom_center);
        g.setTextColor(theme::INK_DIM);
        g.drawString("tap here to go back", 160, 238);
    }

    void onTap(int16_t x, int16_t y) override {
        for (int i = 0; i < 3; ++i) {
            if (paceR_[i].contains(x, y)) {
                game::g.vehicle.pace = static_cast<game::Pace>(i);
                return;
            }
            if (ratR_[i].contains(x, y)) {
                game::g.vehicle.rations = static_cast<game::Rations>(i);
                return;
            }
        }
        app::screens.pop();
    }

private:
    ui::Rect paceR_[3], ratR_[3];
};
