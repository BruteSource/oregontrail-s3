// Trailside advice, in the spirit of AdviceRegistry.cs — half sound, half not.
#pragma once
#include "game/Session.h"

namespace game {

inline const char* randomAdvice() {
    static const char* k[] = {
        "\"Keep your oxen fed. A starving team is a dead team, and then you "
        "walk.\"",
        "\"Don't ford a river you can't see the bottom of. Wait a day, or pay "
        "the ferry.\"",
        "\"Leave early in the year. Snow in the passes has killed more parties "
        "than any fever.\"",
        "\"Buy more food than you think you need. You can always eat lighter; "
        "you can't eat air.\"",
        "\"Rest before the mountains, not in them.\"",
        "\"A grueling pace looks fast until somebody takes sick. Then it's "
        "slower than steady ever was.\"",
        "\"Spare parts weigh nothing next to a broken axle a hundred miles from "
        "a fort.\"",
        "\"Warm clothes are worth their weight. The cold gets in through what "
        "you didn't bring.\"",
        "\"If the water tastes wrong, it is wrong. Don't drink it.\"",
        "\"The Barlow road costs money. The Columbia costs wagons. Choose with "
        "your eyes open.\"",
    };
    return k[rngRange(0, (int)(sizeof(k) / sizeof(k[0])))];
}

}  // namespace game
