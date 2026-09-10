// Device settings: brightness, volume (placeholder), recalibrate, sleep,
// restart, wipe the Top Five. Reachable from the main menu and the trail menu.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "SaveGame.h"
#include "Settings.h"
#include "hw/Battery.h"
#include "screens/MessageScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class SettingsScreen : public Screen {
public:
    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        ui::drawCentered(g, "SETTINGS", 160, 8, theme::ACCENT, 4);

        char b[44];
        const int chg = battery::charging();
        snprintf(b, sizeof(b), "Battery %d%%  %.2fV%s", battery::percent(),
                 battery::volts(),
                 chg == 1 ? "  charging" : chg == 0 ? "  full" : "");
        ui::drawCentered(g, b, 160, 40, theme::INK_DIM, 2);

        slider(g, "Brightness", 56, bMinus_, bPlus_, bBar_,
               (prefs::brightness - prefs::kBrightMin) * 100 /
                   (prefs::kBrightMax - prefs::kBrightMin),
               278, theme::INK);

        // Volume — placeholder until a speaker is wired to the ES8311.
        vMinus_ = vPlus_ = vBar_ = {};
        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::middle_left);
        g.setTextColor(theme::INK_DIM);
        g.drawString("Volume", theme::MARGIN, 92 + 12);
        g.drawString("--   (add a speaker to enable)", 96, 92 + 12);

        const int16_t W = 320 - 2 * theme::MARGIN, cw = (W - 8) / 2;
        const int16_t x2 = theme::MARGIN + cw + 8;
        recal_   = {theme::MARGIN, 128, cw, 30};
        sleep_   = {x2,            128, cw, 30};
        restart_ = {theme::MARGIN, 164, cw, 30};
        erase_   = {x2,            164, cw, 30};
        back_    = {theme::MARGIN, 202, W, 30};
        ui::drawButton(g, recal_, "Recalibrate touch");
        ui::drawButton(g, sleep_, "Sleep now");
        ui::drawButton(g, restart_, "Restart");
        ui::drawButton(g, erase_, "Erase Top Five");
        ui::drawButton(g, back_, "Back", true);
    }

    void onTap(int16_t x, int16_t y) override {
        if (bMinus_.contains(x, y)) bump(prefs::brightness, -prefs::kBrightStep);
        else if (bPlus_.contains(x, y)) bump(prefs::brightness, prefs::kBrightStep);
        else if (recal_.contains(x, y)) app::wantRecal = true;
        else if (sleep_.contains(x, y)) app::wantSleep = true;
        else if (restart_.contains(x, y)) app::wantRestart = true;
        else if (erase_.contains(x, y)) {
            savegame::eraseHighScores();
            app::screens.push(new MessageScreen("Done", "The Top Five is wiped."));
        } else if (back_.contains(x, y)) {
            app::screens.pop();
        }
    }

private:
    ui::Rect bMinus_{}, bPlus_{}, bBar_{}, vMinus_{}, vPlus_{}, vBar_{};
    ui::Rect recal_{}, sleep_{}, restart_{}, erase_{}, back_{};

    void bump(int& v, int d) {
        v += d;
        if (v < prefs::kBrightMin) v = prefs::kBrightMin;
        if (v > prefs::kBrightMax) v = prefs::kBrightMax;
        app::setBrightness(v);
        prefs::save();
    }

    static void slider(LGFX_Sprite& g, const char* label, int16_t y, ui::Rect& minus,
                       ui::Rect& plus, ui::Rect& bar, int pct, int16_t barEndX,
                       uint16_t barColor) {
        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::middle_left);
        g.setTextColor(theme::INK);
        g.drawString(label, theme::MARGIN, y + 12);
        minus = {96, y, 26, 24};
        plus  = {286, y, 26, 24};
        ui::drawButton(g, minus, "-");
        ui::drawButton(g, plus, "+");
        bar = {128, (int16_t)(y + 6), (int16_t)(barEndX - 128), 12};
        g.drawRect(bar.x, bar.y, bar.w, bar.h, theme::FRAME);
        const int fill = (bar.w - 2) * pct / 100;
        if (fill > 0) g.fillRect(bar.x + 1, bar.y + 1, fill, bar.h - 2, barColor);
    }
};
