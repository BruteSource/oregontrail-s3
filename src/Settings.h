// Small persisted preferences: screen brightness and (placeholder) volume.
// Stored in NVS namespace "otrail_prefs". Device-only.
#pragma once

namespace prefs {

constexpr int kBrightMin = 30, kBrightMax = 255, kBrightStep = 25;
constexpr int kVolMax = 10;

extern int brightness;   // 30..255
extern int volume;       // 0..10 — not wired to audio yet

void load();
void save();

}  // namespace prefs
