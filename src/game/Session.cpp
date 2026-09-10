#include "game/Session.h"

namespace game {

Session g;

void Session::beginNewGame(Profession prof) {
    party = Party{};
    vehicle = Vehicle{};
    party.profession = prof;
    vehicle.cash = profInfo(prof).startCash;
    startMonth = Month::April;
}

// ---- tiny xorshift32 RNG (deterministic for the headless playthrough test) --
static uint32_t s_rng = 0x1234567u;

void seedRng(uint32_t seed) { s_rng = seed ? seed : 1u; }

uint32_t rngNext() {
    uint32_t x = s_rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    s_rng = x;
    return x;
}

int rngRange(int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(rngNext() % (uint32_t)(hi - lo));
}

bool rngChance(int percent) { return (int)(rngNext() % 100u) < percent; }

}  // namespace game
