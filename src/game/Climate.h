// Weather over the journey — simplified from ClimateModule.cs but using the
// original's per-zone monthly temperature and rain tables (ClimateRegistry.cs).
// One sky over the party; re-rolled about half of days so spells last.
#pragma once
#include <stdint.h>

#include "game/Trail.h"

namespace game {

enum class Sky : uint8_t { Clear, Rain, Snow };

struct Weather {
    int  tempF     = 55;
    int  band      = 3;      // 0 very cold .. 5 very hot; 2-3 comfortable
    Sky  sky       = Sky::Clear;
    float snowPack = 0.0f;   // lying snow; slows the wagon
    float wetness  = 0.0f;   // soaked ground; swells rivers (Phase 6)

    float snowDrag() const;  // 0 .. 1 fraction the wagon loses to snow
};

// Set up the sky for a journey leaving in `startMonthNum` (1 = January).
void climateInit(Weather& w, int startMonthNum);

// Advance one day. `climate` = the zone the party is currently in, `monthNum`
// 1..12. Re-rolls ~half the time; melts/dries the running totals.
void climateTick(Weather& w, Climate climate, int monthNum);

}  // namespace game
