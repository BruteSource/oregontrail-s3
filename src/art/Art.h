// Drawing helpers for the embedded MECC artwork. LovyanGFX decodes the PNGs
// from flash; sprite alpha is honoured so the wagon/hunter/animal cutouts drop
// straight onto a scene.
#pragma once
#include <stdint.h>

#include <LovyanGFX.hpp>

#include "art/gen/animals.h"
#include "art/gen/events.h"
#include "art/gen/hunter.h"
#include "art/gen/landmarks.h"
#include "art/gen/misc.h"
#include "art/gen/raft.h"
#include "art/gen/scenery.h"
#include "art/gen/terrain.h"
#include "art/gen/wagon.h"

namespace art {

// Landmark painting for a trail node (game/Trail.h index). 0..16 map straight to
// p0..p16; the Barlow/Columbia nodes reuse The Dalles; Oregon City is p17.
inline int landmarkIndexForNode(int node) {
    if (node <= 16) return node;
    if (node == 19) return 17;   // Oregon City -> Willamette Valley
    return 16;                    // Barlow / Columbia -> The Dalles
}

inline const uint8_t* landmarkPng(int node, size_t* len) {
    static const uint8_t* const imgs[18] = {
        lm_0, lm_1, lm_2, lm_3, lm_4, lm_5, lm_6, lm_7, lm_8,
        lm_9, lm_10, lm_11, lm_12, lm_13, lm_14, lm_15, lm_16, lm_17};
    static const size_t lens[18] = {
        lm_0_len, lm_1_len, lm_2_len, lm_3_len, lm_4_len, lm_5_len, lm_6_len,
        lm_7_len, lm_8_len, lm_9_len, lm_10_len, lm_11_len, lm_12_len, lm_13_len,
        lm_14_len, lm_15_len, lm_16_len, lm_17_len};
    const int i = landmarkIndexForNode(node);
    *len = lens[i];
    return imgs[i];
}

// Draw the full-width landmark painting, top at `y`, clipped to `h` px tall
// (paintings are ~160 px). Returns y + h.
inline int16_t drawLandmark(LGFX_Sprite& g, int node, int16_t y, int16_t h = 164) {
    size_t len;
    const uint8_t* png = landmarkPng(node, &len);
    g.drawPng(png, len, 0, y, 320, h);
    return y + h;
}

// A wagon frame: 0-2 rolling, 3 tipped, 4 on fire. ~78x31, transparent.
inline void drawWagon(LGFX_Sprite& g, int frame, int16_t x, int16_t y,
                      float scale = 1.0f) {
    static const uint8_t* const f[5] = {wagon_1, wagon_2, wagon_3, wagon_4, wagon_5};
    static const size_t l[5] = {wagon_1_len, wagon_2_len, wagon_3_len,
                                wagon_4_len, wagon_5_len};
    const int i = frame < 0 ? 0 : frame > 4 ? 4 : frame;
    if (scale == 1.0f) g.drawPng(f[i], l[i], x, y);
    else g.drawPng(f[i], l[i], x, y, 0, 0, 0, 0, scale, scale);
}

// A hunter pose (0..23). The eight aim directions live in here plus walk frames.
// `mirror` flips him to face right (the stored poses all face left).
inline void drawHunter(LGFX_Sprite& g, int pose, int16_t x, int16_t y,
                       float scale = 1.0f, bool mirror = false) {
    static const uint8_t* const f[24] = {
        hunter_1, hunter_2, hunter_3, hunter_4, hunter_5, hunter_6, hunter_7,
        hunter_8, hunter_9, hunter_10, hunter_11, hunter_12, hunter_13, hunter_14,
        hunter_15, hunter_16, hunter_17, hunter_18, hunter_19, hunter_20,
        hunter_21, hunter_22, hunter_23, hunter_24};
    static const size_t l[24] = {
        hunter_1_len, hunter_2_len, hunter_3_len, hunter_4_len, hunter_5_len,
        hunter_6_len, hunter_7_len, hunter_8_len, hunter_9_len, hunter_10_len,
        hunter_11_len, hunter_12_len, hunter_13_len, hunter_14_len, hunter_15_len,
        hunter_16_len, hunter_17_len, hunter_18_len, hunter_19_len, hunter_20_len,
        hunter_21_len, hunter_22_len, hunter_23_len, hunter_24_len};
    (void)mirror;   // negative-scale drawPng smears on this build — left-facing only
    const int i = ((pose % 24) + 24) % 24;
    if (scale == 1.0f) g.drawPng(f[i], l[i], x, y);
    else g.drawPng(f[i], l[i], x, y, 0, 0, 0, 0, scale, scale);
}

// Animal frame (0..47). Sheet holds bison / bear / deer / rabbit walk cycles.
inline void drawAnimal(LGFX_Sprite& g, int frame, int16_t x, int16_t y) {
    static const uint8_t* const f[48] = {
        animal_1, animal_2, animal_3, animal_4, animal_5, animal_6, animal_7,
        animal_8, animal_9, animal_10, animal_11, animal_12, animal_13, animal_14,
        animal_15, animal_16, animal_17, animal_18, animal_19, animal_20,
        animal_21, animal_22, animal_23, animal_24, animal_25, animal_26,
        animal_27, animal_28, animal_29, animal_30, animal_31, animal_32,
        animal_33, animal_34, animal_35, animal_36, animal_37, animal_38,
        animal_39, animal_40, animal_41, animal_42, animal_43, animal_44,
        animal_45, animal_46, animal_47, animal_48};
    static const size_t l[48] = {
        animal_1_len, animal_2_len, animal_3_len, animal_4_len, animal_5_len,
        animal_6_len, animal_7_len, animal_8_len, animal_9_len, animal_10_len,
        animal_11_len, animal_12_len, animal_13_len, animal_14_len, animal_15_len,
        animal_16_len, animal_17_len, animal_18_len, animal_19_len, animal_20_len,
        animal_21_len, animal_22_len, animal_23_len, animal_24_len, animal_25_len,
        animal_26_len, animal_27_len, animal_28_len, animal_29_len, animal_30_len,
        animal_31_len, animal_32_len, animal_33_len, animal_34_len, animal_35_len,
        animal_36_len, animal_37_len, animal_38_len, animal_39_len, animal_40_len,
        animal_41_len, animal_42_len, animal_43_len, animal_44_len, animal_45_len,
        animal_46_len, animal_47_len, animal_48_len};
    const int i = ((frame % 48) + 48) % 48;
    g.drawPng(f[i], l[i], x, y);
}

// --- terrain / scenery objects (small, transparent) ---------------------
inline void drawTerrain(LGFX_Sprite& g, int i, int16_t x, int16_t y) {
    static const uint8_t* const f[14] = {
        terr_1, terr_2, terr_3, terr_4, terr_5, terr_6, terr_7,
        terr_8, terr_9, terr_10, terr_11, terr_12, terr_13, terr_14};
    static const size_t l[14] = {
        terr_1_len, terr_2_len, terr_3_len, terr_4_len, terr_5_len, terr_6_len,
        terr_7_len, terr_8_len, terr_9_len, terr_10_len, terr_11_len, terr_12_len,
        terr_13_len, terr_14_len};
    i = ((i % 14) + 14) % 14;
    g.drawPng(f[i], l[i], x, y);
}

inline void drawScenery(LGFX_Sprite& g, int i, int16_t x, int16_t y) {
    static const uint8_t* const f[17] = {
        scen_1, scen_2, scen_3, scen_4, scen_5, scen_6, scen_7, scen_8, scen_9,
        scen_10, scen_11, scen_12, scen_13, scen_14, scen_15, scen_16, scen_17};
    static const size_t l[17] = {
        scen_1_len, scen_2_len, scen_3_len, scen_4_len, scen_5_len, scen_6_len,
        scen_7_len, scen_8_len, scen_9_len, scen_10_len, scen_11_len, scen_12_len,
        scen_13_len, scen_14_len, scen_15_len, scen_16_len, scen_17_len};
    i = ((i % 17) + 17) % 17;
    g.drawPng(f[i], l[i], x, y);
}

// Event sprite (1..7): berries, fog, snow, rain, storm, walker, trader.
inline void drawEvent(LGFX_Sprite& g, int n, int16_t x, int16_t y, float scale = 1.0f) {
    static const uint8_t* const f[7] = {evt_1, evt_2, evt_3, evt_4, evt_5, evt_6, evt_7};
    static const size_t l[7] = {evt_1_len, evt_2_len, evt_3_len, evt_4_len,
                                evt_5_len, evt_6_len, evt_7_len};
    n = n < 1 ? 1 : n > 7 ? 7 : n;
    if (scale == 1.0f) g.drawPng(f[n - 1], l[n - 1], x, y);
    else g.drawPng(f[n - 1], l[n - 1], x, y, 0, 0, 0, 0, scale, scale);
}

// MECC scene palette
namespace col {
constexpr uint16_t sky    = 0x6D3F;   // daytime blue
constexpr uint16_t ground = 0x5D66;   // prairie green
constexpr uint16_t dirt   = 0x9B85;   // trail brown
}  // namespace col

}  // namespace art
