// Persistence: the in-progress journey (resume after power loss) and the
// high-score table. Both are fixed-size binary blobs in LittleFS (hw/Storage).
// Device-only — not part of the pure-C++ game/ tree.
#pragma once
#include <stdint.h>

namespace savegame {

// --- in-progress journey ------------------------------------------------
bool exists();          // a resumable save is on disk
bool load();            // restore game::g + game::sim from disk; false if none/bad
void save();            // snapshot the current game (call at each landmark)
void clear();           // wipe it (call on game over)

// --- high scores -------------------------------------------------------
constexpr int kTopN = 5;

struct HighScore {
    char    name[14];
    int32_t score;
    uint16_t days;
    uint8_t  arrived;   // 1 = reached Oregon, 0 = perished
    uint8_t  _pad;
};

int  loadHighScores(HighScore out[kTopN]);   // returns count, sorted desc
void eraseHighScores();                       // back to the seeded defaults
bool qualifies(int32_t score);               // would this make the table?
int  submitHighScore(const char* name, int32_t score, uint16_t days, bool arrived);
// ^ inserts, saves, returns the 0-based rank (or -1 if it didn't place)

}  // namespace savegame
