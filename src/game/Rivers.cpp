#include "game/Rivers.h"

#include <stdio.h>
#include <string.h>

#include "game/Session.h"
#include "game/Sim.h"
#include "game/Trail.h"

namespace game {

namespace {

// keyed by trail node name (see game/Trail.cpp)
const RiverInfo kRivers[] = {
    {"Kansas River",   1.0f, 620, true,  5},
    {"Big Blue River", 1.5f, 220, false, 0},
    {"Green River",    12.0f, 400, true, 12},
    {"Snake River",    6.0f, 1000, false, 0},   // Indian guide in the original
    {"Columbia River", 18.0f, 600, false, 0},   // the raft run — Phase 7 minigame
};

int aliveCount() {
    int n = 0;
    for (int i = 0; i < g.party.count; ++i)
        if (g.party.member[i].alive) ++n;
    return n;
}

void hurtSomeone(char* msg, size_t cap, const char* how) {
    int nth = rngRange(0, aliveCount());
    for (int i = 0; i < g.party.count; ++i) {
        Person& p = g.party.member[i];
        if (!p.alive) continue;
        if (nth-- > 0) continue;
        if (p.sick || p.health < 90) {
            p.alive = false;
            p.health = 0;
            strncpy(p.causeOfDeath, "drowning", sizeof(p.causeOfDeath) - 1);
            snprintf(msg, cap, "%s %s and was swept away.", p.name, how);
        } else {
            p.sick = true;
            p.health = p.health > 120 ? p.health - 120 : 20;
            snprintf(msg, cap, "%s %s and was hurt.", p.name, how);
        }
        return;
    }
}

void floodLoss(CrossResult& res, const char* verb) {
    res.mishap = true;
    Vehicle& v = g.vehicle;
    const int foodLost = v.food / 3 + rngRange(0, 40);
    v.food -= foodLost > v.food ? v.food : foodLost;
    char who[96] = {0};
    if (rngChance(35) && v.oxen > 0) {
        v.oxen -= 1;
        strncpy(who, " An ox drowned.", sizeof(who) - 1);
    } else if (rngChance(30)) {
        char h[96];
        hurtSomeone(h, sizeof(h), "was thrown into the water");
        snprintf(who, sizeof(who), " %s", h);
    }
    snprintf(res.text, sizeof(res.text),
             "The wagon %s. About %d pounds of supplies are lost.%s",
             verb, foodLost, who);
}

}  // namespace

const RiverInfo* riverInfoForNode(int nodeIndex) {
    int n;
    const Node* nd = trailNodes(&n);
    if (nodeIndex < 0 || nodeIndex >= n) return nullptr;
    if (nd[nodeIndex].kind != Stop::River) return nullptr;
    for (auto& r : kRivers)
        if (strcmp(r.name, nd[nodeIndex].name) == 0) return &r;
    return nullptr;
}

float riverDepthNow(const RiverInfo& r) {
    float d = r.baseDepthFt + sim.weather.wetness * 0.35f;
    return d < 0.3f ? 0.3f : d;
}

CrossResult attemptCrossing(const RiverInfo& r, CrossChoice choice) {
    CrossResult res;
    const float depth = riverDepthNow(r);

    switch (choice) {
        case CrossChoice::Wait: {
            const int days = 1 + rngRange(0, 3);
            sim.rest(days);
            sim.weather.wetness -= 1.5f * days;
            if (sim.weather.wetness < 0) sim.weather.wetness = 0;
            res.daysLost = days;
            snprintf(res.text, sizeof(res.text),
                     "You wait %d days. The water has dropped some.", days);
            return res;
        }
        case CrossChoice::Ferry: {
            if (!r.ferry) { snprintf(res.text, sizeof(res.text), "There is no ferry here."); return res; }
            if (g.vehicle.cash < r.ferryCost) {
                snprintf(res.text, sizeof(res.text),
                         "The ferry costs $%d and you cannot pay it.", r.ferryCost);
                return res;
            }
            g.vehicle.cash -= r.ferryCost;
            const int wait = rngRange(0, 3);
            if (wait) sim.rest(wait);
            res.crossed = true;
            res.daysLost = wait;
            snprintf(res.text, sizeof(res.text),
                     "You pay $%d and wait %d day%s. The ferry takes the wagon "
                     "across without trouble.", r.ferryCost, wait,
                     wait == 1 ? "" : "s");
            return res;
        }
        case CrossChoice::Ford: {
            res.crossed = true;
            int badPct = depth < 2.0f    ? 3
                         : depth < 3.5f  ? 15 + (int)((depth - 2.0f) * 25)
                                         : 55 + (int)((depth - 3.5f) * 12);
            if (badPct > 92) badPct = 92;
            if (rngChance(badPct)) {
                floodLoss(res, "is caught by the current and floods");
                sim.rest(1);
                res.daysLost = 1;
            } else {
                snprintf(res.text, sizeof(res.text),
                         "The team hauls the wagon across the shallows. You make it.");
            }
            return res;
        }
        case CrossChoice::Caulk: {
            res.crossed = true;
            int badPct = depth < 4.0f   ? 10
                         : depth < 9.0f ? 12 + (int)((depth - 4.0f) * 8)
                                        : 52 + (int)((depth - 9.0f) * 10);
            if (badPct > 90) badPct = 90;
            if (rngChance(badPct)) {
                floodLoss(res, "is swamped and nearly tips");
                sim.rest(1);
                res.daysLost = 1;
            } else {
                snprintf(res.text, sizeof(res.text),
                         "Sealed tight, the wagon floats across like a barge.");
            }
            return res;
        }
    }
    return res;
}

}  // namespace game
