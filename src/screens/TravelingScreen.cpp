#include "screens/TravelingScreen.h"

#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/EventScreen.h"
#include "screens/ForkScreen.h"
#include "screens/GameOverScreen.h"
#include "screens/Hud.h"
#include "screens/LandmarkScreen.h"
#include "screens/MessageScreen.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

namespace {
constexpr uint32_t kDayMs = 650;   // one simulated day per beat
}

void TravelingScreen::onEnter() {
    accum_ = 0;
    halted_ = false;
    note_ = "";
    if (game::sim.lastNews[0]) note_ = game::sim.lastNews;
}

void TravelingScreen::tick(uint32_t dtMs) {
    if (halted_) return;
    animMs_ += dtMs;
    wheelPhase_ = (animMs_ / 160) % 3;   // wagon walk cycle
    accum_ += dtMs;
    if (accum_ < kDayMs) return;
    accum_ -= kDayMs;

    const game::TurnResult r = game::sim.takeTurn();
    if (game::sim.lastNews[0]) note_ = game::sim.lastNews;

    switch (r) {
        case game::TurnResult::Traveled:
            break;
        case game::TurnResult::Event:
            halted_ = true;
            app::screens.push(new EventScreen());   // resumes here when dismissed
            break;
        case game::TurnResult::ForkChoice:
            halted_ = true;
            app::screens.push(new ForkScreen());   // returns here when chosen
            break;
        case game::TurnResult::Arrived:
            halted_ = true;
            app::screens.replace(new LandmarkScreen());
            break;
        case game::TurnResult::ReachedOregon:
            halted_ = true;
            app::screens.replace(new GameOverScreen(true));
            break;
        case game::TurnResult::PartyWiped:
            halted_ = true;
            app::screens.replace(new GameOverScreen(false));
            break;
        case game::TurnResult::Blocked:
            halted_ = true;
            app::screens.replace(new MessageScreen(
                "Stuck", "The team can't pull the wagon. You need oxen."));
            break;
    }
}

void TravelingScreen::onTap(int16_t, int16_t) {
    if (halted_) return;
    halted_ = true;
    app::screens.pop();   // pull up; back to the trail menu
}

void TravelingScreen::render(LGFX_Sprite& g) {
    const game::Sim& s = game::sim;
    const int odo = s.odometer();

    // --- scene bands (clean top, no overlay) ---
    const int16_t horizon = 100, groundBot = 168;
    g.fillRect(0, 0, 320, horizon, art::col::sky);
    g.fillRect(0, horizon, 320, groundBot - horizon, art::col::ground);

    // clouds (slow)
    for (int k = 0; k < 3; ++k) {
        int x = ((k * 150 - odo) % 480 + 480) % 480 - 80;
        g.fillEllipse(x + 14, 24 + k * 9, 16, 6, 0xFFFF);
        g.fillEllipse(x + 30, 26 + k * 9, 12, 5, 0xFFFF);
    }
    // distant snow mountains — scen_2 (320x24), tiled + slow parallax
    {
        int off = (odo * 2) % 320;
        art::drawScenery(g, 1, -off, horizon - 22);
        art::drawScenery(g, 1, 320 - off, horizon - 22);
    }
    // mid-ground hedgerow — scen_1 (320x19), medium parallax
    {
        int off = (odo * 5) % 320;
        art::drawScenery(g, 0, -off, horizon - 4);
        art::drawScenery(g, 0, 320 - off, horizon - 4);
    }
    // the trail
    g.fillRect(0, groundBot - 13, 320, 13, art::col::dirt);
    for (int x = -((odo * 8) % 28); x < 320; x += 28)
        g.fillRect(x, groundBot - 12, 12, 3, 0x8482);

    // foreground trees / rocks, fast parallax
    for (int k = 0; k < 5; ++k) {
        int x = ((k * 130 - odo * 8) % 620 + 620) % 620 - 80;
        art::drawTerrain(g, (k * 5) % 14, x, groundBot - 34);
    }

    // --- the wagon ---
    const int16_t bob = (wheelPhase_ == 1) ? -1 : 0;
    art::drawWagon(g, halted_ ? 0 : wheelPhase_, 118, groundBot - 30 + bob);

    // --- text panel (all UI lives here, below the scene) ---
    g.fillRect(0, groundBot, 320, 240 - groundBot, theme::BG);
    g.drawFastHLine(0, groundBot, 320, theme::FRAME);

    static const char* kM[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_left);
    g.setTextColor(theme::INK_DIM);
    char hud[40];
    snprintf(hud, sizeof(hud), "%s %d    %dF  %s", kM[(s.monthNum - 1) % 12], s.day,
             s.weather.tempF,
             s.weather.sky == game::Sky::Rain ? "Rain"
             : s.weather.sky == game::Sky::Snow ? "Snow" : "Clear");
    g.drawString(hud, theme::MARGIN, groundBot + 5);
    g.setTextDatum(textdatum_t::top_right);
    g.drawString(String(s.odometer()) + " mi", 314, groundBot + 5);

    g.setTextDatum(textdatum_t::top_center);
    g.setTextColor(theme::INK);
    char line[56];
    snprintf(line, sizeof(line), "%s  -  %d mi to go", s.nextName(),
             s.milesToNextStop());
    g.drawString(line, 160, groundBot + 22);

    if (note_.length()) {
        g.setTextColor(theme::WARN);
        g.drawString(note_, 160, groundBot + 40);
    } else {
        g.setTextColor(theme::INK_DIM);
        g.drawString("tap to pull up", 160, groundBot + 40);
    }
}
