// The top-level menu. "Continue journey" appears only when a trip is saved.
#pragma once
#include <Arduino.h>
#include <vector>

#include <LovyanGFX.hpp>

#include "SaveGame.h"
#include "hw/Audio.h"
#include "game/Sim.h"
#include "screens/HighScoresScreen.h"
#include "screens/MessageScreen.h"
#include "screens/ProfessionScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/TrailMenuScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class MainMenuScreen : public Screen {
public:
    void onEnter() override {
        audio::stopSong();
        menu_.clear();
        act_.clear();
        auto row = [&](const char* label, int a) { menu_.add(label); act_.push_back(a); };

        if (savegame::exists()) row("Continue journey", A_CONTINUE);
        row("Travel the trail", A_NEW);
        row("Learn about the trail", A_ABOUT);
        row("See the Oregon Top Five", A_SCORES);
        row("Settings", A_SETTINGS);
        menu_.setBounds(theme::MARGIN, 66, 320 - 2 * theme::MARGIN, 34);
    }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        ui::drawCentered(g, "THE OREGON TRAIL", 160, 16, theme::ACCENT, 4);
        ui::drawCentered(g, "Choose your path", 160, 46, theme::INK_DIM, 2);
        menu_.render(g, selected_);
    }

    void onTap(int16_t x, int16_t y) override {
        const int i = menu_.hitTest(x, y);
        selected_ = i;
        if (i < 0 || i >= (int)act_.size()) return;
        switch (act_[i]) {
            case A_CONTINUE:
                if (savegame::load()) app::screens.reset(new TrailMenuScreen());
                break;
            case A_NEW:
                app::screens.push(new ProfessionScreen());
                break;
            case A_ABOUT:
                app::screens.push(new MessageScreen(
                    "The Oregon Trail",
                    "2,000 miles from Independence to the Willamette Valley. "
                    "Outfit a wagon, keep your party fed and healthy, cross the "
                    "rivers, and reach Oregon before winter."));
                break;
            case A_SCORES:
                app::screens.push(new HighScoresScreen());
                break;
            case A_SETTINGS:
                app::screens.push(new SettingsScreen());
                break;
        }
    }

private:
    enum { A_CONTINUE, A_NEW, A_ABOUT, A_SCORES, A_SETTINGS };
    ui::MenuList     menu_;
    std::vector<int> act_;
    int selected_ = -1;
};
