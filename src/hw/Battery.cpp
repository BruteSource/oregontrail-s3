#include "hw/Battery.h"

#include <Arduino.h>
#include <Preferences.h>

namespace battery {

namespace {
constexpr int   BAT_ADC_PIN     = 9;
constexpr float BAT_DIVIDER_DEF  = 1.955f;   // nominal; trimmed by calibrate()
constexpr float BAT_OFFSET       = 0.0f;
constexpr char  NVS_NS[]         = "otrail_bat";

float    s_divider = BAT_DIVIDER_DEF;
float    s_v = 3.9f;
float    s_hist[10];
int      s_hi = 0;
bool     s_histFull = false;
uint32_t s_lastRead = 0, s_lastSample = 0;

float adcMillivolts() {
    uint32_t sum = 0;
    for (int i = 0; i < 24; i++) sum += analogReadMilliVolts(BAT_ADC_PIN);
    return sum / 24.0f;
}

float readRaw() {
    return (adcMillivolts() / 1000.0f) * s_divider + BAT_OFFSET;
}
}  // namespace

void begin() {
    analogSetPinAttenuation(BAT_ADC_PIN, ADC_11db);

    Preferences p;
    if (p.begin(NVS_NS, true)) {
        const float d = p.getFloat("div", 0.0f);
        if (d > 1.0f && d < 4.0f) s_divider = d;
        p.end();
    }

    s_v = readRaw();
    for (float& h : s_hist) h = s_v;
}

void update() {
    const uint32_t now = millis();
    if (now - s_lastRead < 2000) return;
    s_lastRead = now;

    const float r = readRaw();
    if (r > 2.0f && r < 5.0f) s_v += (r - s_v) * 0.25f;

    if (now - s_lastSample > 60000) {
        s_lastSample = now;
        s_hist[s_hi] = s_v;
        s_hi = (s_hi + 1) % 10;
        if (s_hi == 0) s_histFull = true;
    }
}

float volts()    { return s_v; }
float rawVolts() { return readRaw(); }
float divider()  { return s_divider; }

float calibrate(float trueVolts) {
    if (trueVolts < 2.5f || trueVolts > 4.5f) return 0.0f;
    const float mv = adcMillivolts();
    if (mv < 100.0f) return 0.0f;
    s_divider = (trueVolts - BAT_OFFSET) / (mv / 1000.0f);

    Preferences p;
    if (p.begin(NVS_NS, false)) {
        p.putFloat("div", s_divider);
        p.end();
    }

    s_v = readRaw();
    for (float& h : s_hist) h = s_v;
    s_histFull = false;
    s_hi = 0;
    return s_divider;
}

int charging() {
    if (s_v >= 4.17f) return 0;
    if (s_v >= 4.05f) return 1;
    const float oldest = s_hist[s_hi];
    if ((s_histFull || s_hi > 3) && s_v - oldest > 0.012f) return 1;
    return -1;
}

int percent() {
    const float v = s_v;
    static const float lut[][2] = {
        {4.20f, 100}, {4.10f, 92}, {4.00f, 82}, {3.90f, 68}, {3.83f, 55},
        {3.78f, 45},  {3.73f, 35}, {3.68f, 24}, {3.62f, 15}, {3.52f, 7},
        {3.40f, 2},   {3.20f, 0},
    };
    const int n = sizeof(lut) / sizeof(lut[0]);
    if (v >= lut[0][0]) return 100;
    for (int i = 1; i < n; i++) {
        if (v >= lut[i][0]) {
            const float f = (v - lut[i][0]) / (lut[i - 1][0] - lut[i][0]);
            return (int)(lut[i][1] + f * (lut[i - 1][1] - lut[i][1]) + 0.5f);
        }
    }
    return 0;
}

}  // namespace battery
