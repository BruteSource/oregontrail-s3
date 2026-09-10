// The single in-progress game. New-game screens fill this in step by step; the
// travel loop (Phase 3) reads and advances it.
#pragma once
#include "game/Model.h"

namespace game {

struct Session {
    Party   party;
    Vehicle vehicle;
    Month   startMonth = Month::April;

    // Filled during setup so the store knows the leader's purse and profession.
    void beginNewGame(Profession prof);
};

extern Session g;

// RNG hook — seeded from esp_random() on device, from a fixed seed in tests.
void seedRng(uint32_t seed);
uint32_t rngNext();                 // xorshift32
int  rngRange(int loInclusive, int hiExclusive);
bool rngChance(int percent);

}  // namespace game
