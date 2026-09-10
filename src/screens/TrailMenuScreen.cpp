#include "screens/TrailMenuScreen.h"

#include "SaveGame.h"
#include "game/Advice.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/HuntScreen.h"
#include "screens/Hud.h"
#include "screens/MapScreen.h"
#include "screens/MessageScreen.h"
#include "screens/PaceRationsScreen.h"
#include "screens/RestScreen.h"
#include "screens/RiverScreen.h"
#include "screens/SuppliesScreen.h"
#include "screens/TravelingScreen.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

namespace {
enum Act { A_GO, A_SUPPLIES, A_MAP, A_PACE, A_RATIONS, A_REST, A_HUNT, A_TALK };
}

void TrailMenuScreen::onEnter() {
    rebuildMenu();
    if (!game::sim.atOregon()) savegame::save();   // checkpoint
}

void TrailMenuScreen::rebuildMenu() {
    menu_.clear();
    act_.clear();
    auto row = [&](const String& label, Act a) { menu_.add(label); act_.push_back(a); };

    row(game::sim.mustCross   ? "Cross the river"
        : game::sim.departed  ? "Continue on the trail"
                              : "Head out on the trail", A_GO);
    row("Check supplies", A_SUPPLIES);
    row("Look at the map", A_MAP);
    row(String("Change pace  (") + game::paceName(game::g.vehicle.pace) + ")", A_PACE);
    row(String("Change rations  (") + game::rationsName(game::g.vehicle.rations) + ")", A_RATIONS);
    row("Rest a while", A_REST);
    if (game::g.vehicle.bullets >= 1 && !game::sim.mustCross)
        row("Hunt for food", A_HUNT);
    const auto k = game::sim.here().kind;
    if (!game::sim.departed &&
        (k == game::Stop::Settlement || k == game::Stop::Landmark))
        row("Talk to people", A_TALK);

    menu_.setBounds(theme::MARGIN, 90, 320 - 2 * theme::MARGIN, 18);
}

void TrailMenuScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);
    hud::drawTrailStatus(g);

    const game::Sim& s = game::sim;
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_left);

    if (!s.departed) {
        g.setTextColor(theme::ACCENT);
        g.drawString(String("You are at ") + s.here().name, theme::MARGIN, 28);
    } else {
        g.setTextColor(theme::INK);
        g.drawString(String("Heading for ") + s.nextName(), theme::MARGIN, 28);
    }
    g.setTextColor(theme::INK_DIM);
    char sub[52];
    snprintf(sub, sizeof(sub), "%d mi to go   -   %d mi to Oregon",
             s.milesToNextStop(), s.milesRemaining());
    g.drawString(sub, theme::MARGIN, 46);

    const int16_t ty = 74;
    g.drawFastHLine(theme::MARGIN, ty, 320 - 2 * theme::MARGIN, theme::INK_DIM);
    const int total = game::trailLongestMiles();
    const float frac = total ? (float)s.odometer() / total : 0;
    const int16_t wx = theme::MARGIN + (int16_t)(frac * (320 - 2 * theme::MARGIN));
    g.fillRoundRect(wx - 5, ty - 7, 11, 6, 2, theme::INK);
    g.drawPixel(wx - 3, ty, theme::INK);
    g.drawPixel(wx + 3, ty, theme::INK);

    menu_.render(g);
}

void TrailMenuScreen::onTap(int16_t x, int16_t y) {
    const int i = menu_.hitTest(x, y);
    if (i < 0 || i >= (int)act_.size()) return;

    switch (act_[i]) {
        case A_GO:
            if (game::sim.mustCross) app::screens.push(new RiverScreen());
            else app::screens.push(new TravelingScreen());
            break;
        case A_SUPPLIES: app::screens.push(new SuppliesScreen()); break;
        case A_MAP:      app::screens.push(new MapScreen()); break;
        case A_PACE:     app::screens.push(new PaceRationsScreen(true)); break;
        case A_RATIONS:  app::screens.push(new PaceRationsScreen(false)); break;
        case A_REST:     app::screens.push(new RestScreen()); break;
        case A_HUNT:     app::screens.push(new HuntScreen()); break;
        case A_TALK:
            app::screens.push(new MessageScreen(game::sim.here().name,
                                                game::randomAdvice()));
            break;
    }
}
