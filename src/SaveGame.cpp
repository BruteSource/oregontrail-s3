#include "SaveGame.h"

#include <string.h>

#include "game/Session.h"
#include "game/Sim.h"
#include "hw/Storage.h"

namespace savegame {

namespace {
constexpr char     SAVE_PATH[] = "/save.dat";
constexpr char     HS_PATH[]   = "/scores.dat";
constexpr uint32_t SAVE_MAGIC  = 0x0770'5A11;   // "OTrail save"
constexpr uint16_t SAVE_VER    = 3;             // bump when game::Sim / Party layout changes
constexpr uint32_t HS_MAGIC    = 0x0770'11C5;

#pragma pack(push, 1)
struct SaveBlob {
    uint32_t      magic;
    uint16_t      version;
    game::Party   party;
    game::Vehicle vehicle;
    game::Month   startMonth;
    game::Sim     sim;
};
struct HsBlob {
    uint32_t  magic;
    HighScore rows[kTopN];
};
#pragma pack(pop)
}  // namespace

bool exists() {
    SaveBlob b;
    return storage::loadBlob(SAVE_PATH, &b, sizeof(b)) &&
           b.magic == SAVE_MAGIC && b.version == SAVE_VER;
}

bool load() {
    SaveBlob b;
    if (!storage::loadBlob(SAVE_PATH, &b, sizeof(b))) return false;
    if (b.magic != SAVE_MAGIC || b.version != SAVE_VER) return false;
    game::g.party = b.party;
    game::g.vehicle = b.vehicle;
    game::g.startMonth = b.startMonth;
    game::sim = b.sim;
    return true;
}

void save() {
    SaveBlob b{};
    b.magic = SAVE_MAGIC;
    b.version = SAVE_VER;
    b.party = game::g.party;
    b.vehicle = game::g.vehicle;
    b.startMonth = game::g.startMonth;
    b.sim = game::sim;
    storage::saveBlob(SAVE_PATH, &b, sizeof(b));
}

void clear() { storage::remove(SAVE_PATH); }

// --- high scores -------------------------------------------------------

static void readHs(HsBlob& hs) {
    if (!storage::loadBlob(HS_PATH, &hs, sizeof(hs)) || hs.magic != HS_MAGIC) {
        memset(&hs, 0, sizeof(hs));
        hs.magic = HS_MAGIC;
        // seed a couple of names to beat, like the original's default top ten
        strncpy(hs.rows[0].name, "Stephen Meek", 13);
        hs.rows[0].score = 7650; hs.rows[0].days = 160; hs.rows[0].arrived = 1;
        strncpy(hs.rows[1].name, "Ezra Meeker", 13);
        hs.rows[1].score = 5250; hs.rows[1].days = 172; hs.rows[1].arrived = 1;
        strncpy(hs.rows[2].name, "William Sublette", 13);
        hs.rows[2].score = 2000; hs.rows[2].days = 189; hs.rows[2].arrived = 1;
    }
}

int loadHighScores(HighScore out[kTopN]) {
    HsBlob hs;
    readHs(hs);
    int n = 0;
    for (int i = 0; i < kTopN; ++i) {
        if (hs.rows[i].score <= 0 && hs.rows[i].name[0] == 0) continue;
        out[n++] = hs.rows[i];
    }
    return n;
}

void eraseHighScores() { storage::remove(HS_PATH); }

bool qualifies(int32_t score) {
    HsBlob hs;
    readHs(hs);
    return score > hs.rows[kTopN - 1].score;
}

int submitHighScore(const char* name, int32_t score, uint16_t days, bool arrived) {
    HsBlob hs;
    readHs(hs);

    int at = kTopN;
    for (int i = 0; i < kTopN; ++i)
        if (score > hs.rows[i].score) { at = i; break; }
    if (at >= kTopN) return -1;

    for (int i = kTopN - 1; i > at; --i) hs.rows[i] = hs.rows[i - 1];
    HighScore& r = hs.rows[at];
    memset(&r, 0, sizeof(r));
    strncpy(r.name, name && name[0] ? name : "Traveller", 13);
    r.score = score;
    r.days = days;
    r.arrived = arrived ? 1 : 0;

    storage::saveBlob(HS_PATH, &hs, sizeof(hs));
    return at;
}

}  // namespace savegame
