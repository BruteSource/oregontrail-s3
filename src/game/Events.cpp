#include "game/Events.h"

#include <stdio.h>
#include <string.h>

#include "game/Session.h"
#include "game/Sim.h"

namespace game {

namespace {

int aliveIdx(int nth) {   // index of the nth (0-based) living traveller
    for (int i = 0; i < g.party.count; ++i)
        if (g.party.member[i].alive && nth-- == 0) return i;
    return -1;
}
int aliveCount() {
    int n = 0;
    for (int i = 0; i < g.party.count; ++i)
        if (g.party.member[i].alive) ++n;
    return n;
}

void set(GameEvent& e, EventKind k, const char* head, const char* body) {
    e.kind = k;
    strncpy(e.headline, head, sizeof(e.headline) - 1);
    strncpy(e.text, body, sizeof(e.text) - 1);
}

// --- individual effects --------------------------------------------------
void doIllness(GameEvent& e) {
    static const char* names[] = {"dysentery", "measles",  "typhoid fever",
                                  "mountain fever", "a snakebite", "exhaustion",
                                  "a broken arm", "cholera"};
    const int who = aliveIdx(rngRange(0, aliveCount()));
    if (who < 0) { e.kind = EventKind::None; return; }
    Person& p = g.party.member[who];
    const char* ill = names[rngRange(0, 8)];

    char body[168];
    if (p.sick || p.health < 120) {
        // already ailing — this can be the end
        if (rngChance(45)) {
            p.alive = false;
            p.health = 0;
            strncpy(p.causeOfDeath, ill, sizeof(p.causeOfDeath) - 1);
            snprintf(body, sizeof(body),
                     "%s was already ailing, and %s has taken them.", p.name, ill);
            set(e, EventKind::Illness, "A death", body);
            return;
        }
        p.health = p.health > 40 ? p.health - 40 : 0;
        snprintf(body, sizeof(body), "%s has taken a turn for the worse.", p.name);
    } else {
        p.sick = true;
        p.health = p.health > 90 ? p.health - 90 : 10;
        snprintf(body, sizeof(body), "%s has come down with %s.", p.name, ill);
    }
    set(e, EventKind::Illness, "Illness", body);
}

void doOxLost(GameEvent& e) {
    if (g.vehicle.oxen <= 0) { e.kind = EventKind::None; return; }
    g.vehicle.oxen -= 1;
    if (rngChance(50))
        set(e, EventKind::OxLost, "An ox lost",
            "One of the oxen wandered off in the night and could not be found.");
    else
        set(e, EventKind::OxLost, "An ox lost", "One of the oxen has died.");
}

void doTheft(GameEvent& e) {
    Vehicle& v = g.vehicle;
    const int pick = rngRange(0, 3);
    char body[168];
    if (pick == 0 && v.food > 40) {
        const int amt = 20 + rngRange(0, 40);
        v.food -= amt > v.food ? v.food : amt;
        snprintf(body, sizeof(body), "A thief crept in and made off with %d "
                                     "pounds of food.", amt);
    } else if (pick == 1 && v.bullets >= 20) {
        const int amt = 20 * (1 + rngRange(0, 3));
        v.bullets -= amt > v.bullets ? v.bullets : amt;
        snprintf(body, sizeof(body), "A thief made off with %d rounds of "
                                     "ammunition.", amt);
    } else {
        const int amt = 5 + rngRange(0, 20);
        v.cash -= amt > v.cash ? v.cash : amt;
        snprintf(body, sizeof(body), "A thief lifted $%d from the wagon while "
                                     "the camp slept.", amt);
    }
    set(e, EventKind::Theft, "Thief in camp", body);
}

void doBadWeather(GameEvent& e) {
    Vehicle& v = g.vehicle;
    const int pick = rngRange(0, 3);
    if (pick == 0) {
        set(e, EventKind::BadWeather, "Severe storm",
            "A violent storm pins the party down. A day is lost.");
        sim.rest(1);
    } else if (pick == 1) {
        const int lost = v.food / 10;
        v.food -= lost;
        char body[168];
        snprintf(body, sizeof(body), "Hail battered the wagon and spoiled %d "
                                     "pounds of food.", lost);
        set(e, EventKind::BadWeather, "Hailstorm", body);
    } else {
        set(e, EventKind::BadWeather, "Heavy fog",
            "Thick fog rolls in and the party loses the trail for a day.");
        sim.rest(1);
    }
}

void doGoodFind(GameEvent& e) {
    Vehicle& v = g.vehicle;
    const int pick = rngRange(0, 3);
    char body[168];
    if (pick == 0) {
        const int amt = 15 + rngRange(0, 35);
        v.food += amt;
        snprintf(body, sizeof(body), "The party finds wild berries and game - "
                                     "%d pounds of food.", amt);
        set(e, EventKind::GoodFind, "A good day", body);
    } else if (pick == 1) {
        v.wheels += 1;
        set(e, EventKind::GoodFind, "Abandoned wagon",
            "An abandoned wagon by the trail yields a usable spare wheel.");
    } else {
        const int who = aliveIdx(rngRange(0, aliveCount()));
        if (who >= 0 && g.party.member[who].sick && rngChance(60)) {
            g.party.member[who].sick = false;
            snprintf(body, sizeof(body), "Travellers share medicine, and %s "
                                         "recovers.", g.party.member[who].name);
        } else {
            v.food += 20;
            snprintf(body, sizeof(body), "Friendly travellers share a meal and "
                                         "20 pounds of food.");
        }
        set(e, EventKind::GoodFind, "Kindness", body);
    }
}

}  // namespace

const char* partName(PartId p) {
    switch (p) {
        case PartId::Wheel:  return "wheel";
        case PartId::Tongue: return "tongue";
        default:             return "axle";
    }
}

bool eventsEnabled = true;   // dev toggle ('x' serial command)

bool rollDailyEvent(GameEvent& out) {
    out = GameEvent{};
    if (!eventsEnabled) return false;

    // ~14% of travel days bring something. Rougher country brings more.
    int chance = 14;
    if (sim.here().highGround) chance += 8;
    if (!rngChance(chance)) return false;

    // weighted pick
    const int roll = rngRange(0, 100);
    if (roll < 26) {
        doIllness(out);
    } else if (roll < 40) {
        doTheft(out);
    } else if (roll < 54) {
        doBadWeather(out);
    } else if (roll < 66) {
        doOxLost(out);
    } else if (roll < 80) {
        // broken part — resolved by the caller
        out.kind = EventKind::BrokenPart;
        out.part = static_cast<PartId>(rngRange(0, 3));
        out.repairDays = 2 + rngRange(0, 4);
        snprintf(out.headline, sizeof(out.headline), "Broken %s", partName(out.part));
        snprintf(out.text, sizeof(out.text),
                 "The wagon %s has broken. You can fit a spare if you have one, "
                 "or make camp and repair it - several days lost.",
                 partName(out.part));
    } else if (roll < 88) {
        set(out, EventKind::LostTrail, "Lost the trail",
            "The party wanders off the trail. Two days lost finding it again.");
        sim.rest(2);
    } else if (roll < 94) {
        set(out, EventKind::StuckMud, "Stuck in the mud",
            "The wagon bogs down to the axles. A day lost digging out.");
        sim.rest(1);
    } else {
        doGoodFind(out);
    }

    return out.kind != EventKind::None;
}

int resolveBrokenPart(const GameEvent& ev, bool useSpare) {
    int* stock = ev.part == PartId::Wheel    ? &g.vehicle.wheels
                 : ev.part == PartId::Tongue ? &g.vehicle.tongues
                                             : &g.vehicle.axles;
    if (useSpare && *stock > 0) {
        --*stock;
        return 0;
    }
    sim.rest(ev.repairDays);
    return ev.repairDays;
}

}  // namespace game
