// Core simulation state — pure C++, no Arduino. Numbers and rules are the
// Maxwolf/OregonTrail C# clone's, simplified where they were fiddly (this is an
// "inspired-by" port, not a mechanical one).
#pragma once
#include <stdint.h>
#include <string.h>

namespace game {

// ---- profession -----------------------------------------------------------
// Leader's profession applies to the whole party. Cash / score multiplier from
// ProfessionSelector.cs.
enum class Profession : uint8_t { Banker, Carpenter, Farmer };

struct ProfInfo {
    const char* label;
    const char* blurb;
    int         startCash;   // whole dollars
    int         scoreMult;
};

inline const ProfInfo& profInfo(Profession p) {
    static const ProfInfo k[] = {
        {"Banker from Boston",     "Most money to start, no score bonus.",       1600, 1},
        {"Carpenter from Ohio",    "Middle money, doubles your final score.",     800, 2},
        {"Farmer from Illinois",   "Least money, triples your final score.",      400, 3},
    };
    return k[static_cast<int>(p)];
}

// ---- start month ---------------------------------------------------------
// Earlier = more grass and milder passes but a longer wait for spring; later =
// risk of snow in the mountains. (StartingMonthEnum.cs offers Mar–Jul.)
enum class Month : uint8_t { March, April, May, June, July,
                             August, September, October, November,
                             December, January, February };

inline const char* monthName(Month m) {
    static const char* k[] = {"March", "April", "May", "June", "July", "August",
                              "September", "October", "November", "December",
                              "January", "February"};
    return k[static_cast<int>(m)];
}

// Calendar month number, 1 = January. Our enum starts at March.
inline int monthNumber(Month m) { return (static_cast<int>(m) + 2) % 12 + 1; }

// ---- people ------------------------------------------------------------
// Health as a 0..500 pool like the C# HealthStatus bands (500 Good, 400 Fair,
// 300 Poor, 200 "very poor", <=0 Dead).
enum class Health : uint8_t { Good, Fair, Poor, VeryPoor, Dead };

struct Person {
    char  name[16] = {0};
    int   health   = 500;
    bool  sick     = false;
    bool  alive    = true;
    char  causeOfDeath[24] = {0};

    void setName(const char* n) {
        strncpy(name, n, sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
    }
    Health band() const {
        if (!alive || health <= 0) return Health::Dead;
        if (health >= 450) return Health::Good;
        if (health >= 350) return Health::Fair;
        if (health >= 250) return Health::Poor;
        return Health::VeryPoor;
    }
};

inline const char* healthName(Health h) {
    static const char* k[] = {"good", "fair", "poor", "very poor", "dead"};
    return k[static_cast<int>(h)];
}

constexpr int kMaxParty = 5;

// Most meat one hunt can carry back to the wagon (HuntGame CarryCap).
constexpr int CarryCapLb = 100;

struct Party {
    Person     member[kMaxParty];
    int        count = 0;
    Profession profession = Profession::Banker;

    Person&       leader()       { return member[0]; }
    const Person& leader() const { return member[0]; }
    int aliveCount() const {
        int n = 0;
        for (int i = 0; i < count; ++i) if (member[i].alive) ++n;
        return n;
    }
};

// ---- vehicle / inventory ---------------------------------------------------
enum class Pace : uint8_t { Steady, Strenuous, Grueling };
enum class Rations : uint8_t { FillingRation, MeagerRation, BareBonesRation };

inline const char* paceName(Pace p) {
    static const char* k[] = {"steady", "strenuous", "grueling"};
    return k[static_cast<int>(p)];
}
inline const char* rationsName(Rations r) {
    static const char* k[] = {"filling", "meager", "bare bones"};
    return k[static_cast<int>(r)];
}

// Everything the wagon carries. Food in pounds, cash in whole dollars, bullets
// as individual rounds, parts/oxen/clothes as counts.
struct Vehicle {
    int cash     = 0;
    int food     = 0;
    int oxen     = 0;
    int clothes  = 0;
    int bullets  = 0;
    int wheels   = 0;
    int axles    = 0;
    int tongues  = 0;
    int medicine = 0;

    Pace    pace    = Pace::Steady;
    Rations rations = Rations::FillingRation;

    // Odometer (miles travelled since Independence) and current-day distance.
    int   odometer  = 0;
    int   milesToday = 0;
};

}  // namespace game
