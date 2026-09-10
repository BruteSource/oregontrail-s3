// Dead end: the wagon has no oxen to pull it and (usually) no way to get any
// out on the trail. Rather than drop the player back onto a trail menu they
// can't act from, end the journey here and offer a fresh start. If the wagon is
// still sitting at a settlement, a trip into the store is offered as a lifeline;
// buy a team there and this screen lets the journey continue.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "SaveGame.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/MainMenuScreen.h"
#include "screens/ProfessionScreen.h"
#include "screens/StoreScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class StuckScreen : public Screen {
public:
    void onEnter() override {
        recovered_ = game::g.vehicle.oxen > 0;
        atStore_ = !recovered_ && game::sim.milesIntoLeg == 0 &&
                   game::sim.here().kind == game::Stop::Settlement;
    }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        const int16_t W = 320 - 2 * theme::MARGIN;

        if (recovered_) {
            ui::drawCentered(g, "BACK IN HARNESS", 160, 40, theme::ACCENT, 4);
            ui::drawText(g, "You have a team again. The wagon can roll on.",
                         theme::MARGIN, 90, W, theme::INK, 2, 18, true);
            go_ = {theme::MARGIN, 150, W, 34};
            store_ = newRun_ = menu_ = {};
            ui::drawButton(g, go_, "Back to the trail", true);
            return;
        }

        ui::drawCentered(g, "THE TRAIL ENDS HERE", 160, 34, theme::WARN, 4);
        ui::drawText(g,
                     atStore_
                         ? "With no oxen, the wagon can't move. Trade for a "
                           "team at the store, or the journey is over."
                         : "With no oxen to pull it, the wagon can go no "
                           "further. Your journey is over.",
                     theme::MARGIN, 78, W, theme::INK, 2, 18, true);

        go_ = {};
        if (atStore_) {
            store_  = {theme::MARGIN, 138, W, 32};
            newRun_ = {theme::MARGIN, 176, 150, 30};
            menu_   = {166, 176, 146, 30};
            ui::drawButton(g, store_, "Visit the store", true);
        } else {
            store_  = {};
            newRun_ = {theme::MARGIN, 150, 150, 34};
            menu_   = {166, 150, 146, 34};
        }
        ui::drawButton(g, newRun_, "New journey", !atStore_);
        ui::drawButton(g, menu_, "Main menu");
    }

    void onTap(int16_t x, int16_t y) override {
        if (go_.w && go_.contains(x, y)) {
            app::screens.pop();                          // back to the trail menu
        } else if (store_.w && store_.contains(x, y)) {
            app::screens.push(new StoreScreen(false));   // marked-up fort store
        } else if (newRun_.w && newRun_.contains(x, y)) {
            savegame::clear();
            app::screens.reset(new ProfessionScreen());
        } else if (menu_.w && menu_.contains(x, y)) {
            savegame::clear();
            app::screens.reset(new MainMenuScreen());
        }
    }

private:
    bool     recovered_ = false;
    bool     atStore_ = false;
    ui::Rect go_{}, store_{}, newRun_{}, menu_{};
};
