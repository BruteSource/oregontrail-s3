#include "game/Scoring.h"

#include "game/Session.h"

namespace game {

static int bandValue(Health h) {
    switch (h) {
        case Health::Good:     return 500;
        case Health::Fair:     return 400;
        case Health::Poor:     return 300;
        case Health::VeryPoor: return 200;
        default:               return 0;
    }
}

Score computeScore() {
    Score s;
    const Vehicle& v = g.vehicle;

    for (int i = 0; i < g.party.count; ++i) {
        const Person& p = g.party.member[i];
        if (p.alive) s.people += bandValue(p.band());
    }
    s.wagon    = 50;
    s.oxen     = v.oxen * 4;
    s.parts    = (v.axles + v.wheels + v.tongues) * 2;
    s.clothing = v.clothes * 2;
    s.bullets  = v.bullets / 50;
    s.food     = v.food / 25;
    s.cash     = v.cash / 5;

    s.subtotal = s.people + s.wagon + s.oxen + s.parts + s.clothing +
                 s.bullets + s.food + s.cash;
    s.multiplier = profInfo(g.party.profession).scoreMult;
    s.total = s.subtotal * s.multiplier;

    s.rating = s.total >= 6000 ? "Trail Guide"
               : s.total >= 3000 ? "Adventurer"
                                 : "Greenhorn";
    return s;
}

}  // namespace game
