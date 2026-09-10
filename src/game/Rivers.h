// River crossings — ford, caulk & float, ferry, or wait it out. Depths/widths
// are the 1985 original's (TrailRegistry.cs river params); spring runoff
// (weather wetness) makes them worse.
#pragma once
#include <stdint.h>

namespace game {

struct RiverInfo {
    const char* name;
    float       baseDepthFt;
    int         widthFt;
    bool        ferry;        // a ferry operator works this crossing
    int         ferryCost;    // dollars
};

// nullptr if the node is not a river.
const RiverInfo* riverInfoForNode(int nodeIndex);

// Today's depth: base plus spring runoff.
float riverDepthNow(const RiverInfo& r);

enum class CrossChoice : uint8_t { Ford, Caulk, Ferry, Wait };

struct CrossResult {
    bool  crossed = false;      // false only for Wait
    bool  mishap  = false;
    int   daysLost = 0;
    char  text[168] = {0};
};

// Attempt the crossing. Applies effects (lost food, drowned ox, injuries, time,
// ferry fee) and returns what happened. `Wait` just passes 1-3 days and lowers
// the water.
CrossResult attemptCrossing(const RiverInfo& r, CrossChoice choice);

}  // namespace game
