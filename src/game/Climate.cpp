#include "game/Climate.h"

#include "game/Session.h"

namespace game {

namespace {

// Coldest expected temperature (F) per zone, by month (Jan first) — the 1985
// original's VAR.BIN table via ClimateRegistry.cs. A day's reading is drawn from
// a ~40 degree band starting here.
const int kMonthlyLow[5][12] = {
    { 9, 13, 23, 36, 45, 55, 60, 58, 50, 39, 25, 14},  // Missouri valley
    { 3,  8, 16, 29, 39, 49, 55, 53, 43, 31, 17,  7},  // Great plains
    { 3,  7, 12, 22, 32, 42, 51, 49, 38, 27, 13,  6},  // High country
    {-1,  4, 12, 23, 32, 41, 49, 47, 37, 26, 11,  1},  // Snake river plain
    {10, 16, 22, 30, 38, 45, 54, 52, 43, 32, 20, 12},  // Pacific slope
};

// Chance of precipitation per day, per zone, by month.
const int kRainPct[5][12] = {
    { 4,  4,  8, 10, 14, 14, 12, 12, 13,  9,  6,  5},
    { 2,  2,  3,  6,  9, 10,  8,  7,  5,  3,  2,  2},
    { 2,  2,  3,  5,  6,  4,  3,  2,  3,  3,  2,  2},
    { 2,  2,  4,  7,  8,  4,  2,  2,  3,  4,  2,  2},
    { 5,  4,  4,  4,  4,  3,  1,  1,  2,  3,  4,  4},
};

int bandFor(int tempF) {
    if (tempF < 20) return 0;
    if (tempF < 35) return 1;
    if (tempF < 55) return 2;
    if (tempF < 70) return 3;
    if (tempF < 85) return 4;
    return 5;
}

void roll(Weather& w, Climate climate, int monthNum, bool force) {
    if (!force && !rngChance(50)) return;   // keep a spell going

    const int zi = static_cast<int>(climate);
    const int mi = (monthNum - 1) % 12;
    const int low = kMonthlyLow[zi][mi];
    w.tempF = low + rngRange(0, 41);
    w.band = bandFor(w.tempF);

    if (rngChance(kRainPct[zi][mi] + (int)(w.wetness))) {
        w.sky = (w.tempF <= 32) ? Sky::Snow : Sky::Rain;
    } else {
        w.sky = Sky::Clear;
    }
}

}  // namespace

float Weather::snowDrag() const {
    float d = snowPack / 40.0f;
    return d < 0 ? 0 : (d > 1 ? 1 : d);
}

void climateInit(Weather& w, int startMonthNum) {
    w = Weather{};
    // Early in the year the country is still soaked and rivers run high; snow is
    // only still lying if leaving before April (ClimateModule ctor).
    w.wetness = (float)(7 - startMonthNum) + (rngRange(0, 100) / 100.0f);
    if (w.wetness < 0) w.wetness = 0;
    w.snowPack = (startMonthNum < 4) ? (rngRange(0, 100) / 100.0f) * 12.0f : 0.0f;
    roll(w, Climate::MissouriValley, startMonthNum, true);
}

void climateTick(Weather& w, Climate climate, int monthNum) {
    roll(w, climate, monthNum, false);

    // Rain soaks in and dries quickly; snow lingers, then melts when it warms.
    if (w.sky == Sky::Rain) w.wetness += 0.5f;
    if (w.sky == Sky::Snow) w.snowPack += 2.0f;
    w.wetness -= 0.1f;
    if (w.wetness < 0) w.wetness = 0;
    if (w.band >= 3 && w.snowPack > 0) {       // a thaw
        w.snowPack -= 1.0f;
        w.wetness += 0.3f;
    }
    if (w.snowPack < 0) w.snowPack = 0;
}

}  // namespace game
