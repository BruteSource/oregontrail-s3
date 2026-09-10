// The turn engine: one simulated day of travel at a time. Distilled from
// DriveTick.cs + Vehicle/Person OnTick — same shape (mileage, then the day is
// lived, then arrival is checked) with a simpler health model. Walks the trail
// node graph (game/Trail.h); forks pause the loop for the player to choose.
#pragma once
#include <stdint.h>

#include "game/Climate.h"
#include "game/Events.h"
#include "game/Trail.h"

namespace game {

enum class TurnResult : uint8_t {
    Traveled,       // an ordinary day on the trail
    Event,          // something happened — see pendingEvent
    Arrived,        // reached the next node — stop here
    ForkChoice,     // sitting at a fork, waiting for chooseBranch()
    Blocked,        // can't move: no oxen
    PartyWiped,     // everyone is dead
    ReachedOregon,  // the wagon rolled into Oregon City
};

struct Sim {
    int  locIndex = 0;       // current trail node
    int  branch = 0;         // which outgoing edge is being travelled
    int  milesIntoLeg = 0;
    bool departed = false;
    bool needBranch = false; // at a fork, no branch chosen yet
    bool mustCross = false;  // arrived at a river, not yet crossed

    int  year = 1848;
    int  monthNum = 4;       // 1..12
    int  day = 1;
    int  totalDays = 0;
    int  turns = 0;
    int  totalMiles = 0;     // real odometer (path varies, so accumulated)
    int  fortsReached = 0;   // store markup steps

    Weather weather;
    bool    fortDeparturePenalty = false;
    bool    tollPaid = false;

    int       lastMiles = 0;
    bool      oxStarved = false;   // an ox died of hunger this turn
    char      lastNews[64] = {0};
    GameEvent pendingEvent;

    void begin();
    TurnResult takeTurn();
    void chooseBranch(int edge);
    void rest(int days);

    const Node& here() const;
    const char* nextName() const;
    bool  atOregon() const;
    int   legLength() const;
    int   milesToNextStop() const;
    int   odometer() const { return totalMiles + milesIntoLeg; }
    int   milesRemaining() const;
    int   dailyMileageEstimate() const;

private:
    void advanceCalendar(int days);
    void liveTheDay(bool traveling);
    int  dailyMileage();
    void onArrive();
};

extern Sim sim;

}  // namespace game
