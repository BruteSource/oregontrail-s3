#include "screens/RiverScreen.h"

#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/GameOverScreen.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

namespace {
const char* depthWord(float ft) {
    if (ft < 1.5f) return "knee deep";
    if (ft < 3.0f) return "waist deep";
    if (ft < 5.0f) return "over your head";
    return "deep, fast water";
}
}  // namespace

void RiverScreen::onEnter() {
    r_ = game::riverInfoForNode(game::sim.locIndex);
    showResult_ = false;
}

void RiverScreen::doChoice(game::CrossChoice c) {
    if (!r_) { game::sim.mustCross = false; app::screens.pop(); return; }
    result_ = game::attemptCrossing(*r_, c);
    showResult_ = true;
    if (game::g.party.aliveCount() == 0) {
        app::screens.replace(new GameOverScreen(false));
        return;
    }
    if (result_.crossed) game::sim.mustCross = false;
}

void RiverScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);

    if (!r_) {
        ui::drawCentered(g, "The crossing is easy", 160, 100, theme::INK, 2);
        ok_ = {(int16_t)(160 - 70), 150, 140, 30};
        ui::drawButton(g, ok_, "Continue", true);
        return;
    }

    // the river painting
    const int16_t artH = art::drawLandmark(g, game::sim.locIndex, 0, 96);
    g.fillRect(0, artH, 320, 240 - artH, theme::BG);

    const float depth = game::riverDepthNow(*r_);
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_center);
    g.setTextColor(theme::ACCENT);
    g.drawString(r_->name, 160, artH + 2);
    char info[64];
    snprintf(info, sizeof(info), "~%d ft deep, %d ft wide - %s",
             (int)(depth + 0.5f), r_->widthFt, depthWord(depth));
    g.setTextColor(theme::INK_DIM);
    g.drawString(info, 160, artH + 18);

    if (showResult_) {
        g.setTextColor(result_.mishap ? theme::WARN : theme::INK);
        ui::drawText(g, result_.text, theme::MARGIN, artH + 36,
                     320 - 2 * theme::MARGIN, result_.mishap ? theme::WARN : theme::INK);
        ok_ = {(int16_t)(160 - 70), 208, 140, 28};
        ui::drawButton(g, ok_, result_.crossed ? "On we go" : "Now what?", true);
        return;
    }

    const int16_t r1 = artH + 38, r2 = r1 + 34;
    ford_  = {theme::MARGIN, r1, 150, 30};
    caulk_ = {166, r1, 146, 30};
    ferry_ = {theme::MARGIN, r2, 150, 30};
    wait_  = {166, r2, 146, 30};
    ui::drawButton(g, ford_, "Ford it");
    ui::drawButton(g, caulk_, "Caulk & float");
    const bool canFerry = r_->ferry && game::g.vehicle.cash >= r_->ferryCost;
    ui::drawButton(g, ferry_,
                   r_->ferry ? (String("Ferry $") + r_->ferryCost) : String("No ferry"),
                   canFerry, canFerry);
    ui::drawButton(g, wait_, "Wait it out");
}

void RiverScreen::onTap(int16_t x, int16_t y) {
    if (showResult_) {
        if (ok_.contains(x, y)) {
            if (result_.crossed) app::screens.pop();
            else showResult_ = false;
        }
        return;
    }
    if (!r_) { if (ok_.contains(x, y)) { game::sim.mustCross = false; app::screens.pop(); } return; }

    if (ford_.contains(x, y))       doChoice(game::CrossChoice::Ford);
    else if (caulk_.contains(x, y)) doChoice(game::CrossChoice::Caulk);
    else if (wait_.contains(x, y))  doChoice(game::CrossChoice::Wait);
    else if (ferry_.contains(x, y) && r_->ferry &&
             game::g.vehicle.cash >= r_->ferryCost)
        doChoice(game::CrossChoice::Ferry);
}
