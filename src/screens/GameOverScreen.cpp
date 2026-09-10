#include "screens/GameOverScreen.h"

#include "SaveGame.h"
#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/HighScoresScreen.h"
#include "screens/MainMenuScreen.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

void GameOverScreen::onEnter() {
    score_ = game::computeScore();
    rank_ = savegame::submitHighScore(game::g.party.leader().name, score_.total,
                                      (uint16_t)game::sim.turns, won_);
    savegame::clear();   // the journey is over
}

void GameOverScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);
    const game::Sim& s = game::sim;

    if (won_) {
        art::drawLandmark(g, 19, 0, 60);   // Willamette Valley strip
        g.fillRect(0, 60, 320, 22, theme::BG);
        ui::drawCentered(g, "OREGON CITY", 160, 62, theme::ACCENT, 2);
        char sub[48];
        snprintf(sub, sizeof(sub), "%d days   -   %d miles", s.turns, s.odometer());
        ui::drawCentered(g, sub, 160, 82, theme::INK_DIM, 2);
    } else {
        // tombstone on the left, only the name engraved on it
        const float sc = 90.0f / 384.0f;                 // ~131 px wide
        g.drawPng(art::tomb_img, art::tomb_img_len, 2, 0, 0, 0, 0, 0, sc, sc);
        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::middle_center);
        g.setTextColor(0x0000);
        g.drawString(game::g.party.leader().name, 66, 40);

        // the rest of the epitaph next to the stone
        g.setTextDatum(textdatum_t::top_left);
        g.setTextColor(theme::WARN);
        g.drawString("The whole party", 148, 16);
        g.drawString("is lost.", 148, 30);
        g.setTextColor(theme::INK_DIM);
        char d[24];
        snprintf(d, sizeof(d), "%d days", s.turns);
        g.drawString(d, 148, 52);
        snprintf(d, sizeof(d), "%d miles travelled", s.odometer());
        g.drawString(d, 148, 66);
    }

    // left column: the point breakdown
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_left);
    int16_t y = 98;
    auto row = [&](const char* label, int pts) {
        g.setTextColor(theme::INK);
        g.drawString(label, theme::MARGIN, y);
        g.setTextDatum(textdatum_t::top_right);
        g.setTextColor(theme::INK_DIM);
        g.drawString(String(pts), 150, y);
        g.setTextDatum(textdatum_t::top_left);
        y += 11;
    };
    row("Party health", score_.people);
    row("Wagon", score_.wagon);
    row("Oxen", score_.oxen);
    row("Spare parts", score_.parts);
    row("Clothing", score_.clothing);
    row("Ammunition", score_.bullets);
    row("Food", score_.food);
    row("Cash", score_.cash);
    g.drawFastHLine(theme::MARGIN, y + 1, 142, theme::FRAME);
    y += 4;
    g.setTextColor(theme::INK);
    g.drawString("Subtotal", theme::MARGIN, y);
    g.setTextDatum(textdatum_t::top_right);
    g.drawString(String(score_.subtotal), 150, y);
    g.setTextDatum(textdatum_t::top_left);

    // right column: the multiplied total
    const int16_t rmid = 244;
    g.setTextDatum(textdatum_t::top_center);
    g.setTextColor(theme::INK_DIM);
    g.drawString(String("x ") + score_.multiplier + " for your trade", rmid, 104);
    g.drawString("FINAL SCORE", rmid, 128);
    g.setFont(&fonts::Font4);
    g.setTextColor(theme::ACCENT);
    g.drawString(String(score_.total), rmid, 142);
    g.setFont(&fonts::Font2);
    g.setTextColor(theme::INK);
    g.drawString(score_.rating, rmid, 174);
    if (rank_ >= 0) {
        g.setTextColor(theme::ACCENT);
        g.drawString(String("Top Five  #") + (rank_ + 1), rmid, 190);
    }

    scores_ = {theme::MARGIN, 208, 150, 28};
    menu_   = {166, 208, 146, 28};
    ui::drawButton(g, scores_, "Top Five");
    ui::drawButton(g, menu_, "Main menu", true);
}

void GameOverScreen::onTap(int16_t x, int16_t y) {
    if (scores_.contains(x, y)) app::screens.push(new HighScoresScreen());
    else if (menu_.contains(x, y)) app::screens.reset(new MainMenuScreen());
}
