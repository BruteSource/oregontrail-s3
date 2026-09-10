// Headless simulation checks — `pio test -e native`.
// Auto-drives a full Independence -> Oregon City run and asserts the invariants
// that must hold however the dice fall.
#include <unity.h>
#include <cstdio>

#include "game/Rivers.h"
#include "game/Scoring.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "game/Store.h"
#include "game/Trail.h"

using namespace game;

void setUp() {}
void tearDown() {}

static void outfit(int foodLb, Pace pace, Rations rations) {
    g.beginNewGame(Profession::Banker);
    g.party.count = kMaxParty;
    for (int i = 0; i < kMaxParty; ++i) {
        g.party.member[i] = Person{};
        g.party.member[i].setName("Traveller");
    }
    applyPurchase(g.vehicle, ItemId::Oxen, 6, 0);
    applyPurchase(g.vehicle, ItemId::Food, foodLb, 0);
    applyPurchase(g.vehicle, ItemId::Clothes, 5, 0);
    applyPurchase(g.vehicle, ItemId::Bullets, 200, 0);
    applyPurchase(g.vehicle, ItemId::Wheel, 1, 0);
    g.vehicle.pace = pace;
    g.vehicle.rations = rations;
}

// A player who watches the larder and eases the pace should reach Oregon, with
// distance and the calendar behaving monotonically the whole way.
static void test_full_playthrough() {
    seedRng(0xC0FFEE);
    outfit(2000, Pace::Steady, Rations::FillingRation);
    sim.begin();

    int lastOdo = 0;
    int forks = 0;
    TurnResult r = TurnResult::Traveled;

    for (int guard = 0; guard < 3000; ++guard) {
        // stand-in for a player managing rations against remaining food
        g.vehicle.rations = g.vehicle.food > 900   ? Rations::FillingRation
                            : g.vehicle.food > 300 ? Rations::MeagerRation
                                                   : Rations::BareBonesRation;

        const int daysBefore = sim.totalDays;
        r = sim.takeTurn();

        if (r == TurnResult::ForkChoice) {
            sim.chooseBranch(forks++ % 2);   // alternate the branches
            TEST_ASSERT_EQUAL_INT(daysBefore, sim.totalDays);   // no day spent
            continue;
        }

        if (r == TurnResult::Event && sim.pendingEvent.kind == EventKind::BrokenPart)
            resolveBrokenPart(sim.pendingEvent, true);   // use a spare if we have one

        // A player restocks at forts: replace lost oxen, top up food.
        if (r == TurnResult::Arrived && sim.here().kind == Stop::Settlement) {
            if (g.vehicle.oxen < 4)
                applyPurchase(g.vehicle, ItemId::Oxen, 4 - g.vehicle.oxen + 1, sim.fortsReached);
            if (g.vehicle.food < 600)
                applyPurchase(g.vehicle, ItemId::Food, 800, sim.fortsReached);
        }

        // A player crosses the river they've arrived at.
        if (sim.mustCross) {
            const RiverInfo* ri = riverInfoForNode(sim.locIndex);
            if (ri) {
                CrossChoice c = (ri->ferry && g.vehicle.cash >= ri->ferryCost)
                                    ? CrossChoice::Ferry
                                : riverDepthNow(*ri) < 3.0f ? CrossChoice::Ford
                                                            : CrossChoice::Caulk;
                attemptCrossing(*ri, c);
            }
            sim.mustCross = false;
            if (g.party.aliveCount() == 0) { r = TurnResult::PartyWiped; break; }
        }

        TEST_ASSERT_GREATER_OR_EQUAL(lastOdo, sim.odometer());
        lastOdo = sim.odometer();
        TEST_ASSERT_GREATER_OR_EQUAL(0, g.vehicle.food);
        TEST_ASSERT_GREATER_OR_EQUAL(daysBefore + 1, sim.totalDays);  // events can burn extra days
        TEST_ASSERT_EQUAL_INT(sim.turns, sim.totalDays);
        TEST_ASSERT_TRUE(r != TurnResult::Blocked);   // full team, never stuck

        if (r == TurnResult::ReachedOregon || r == TurnResult::PartyWiped) break;
    }
    TEST_ASSERT_EQUAL_INT(3, forks);   // hit all three historic forks

    TEST_ASSERT_EQUAL_INT((int)TurnResult::ReachedOregon, (int)r);
    TEST_ASSERT_GREATER_OR_EQUAL(trailLongestMiles(), sim.odometer());
    TEST_ASSERT_GREATER_OR_EQUAL(1, g.party.aliveCount());
    TEST_ASSERT_LESS_THAN(300, sim.turns);   // a sane pace, no infinite crawl

    const Score sc = computeScore();
    TEST_ASSERT_GREATER_THAN(0, sc.total);
    TEST_ASSERT_EQUAL_INT(sc.subtotal * sc.multiplier, sc.total);

    char msg[128];
    snprintf(msg, sizeof(msg),
             "arrived day %d, %d/%d alive, %d lb food, score %d (%s)",
             sim.turns, g.party.aliveCount(), kMaxParty, g.vehicle.food,
             sc.total, sc.rating);
    TEST_MESSAGE(msg);
}

// Starve the party: someone must die, and the dead never come back.
static void test_dead_stay_dead() {
    seedRng(7);
    outfit(80, Pace::Grueling, Rations::BareBonesRation);
    g.vehicle.food = 0;
    sim.begin();

    bool deadFlag[kMaxParty] = {false};
    bool sawDeath = false;

    for (int i = 0; i < 500; ++i) {
        TurnResult r = sim.takeTurn();
        for (int j = 0; j < g.party.count; ++j) {
            const bool alive = g.party.member[j].alive;
            if (deadFlag[j]) TEST_ASSERT_FALSE(alive);   // no resurrection
            if (!alive) { deadFlag[j] = true; sawDeath = true; }
        }
        if (r == TurnResult::PartyWiped || r == TurnResult::ReachedOregon) break;
    }
    TEST_ASSERT_TRUE(sawDeath);
}

// No oxen -> the wagon cannot move.
static void test_no_oxen_blocks() {
    seedRng(1);
    outfit(500, Pace::Steady, Rations::FillingRation);
    g.vehicle.oxen = 0;
    sim.begin();
    const int odoBefore = sim.odometer();
    TurnResult r = sim.takeTurn();
    TEST_ASSERT_EQUAL_INT((int)TurnResult::Blocked, (int)r);
    TEST_ASSERT_EQUAL_INT(odoBefore, sim.odometer());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_full_playthrough);
    RUN_TEST(test_dead_stay_dead);
    RUN_TEST(test_no_oxen_blocks);
    return UNITY_END();
}
