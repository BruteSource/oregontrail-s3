// Preset party-member names. The first list is the C# clone's random pool
// (InputPlayerNames.GetPlayerName); a few extra period-appropriate names are
// added so the on-screen picker has a fuller grid.
#pragma once

namespace game {

inline const char* const* presetNames(int* count) {
    static const char* const k[] = {
        "Bob",     "Joe",      "Sally",    "Tim",      "Steve",
        "Zeke",    "Suzan",    "Rebekah",  "Young",    "Iris",
        "Kristy",  "Joanna",   "Angela",   "Buck",     "Anderson",
        "Siobhan", "Karey",    "Jolie",    "Carlene",  "Lekisha",
        "Amos",    "Clara",    "Hank",     "Nettie",   "Silas",
        "Etta",    "Josiah",   "Millie",   "Gus",      "Prudence",
    };
    *count = sizeof(k) / sizeof(k[0]);
    return k;
}

}  // namespace game
