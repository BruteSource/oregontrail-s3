#include "game/Trail.h"

namespace game {

namespace {
constexpr int PLAINS = 20;
constexpr int MTN = 12;
constexpr Climate MV = Climate::MissouriValley;
constexpr Climate GP = Climate::GreatPlains;
constexpr Climate HC = Climate::HighCountry;
constexpr Climate SR = Climate::SnakeRiverPlain;
constexpr Climate PS = Climate::PacificSlope;

// index:               0            1           2            3
//        4            5            6            7 (fork)
//        8            9            10           11           12          13
//        14 (fork)    15           16 (fork)    17           18           19
const Node kNodes[] = {
    // name, kind, climate, highGround, rateOut, edges{...}, edgeCount, optA, optB, blurbA, blurbB
    {"Independence",      Stop::Start,      MV, false, PLAINS, {{102, 1}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Kansas River",      Stop::River,      MV, false, PLAINS, {{ 83, 2}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Big Blue River",    Stop::River,      MV, false, PLAINS, {{119, 3}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Fort Kearney",      Stop::Settlement, GP, false, PLAINS, {{250, 4}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Chimney Rock",      Stop::Landmark,   GP, false, PLAINS, {{ 86, 5}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Fort Laramie",      Stop::Settlement, GP, false, MTN,    {{190, 6}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Independence Rock", Stop::Landmark,   HC, false, MTN,    {{102, 7}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"South Pass",        Stop::Fork,       HC, true,  MTN,    {{125, 8}, {57, 9}}, 2,
        "Fort Bridger road", "Green River ford",
        "Longer, but a fort to resupply at and one less river.",
        "Shorter by 30+ miles. The Green runs deep and fast."},
    {"Fort Bridger",      Stop::Settlement, HC, false, MTN,    {{ 60, 10}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Green River",       Stop::River,      HC, false, MTN,    {{ 90, 10}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Soda Springs",      Stop::Landmark,   HC, false, MTN,    {{ 57, 11}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Fort Hall",         Stop::Settlement, SR, false, MTN,    {{182, 12}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Snake River",       Stop::River,      SR, false, MTN,    {{114, 13}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Fort Boise",        Stop::Settlement, SR, false, MTN,    {{ 65, 14}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Blue Mountains",    Stop::Fork,       PS, true,  MTN,    {{55, 15}, {125, 16}}, 2,
        "By Fort Walla Walla", "Straight through",
        "A detour to the fort — 65 extra miles, but a resupply.",
        "The hard road over the Blues, and a chance to get stuck."},
    {"Fort Walla Walla",  Stop::Settlement, PS, false, MTN,    {{ 65, 16}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"The Dalles",        Stop::Fork,       PS, false, MTN,    {{4, 17}, {4, 18}}, 2,
        "Barlow Toll Road", "Float the Columbia",
        "A safe road over the mountain shoulder. Costs a toll.",
        "Run the wagon down the river on a raft. Free, and dangerous."},
    {"Barlow Toll Road",  Stop::TollRoad,   PS, false, MTN,    {{100, 19}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Columbia River",    Stop::River,      PS, false, MTN,    {{100, 19}}, 1, nullptr, nullptr, nullptr, nullptr},
    {"Oregon City",       Stop::End,        PS, false, MTN,    {{0, 19}}, 0, nullptr, nullptr, nullptr, nullptr},
};
}  // namespace

const Node* trailNodes(int* count) {
    *count = sizeof(kNodes) / sizeof(kNodes[0]);
    return kNodes;
}

int trailLongestMiles() {
    // DFS the DAG picking the max-mileage path from node 0.
    int n;
    const Node* nd = trailNodes(&n);
    int best[32] = {0};
    for (int i = n - 1; i >= 0; --i) {
        int b = 0;
        for (int e = 0; e < nd[i].edgeCount; ++e) {
            const int cand = nd[i].edge[e].miles + best[nd[i].edge[e].to];
            if (cand > b) b = cand;
        }
        best[i] = b;
    }
    return best[0];
}

}  // namespace game
