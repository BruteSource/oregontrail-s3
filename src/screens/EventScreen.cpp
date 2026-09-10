#include "screens/EventScreen.h"

#include <string.h>

#include "art/Art.h"
#include "game/Events.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/Hud.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

namespace {
int spareStock(game::PartId p) {
    switch (p) {
        case game::PartId::Wheel:  return game::g.vehicle.wheels;
        case game::PartId::Tongue: return game::g.vehicle.tongues;
        default:                   return game::g.vehicle.axles;
    }
}

// A little illustration for the event, drawn centred at (cx, cy).
void drawEventArt(LGFX_Sprite& g, const game::GameEvent& ev, int16_t cx, int16_t cy) {
    using K = game::EventKind;
    auto has = [&](const char* s) { return strstr(ev.text, s) != nullptr; };
    auto headIs = [&](const char* s) { return strstr(ev.headline, s) != nullptr; };
    auto cloud = [&](int n, int w) { art::drawEvent(g, n, cx - w, cy - 18, 2.0f); };

    switch (ev.kind) {
        case K::BrokenPart:
        case K::StuckMud:
            art::drawWagon(g, 3, cx - 55, cy - 22, 1.4f);       // tipped wagon
            break;
        case K::GoodFind:
            if (has("wheel") || has("wagon"))
                art::drawWagon(g, 0, cx - 55, cy - 20, 1.3f);   // a whole wagon
            else if (has("meal") || has("medicine") || has("share"))
                art::drawEvent(g, 7, cx - 18, cy - 22, 1.8f);   // travellers
            else
                art::drawEvent(g, 1, cx - 25, cy - 15, 2.0f);   // berries
            break;
        case K::BadWeather:
            if (headIs("fog"))        cloud(2, 34);
            else if (headIs("Hail"))  cloud(3, 37);
            else                      cloud(5, 37);             // storm + bolt
            break;
        case K::LostTrail:
            cloud(2, 34);                                       // fog
            break;
        case K::OxLost:
            art::drawAnimal(g, 2, cx - 15, cy - 9);             // an ox
            break;
        case K::Theft:
        case K::Illness:
        default:
            art::drawEvent(g, 6, cx - 16, cy - 33, 2.6f);       // a figure
            break;
    }
}
}  // namespace

void EventScreen::onEnter() {
    resolved_ = game::sim.pendingEvent.kind != game::EventKind::BrokenPart;
    daysLost_ = 0;
}

void EventScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);
    hud::drawTrailStatus(g);

    const game::GameEvent& ev = game::sim.pendingEvent;
    const bool death = ev.kind == game::EventKind::Illness &&
                       strcmp(ev.headline, "A death") == 0;

    ui::drawCentered(g, ev.headline, 160, 30, death ? theme::WARN : theme::ACCENT, 4);
    drawEventArt(g, ev, 160, 92);
    ui::drawText(g, ev.text, theme::MARGIN, 118, 320 - 2 * theme::MARGIN,
                 theme::INK, 2, 17, /*centered=*/true);

    if (ev.kind == game::EventKind::BrokenPart && !resolved_) {
        const int stock = spareStock(ev.part);
        spare_  = {theme::MARGIN, 178, 150, 30};
        repair_ = {166, 178, 146, 30};
        const String spareLabel = stock > 0 ? String("Fit a spare (") + stock + ")"
                                            : String("No spare");
        ui::drawButton(g, spare_, spareLabel, stock > 0, stock > 0);
        ui::drawButton(g, repair_, String("Repair ~") + ev.repairDays + " days", true);
        return;
    }

    if (resolved_ && daysLost_ > 0) {
        char s[48];
        snprintf(s, sizeof(s), "%d days lost making the repair.", daysLost_);
        ui::drawCentered(g, s, 160, 160, theme::INK_DIM, 2);
    }

    ok_ = {(int16_t)(160 - 80), 196, 160, 30};
    ui::drawButton(g, ok_, "Continue", true);
}

void EventScreen::onTap(int16_t x, int16_t y) {
    const game::GameEvent& ev = game::sim.pendingEvent;

    if (ev.kind == game::EventKind::BrokenPart && !resolved_) {
        if (spare_.contains(x, y) && spareStock(ev.part) > 0) {
            game::resolveBrokenPart(ev, true);
            resolved_ = true;
        } else if (repair_.contains(x, y)) {
            daysLost_ = game::resolveBrokenPart(ev, false);
            resolved_ = true;
        }
        return;
    }

    if (ok_.contains(x, y)) {
        if (game::sim.pendingEvent.kind == game::EventKind::Illness &&
            game::g.party.aliveCount() == 0) {
            // party wiped by the event — fall through to game over
        }
        game::sim.pendingEvent = game::GameEvent{};
        app::screens.pop();
    }
}
