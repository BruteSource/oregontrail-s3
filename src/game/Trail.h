// The route from Independence to Oregon City as a small node graph, so the three
// historic forks are real choices: Fort Bridger vs the Green River ford, the
// Fort Walla Walla detour, and floating the Columbia vs the Barlow toll road.
// Distances / climate zones are the 1985 original's (TrailRegistry.cs).
#pragma once
#include <stdint.h>

namespace game {

enum class Climate : uint8_t {
    MissouriValley, GreatPlains, HighCountry, SnakeRiverPlain, PacificSlope
};

enum class Stop : uint8_t {
    Start,        // Independence
    Settlement,   // a fort — store + resupply
    Landmark,     // scenic
    River,        // a crossing (Phase 6)
    Fork,         // a choice of roads
    TollRoad,     // pay to pass
    End,          // Oregon City
};

struct Edge {
    int miles;    // distance of this leg
    int to;       // destination node index
};

struct Node {
    const char* name;
    Stop        kind;
    Climate     climate;
    bool        highGround;   // a pass: slower, small mishap risk
    int         rateOut;      // base miles/day on the leg leaving here
    Edge        edge[2];
    uint8_t     edgeCount;
    // fork prompts (kind == Fork)
    const char* optA;
    const char* optB;
    const char* blurbA;
    const char* blurbB;
};

const Node* trailNodes(int* count);

// Longest path Independence -> Oregon City, for progress bars / "miles to go".
int trailLongestMiles();

}  // namespace game
