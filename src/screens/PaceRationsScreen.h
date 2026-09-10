// Change the travelling pace or the food rations. One screen, two modes.
// (ChangePace.cs / ChangeRations.cs)
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "game/Session.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class PaceRationsScreen : public Screen {
public:
    explicit PaceRationsScreen(bool pace) : pace_(pace) {}

    void onEnter() override {
        menu_.clear();
        if (pace_) {
            menu_.add("Steady - the normal pace");
            menu_.add("Strenuous - 50% more miles a day");
            menu_.add("Grueling - double miles, hard on all");
        } else {
            menu_.add("Filling - 3 lb each, keeps spirits up");
            menu_.add("Meager - 2 lb each, wears a little");
            menu_.add("Bare bones - 1 lb each, wears hard");
        }
        menu_.setBounds(theme::MARGIN, 70, 320 - 2 * theme::MARGIN, 40);
    }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        ui::drawCentered(g, pace_ ? "TRAVELLING PACE" : "FOOD RATIONS", 160, 14,
                         theme::ACCENT, 4);
        String cur = pace_ ? String("Now: ") + game::paceName(game::g.vehicle.pace)
                           : String("Now: ") + game::rationsName(game::g.vehicle.rations);
        ui::drawCentered(g, cur, 160, 46, theme::INK_DIM, 2);
        int sel = pace_ ? static_cast<int>(game::g.vehicle.pace)
                        : static_cast<int>(game::g.vehicle.rations);
        menu_.render(g, sel);
    }

    void onTap(int16_t x, int16_t y) override {
        const int i = menu_.hitTest(x, y);
        if (i < 0) return;
        if (pace_) game::g.vehicle.pace = static_cast<game::Pace>(i);
        else       game::g.vehicle.rations = static_cast<game::Rations>(i);
        app::screens.pop();
    }

private:
    bool pace_;
    ui::MenuList menu_;
};
