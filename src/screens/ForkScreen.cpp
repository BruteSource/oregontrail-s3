#include "screens/ForkScreen.h"

#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

void ForkScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);

    const game::Node& h = game::sim.here();
    const int16_t artH = art::drawLandmark(g, game::sim.locIndex, 0, 60);
    g.fillRect(0, artH, 320, 240 - artH, theme::BG);
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_center);
    g.setTextColor(theme::ACCENT);
    g.drawString(String(h.name) + " - the trail divides", 160, artH + 3);

    a_ = {theme::MARGIN, (int16_t)(artH + 22), 320 - 2 * theme::MARGIN, 74};
    b_ = {theme::MARGIN, (int16_t)(artH + 100), 320 - 2 * theme::MARGIN, 74};

    for (int i = 0; i < 2; ++i) {
        const ui::Rect& r = i == 0 ? a_ : b_;
        const char* opt = i == 0 ? h.optA : h.optB;
        const char* blurb = i == 0 ? h.blurbA : h.blurbB;
        ui::drawPanel(g, r);
        g.setFont(&fonts::Font2);
        g.setTextColor(theme::ACCENT);
        g.setTextDatum(textdatum_t::top_left);
        g.drawString(String(i + 1) + ".  " + opt, r.x + 8, r.y + 6);
        ui::drawText(g, blurb, r.x + 8, r.y + 24, r.w - 16, theme::INK);
    }
}

void ForkScreen::onTap(int16_t x, int16_t y) {
    if (a_.contains(x, y))      { game::sim.chooseBranch(0); app::screens.pop(); }
    else if (b_.contains(x, y)) { game::sim.chooseBranch(1); app::screens.pop(); }
}
