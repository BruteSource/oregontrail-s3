#include "screens/LandmarkScreen.h"

#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/Hud.h"
#include "screens/StoreScreen.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

namespace {
const char* flavorFor(const game::Node& n) {
    switch (n.kind) {
        case game::Stop::Settlement: return "Rest here, and trade for supplies.";
        case game::Stop::River:      return "A river lies across the trail.";
        case game::Stop::Fork:       return "The trail splits here.";
        case game::Stop::TollRoad:   return "A toll gate bars the road.";
        default:                     return "A welcome sight on a long road.";
    }
}
}  // namespace

void LandmarkScreen::onEnter() {
    hasStore_ = game::sim.here().kind == game::Stop::Settlement;
}

void LandmarkScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);
    const game::Sim& s = game::sim;

    // full painting, nothing over it
    const int16_t artH = art::drawLandmark(g, s.locIndex, 0, 148);
    g.fillRect(0, artH, 320, 240 - artH, theme::BG);

    // caption + info below
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_center);
    g.setTextColor(theme::ACCENT);
    g.drawString(s.here().name, 160, artH + 3);

    char sub[48];
    snprintf(sub, sizeof(sub), "Day %d   -   %d miles out", s.turns, s.odometer());
    g.setTextColor(theme::INK_DIM);
    g.drawString(sub, 160, artH + 19);
    g.setTextColor(theme::INK);
    g.drawString(flavorFor(s.here()), 160, artH + 34);

    const int16_t by = artH + 52;
    if (hasStore_) {
        store_ = {theme::MARGIN, by, 150, 28};
        go_    = {166, by, 146, 28};
        ui::drawButton(g, store_, "Visit the store");
        ui::drawButton(g, go_, "Move on", true);
    } else {
        go_ = {(int16_t)(160 - 90), by, 180, 28};
        ui::drawButton(g, go_, s.atOregon() ? "Arrived" : "Continue", true);
    }
}

void LandmarkScreen::onTap(int16_t x, int16_t y) {
    if (hasStore_ && store_.contains(x, y)) {
        app::screens.push(new StoreScreen(false));   // fort store, marked up
        return;
    }
    if (go_.contains(x, y)) app::screens.pop();       // back to the trail menu
}
