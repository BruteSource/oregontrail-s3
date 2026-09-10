// End-of-game score, from ScoringModule.cs / FinalPoints.cs: sum the party's
// health and what the wagon still carries, times the leader's profession
// multiplier. Max possible is 13,860.
#pragma once
#include <stdint.h>

namespace game {

struct Score {
    int people   = 0;   // living members x their health band value
    int wagon    = 0;    // 50
    int oxen     = 0;    // 4 each
    int parts    = 0;    // 2 each (axle + wheel + tongue)
    int clothing = 0;    // 2 each
    int bullets  = 0;    // 1 per 50 rounds
    int food     = 0;    // 1 per 25 lb
    int cash     = 0;    // 1 per $5

    int subtotal   = 0;
    int multiplier = 1;
    int total      = 0;

    const char* rating = "Greenhorn";   // <3000 Greenhorn, <6000 Adventurer, else Trail Guide
};

Score computeScore();

}  // namespace game
