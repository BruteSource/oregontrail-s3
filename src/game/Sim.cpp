#include "game/Sim.h"

#include <stdio.h>
#include <string.h>

#include "game/Session.h"

namespace game {

Sim sim;

namespace {
constexpr int kDaysInMonth = 30;

int aliveCount() {
    int n = 0;
    for (int i = 0; i < g.party.count; ++i)
        if (g.party.member[i].alive) ++n;
    return n;
}
int sickCount() {
    int n = 0;
    for (int i = 0; i < g.party.count; ++i)
        if (g.party.member[i].alive && g.party.member[i].sick) ++n;
    return n;
}
int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
}  // namespace

const Node& Sim::here() const {
    int n;
    const Node* t = trailNodes(&n);
    return t[locIndex < n ? locIndex : n - 1];
}

const char* Sim::nextName() const {
    int n;
    const Node* t = trailNodes(&n);
    const Node& h = here();
    if (h.edgeCount == 0) return h.name;
    const int e = (branch < h.edgeCount) ? branch : 0;
    return t[h.edge[e].to].name;
}

bool Sim::atOregon() const { return here().kind == Stop::End; }

int Sim::legLength() const {
    const Node& h = here();
    if (h.edgeCount == 0) return 0;
    const int e = (branch < h.edgeCount) ? branch : 0;
    return h.edge[e].miles;
}
int Sim::milesToNextStop() const {
    int r = legLength() - milesIntoLeg;
    return r < 0 ? 0 : r;
}
int Sim::milesRemaining() const {
    int r = trailLongestMiles() - odometer();
    return r < 0 ? 0 : r;
}

void Sim::begin() {
    locIndex = 0;
    branch = 0;
    milesIntoLeg = 0;
    departed = false;
    needBranch = false;
    mustCross = false;
    year = 1848;
    monthNum = monthNumber(g.startMonth);
    day = 1;
    totalDays = 0;
    turns = 0;
    totalMiles = 0;
    fortsReached = 0;
    fortDeparturePenalty = false;
    tollPaid = false;
    lastMiles = 0;
    lastNews[0] = 0;
    climateInit(weather, monthNum);
}

void Sim::chooseBranch(int edge) {
    const Node& h = here();
    if (h.kind != Stop::Fork) return;
    branch = clampi(edge, 0, h.edgeCount - 1);
    needBranch = false;
}

void Sim::advanceCalendar(int days) {
    for (int i = 0; i < days; ++i) {
        ++totalDays;
        if (++day > kDaysInMonth) {
            day = 1;
            if (++monthNum > 12) { monthNum = 1; ++year; }
        }
    }
}

int Sim::dailyMileage() {
    const Node& loc = here();
    if (g.vehicle.oxen <= 0) return -1;

    const float oxenFactor = g.vehicle.oxen >= 4 ? 1.0f : g.vehicle.oxen / 4.0f;
    const float paceFactor = 1.0f + 0.5f * static_cast<int>(g.vehicle.pace);
    float sickFactor = 1.0f - 0.1f * sickCount();
    if (sickFactor < 0) sickFactor = 0;
    const float snowFactor = 1.0f - weather.snowDrag();

    int miles = (int)(loc.rateOut * oxenFactor * paceFactor * sickFactor * snowFactor);
    if (fortDeparturePenalty) { miles /= 4; fortDeparturePenalty = false; }
    if (loc.highGround) miles -= miles / 3;
    if (miles < 0) miles = 0;
    return miles;
}

int Sim::dailyMileageEstimate() const {
    const Node& loc = here();
    if (g.vehicle.oxen <= 0) return 0;
    const float oxenFactor = g.vehicle.oxen >= 4 ? 1.0f : g.vehicle.oxen / 4.0f;
    const float paceFactor = 1.0f + 0.5f * static_cast<int>(g.vehicle.pace);
    float sickFactor = 1.0f - 0.1f * sickCount();
    if (sickFactor < 0) sickFactor = 0;
    int miles = (int)(loc.rateOut * oxenFactor * paceFactor * sickFactor *
                      (1.0f - weather.snowDrag()));
    if (fortDeparturePenalty) miles /= 4;
    if (loc.highGround) miles -= miles / 3;
    return miles < 0 ? 0 : miles;
}

static int healthDelta(const Person& p, bool traveling) {
    const Vehicle& v = g.vehicle;
    int delta = 4;
    const int alive = aliveCount();
    const float clothesPerHead = alive > 0 ? (float)v.clothes / alive : 0;

    if (sim.weather.band <= 1)
        delta -= (clothesPerHead >= 1.0f) ? 4 : 12;
    else if (sim.weather.band >= 5)
        delta -= 6;

    if (v.food <= 0)                                delta -= 18;
    else if (v.rations == Rations::BareBonesRation) delta -= 7;
    else if (v.rations == Rations::MeagerRation)    delta -= 3;
    else                                           delta += 2;

    if (traveling) {
        delta -= 2 * static_cast<int>(v.pace);
        if (sim.weather.sky != Sky::Clear) delta -= 3;
    } else {
        delta += 8;
    }
    if (p.sick) delta -= 7;
    return clampi(delta, -25, 15);
}

void Sim::liveTheDay(bool traveling) {
    Vehicle& v = g.vehicle;
    const int alive = aliveCount();

    const int perHead = 3 - static_cast<int>(v.rations);
    v.food -= perHead * alive;
    if (v.food < 0) v.food = 0;

    for (int i = 0; i < g.party.count; ++i) {
        Person& p = g.party.member[i];
        if (!p.alive) continue;
        p.health = clampi(p.health + healthDelta(p, traveling), 0, 500);
    }

    int worst = -1, worstH = 1 << 30;
    for (int i = 0; i < g.party.count; ++i) {
        const Person& p = g.party.member[i];
        if (!p.alive) continue;
        if (i == 0 && alive > 1) continue;   // leader shielded
        if (p.health < worstH) { worstH = p.health; worst = i; }
    }
    lastNews[0] = 0;
    if (worst >= 0 && worstH <= 0) {
        Person& p = g.party.member[worst];
        if (p.sick) {
            p.alive = false;
            p.health = 0;
            strncpy(p.causeOfDeath, "illness", sizeof(p.causeOfDeath) - 1);
            snprintf(lastNews, sizeof(lastNews), "%s has died of illness.", p.name);
        } else {
            p.sick = true;
            p.health = 55;
            snprintf(lastNews, sizeof(lastNews), "%s has fallen ill.", p.name);
        }
    }

    for (int i = 0; i < g.party.count; ++i) {
        Person& p = g.party.member[i];
        if (p.alive && p.sick && p.health >= 350 && rngChance(traveling ? 8 : 25)) {
            p.sick = false;
            if (!lastNews[0])
                snprintf(lastNews, sizeof(lastNews), "%s is well again.", p.name);
        }
    }

    if (v.food <= 0 && v.oxen > 0) {
        v.oxen -= 1;
        oxStarved = true;
        snprintf(lastNews, sizeof(lastNews),
                 v.oxen == 0 ? "The last ox has starved."
                             : "An ox starved for want of food.");
    }
}

void Sim::onArrive() {
    const Node& h = here();
    const int e = (branch < h.edgeCount) ? branch : 0;
    const int dest = h.edge[e].to;

    milesIntoLeg = 0;
    locIndex = dest;
    branch = 0;
    departed = false;

    int n;
    const Node* t = trailNodes(&n);
    if (t[locIndex].kind == Stop::Settlement) ++fortsReached;
    mustCross = (t[locIndex].kind == Stop::River);
}

TurnResult Sim::takeTurn() {
    oxStarved = false;
    if (atOregon()) return TurnResult::ReachedOregon;

    if (here().kind == Stop::Fork && !departed && needBranch)
        return TurnResult::ForkChoice;

    if (!departed) {
        departed = true;
        if (here().kind == Stop::Fork) { needBranch = true; return TurnResult::ForkChoice; }
        if ((here().kind == Stop::Settlement) && locIndex > 0)
            fortDeparturePenalty = true;
    }

    const int miles = dailyMileage();
    if (miles < 0) {
        advanceCalendar(1);
        ++turns;
        liveTheDay(false);
        lastMiles = 0;
        return aliveCount() == 0 ? TurnResult::PartyWiped : TurnResult::Blocked;
    }

    advanceCalendar(1);
    ++turns;
    climateTick(weather, here().climate, monthNum);
    liveTheDay(true);

    // Don't overshoot the landmark — the party pulls up when it's reached.
    int covered = miles;
    if (milesIntoLeg + covered > legLength()) covered = legLength() - milesIntoLeg;
    milesIntoLeg += covered;
    lastMiles = covered;

    if (aliveCount() == 0) return TurnResult::PartyWiped;

    if (milesIntoLeg >= legLength()) {
        totalMiles += legLength();
        onArrive();
        if (atOregon()) return TurnResult::ReachedOregon;
        return TurnResult::Arrived;
    }

    if (rollDailyEvent(pendingEvent)) {
        if (aliveCount() == 0) return TurnResult::PartyWiped;
        return TurnResult::Event;
    }
    return TurnResult::Traveled;
}

void Sim::rest(int days) {
    for (int d = 0; d < days; ++d) {
        advanceCalendar(1);
        ++turns;                 // a rest day is still a day/turn
        climateTick(weather, here().climate, monthNum);
        liveTheDay(false);
        if (aliveCount() == 0) return;
    }
}

}  // namespace game
