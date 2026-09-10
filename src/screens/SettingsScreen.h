// Device utilities: recalibrate touch, sleep, restart, wipe the Top Five.
#pragma once
#include <Arduino.h>
#include <vector>

#include <LovyanGFX.hpp>

#include "SaveGame.h"
#include "hw/Battery.h"
#include "screens/MessageScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class SettingsScreen : public Screen {
public:
    void onEnter() override {
        menu_.clear();
        menu_.add("Recalibrate touch");
        menu_.add("Sleep now");
        menu_.add("Restart the device");
        menu_.add("Erase the Top Five");
        menu_.add("Back");
        menu_.setBounds(theme::MARGIN, 64, 320 - 2 * theme::MARGIN, 30);
    }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        ui::drawCentered(g, "SETTINGS", 160, 12, theme::ACCENT, 4);

        char b[40];
        const int pc = battery::percent();
        snprintf(b, sizeof(b), "Battery  %d%%   %.2f V%s", pc, battery::volts(),
                 battery::charging() == 1 ? "  (charging)"
                 : battery::charging() == 0 ? "  (full)" : "");
        ui::drawCentered(g, b, 160, 42, theme::INK_DIM, 2);

        menu_.render(g, sel_);
    }

    void onTap(int16_t x, int16_t y) override {
        const int i = menu_.hitTest(x, y);
        sel_ = i;
        switch (i) {
            case 0: app::wantRecal = true; break;
            case 1: app::wantSleep = true; break;
            case 2: app::wantRestart = true; break;
            case 3:
                savegame::eraseHighScores();
                app::screens.push(new MessageScreen("Done", "The Top Five is wiped."));
                break;
            case 4: app::screens.pop(); break;
            default: break;
        }
    }

private:
    ui::MenuList menu_;
    int sel_ = -1;
};
