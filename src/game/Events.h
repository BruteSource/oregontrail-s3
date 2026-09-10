// Weighted random events on a travel day — a representative slice of the C#
// clone's Event/** catalog (illness, thieves, weather, broken parts, lucky
// finds). Most resolve immediately; a broken wagon part hands a choice to the
// player (EventScreen).
#pragma once
#include <stdint.h>

namespace game {

enum class EventKind : uint8_t {
    None,
    Illness,       // someone sickens (or worse, if already ill)
    OxLost,        // an ox wanders off or dies
    BrokenPart,    // axle / wheel / tongue — needs a spare or days of repair
    Theft,         // a thief takes food / ammo / cash
    BadWeather,    // storm / fog / hail — time or supplies lost
    LostTrail,     // days lost finding the way
    StuckMud,      // a day lost
    GoodFind,      // berries, an abandoned wagon, help from strangers
};

enum class PartId : uint8_t { Axle, Wheel, Tongue };

struct GameEvent {
    EventKind kind = EventKind::None;
    char      headline[28] = {0};
    char      text[168] = {0};
    PartId    part = PartId::Axle;   // valid when kind == BrokenPart
    int       repairDays = 0;        // days lost if repairing without a spare
};

// Roll once for the current travel day. Applies the effect for every kind
// except BrokenPart (which the caller resolves). Returns false if nothing fired.
bool rollDailyEvent(GameEvent& out);

// Resolve a broken part. useSpare only succeeds if one is in the wagon.
// Returns the days lost (0 if a spare was used).
int resolveBrokenPart(const GameEvent& ev, bool useSpare);

const char* partName(PartId p);

extern bool eventsEnabled;   // dev toggle

}  // namespace game
