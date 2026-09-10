#include "Settings.h"

#include <Preferences.h>

namespace prefs {

int brightness = 180;
int volume = 0;

namespace {
constexpr char NS[] = "otrail_prefs";
int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
}  // namespace

void load() {
    Preferences p;
    if (p.begin(NS, true)) {
        brightness = clampi(p.getInt("bright", brightness), kBrightMin, kBrightMax);
        volume = clampi(p.getInt("vol", volume), 0, kVolMax);
        p.end();
    }
}

void save() {
    Preferences p;
    if (p.begin(NS, false)) {
        p.putInt("bright", brightness);
        p.putInt("vol", volume);
        p.end();
    }
}

}  // namespace prefs
