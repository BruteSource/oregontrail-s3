// "Give up and turn back" confirmation, reached from the trail menu.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "SaveGame.h"
#include "screens/MainMenuScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class AbandonScreen : public Screen {
public:
    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        ui::drawCentered(g, "GIVE UP THE TRAIL?", 160, 40, theme::WARN, 4);
        ui::drawText(g,
                     "The journey so far will be lost and you will not be scored.",
                     theme::MARGIN, 84, 320 - 2 * theme::MARGIN, theme::INK, 2, 18,
                     true);

        yes_ = {theme::MARGIN, 150, 150, 34};
        no_  = {166, 150, 146, 34};
        ui::drawButton(g, yes_, "Turn back");
        ui::drawButton(g, no_, "Keep going", true);
    }

    void onTap(int16_t x, int16_t y) override {
        if (yes_.contains(x, y)) {
            savegame::clear();
            app::screens.reset(new MainMenuScreen());
        } else if (no_.contains(x, y)) {
            app::screens.pop();
        }
    }

private:
    ui::Rect yes_{}, no_{};
};
