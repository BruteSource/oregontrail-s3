#include "hw/Touch.h"

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include <LovyanGFX.hpp>

#include "hw/Display.h"   // OT_ROTATION, OT_W, OT_H
#include "ui/Theme.h"

namespace touch {

namespace {

constexpr uint8_t FT_ADDR = 0x38;
constexpr int     PIN_SDA = 16;
constexpr int     PIN_SCL = 15;
constexpr int     PIN_RST = 18;
constexpr int     PIN_INT = 17;

constexpr char NVS_NS[] = "otrail_touch";

// A sane default so an uncalibrated build is still usable. Landscape rot 1:
// screen X spans native-Y ~16..306, screen Y spans native-X ~20..230.
Cal  s_cal = {16, 306, 20, 230};
bool s_calValid = false;

// tap/drag detection state
bool     s_down = false;
int16_t  s_x0 = 0, s_y0 = 0;
int      s_maxMove = 0;
uint32_t s_downMs = 0;

// Raw register read with the 0x0FFF garbage guard (gridiron ftRaw, verbatim).
bool ftRaw(int16_t* rx, int16_t* ry) {
    uint8_t d[7];
    Wire.beginTransmission(FT_ADDR);
    Wire.write(0x02);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((uint8_t)FT_ADDR, (uint8_t)7) != 7) return false;
    for (int i = 0; i < 7; i++) d[i] = Wire.read();

    if ((d[0] & 0x0F) == 0) return false;                     // no finger
    int x = ((uint16_t)(d[1] & 0x0F) << 8) | d[2];
    int y = ((uint16_t)(d[3] & 0x0F) << 8) | d[4];
    if (x > 800 || y > 800) return false;                     // garbage sample
    *rx = (int16_t)x;
    *ry = (int16_t)y;
    return true;
}

// Raw -> landscape screen coords. Native Y drives screen X, native X drives
// screen Y (rotation 1). Edge nudge (gridiron): near an edge the contact patch
// pulls toward centre, so edge targets get missed — push those taps back out.
bool readPoint(int16_t* sx, int16_t* sy) {
    int16_t rx, ry;
    if (!ftRaw(&rx, &ry)) return false;

    long x = map(ry, s_cal.sxAtLeft, s_cal.sxAtRight, 0, OT_W);
    long y = map(rx, s_cal.syAtTop, s_cal.syAtBottom, 0, OT_H);

    // Outward nudge near an edge: the contact patch pulls the reported point
    // toward centre so edge targets get missed. (gridiron readPoint, scaled.)
    if (y < 40)             y -= (40 - y) / 4;
    else if (y > OT_H - 41) y += (y - (OT_H - 41)) / 4;
    if (x < 40)             x -= (40 - x) / 4;
    else if (x > OT_W - 41) x += (x - (OT_W - 41)) / 4;

    *sx = (int16_t)constrain(x, 0, OT_W - 1);
    *sy = (int16_t)constrain(y, 0, OT_H - 1);
    return true;
}

uint8_t readReg(uint8_t reg) {
    Wire.beginTransmission(FT_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)FT_ADDR, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0xFF;
}

}  // namespace

void begin() {
    pinMode(PIN_RST, OUTPUT);
    digitalWrite(PIN_RST, LOW);
    delay(10);
    digitalWrite(PIN_RST, HIGH);
    delay(300);

    Wire.begin(PIN_SDA, PIN_SCL, 400000U);
    pinMode(PIN_INT, INPUT_PULLUP);

    // Raise the touch threshold a little to cut phantom taps.
    Wire.beginTransmission(FT_ADDR);
    Wire.write(0x80);
    Wire.write(40);
    Wire.endTransmission();

    // G_MODE = polling: INT pulses low repeatedly while touched.
    Wire.beginTransmission(FT_ADDR);
    Wire.write(0xA4);
    Wire.write(0x00);
    Wire.endTransmission();

    Preferences p;
    if (p.begin(NVS_NS, true)) {
        if (p.isKey("sxl") && p.getInt("rot", -1) == OT_ROTATION) {
            s_cal.sxAtLeft   = p.getInt("sxl", s_cal.sxAtLeft);
            s_cal.sxAtRight  = p.getInt("sxr", s_cal.sxAtRight);
            s_cal.syAtTop    = p.getInt("syt", s_cal.syAtTop);
            s_cal.syAtBottom = p.getInt("syb", s_cal.syAtBottom);
            s_calValid = true;
        }
        p.end();
    }
}

void diag() {
    Serial.println("[touch] I2C scan:");
    int found = 0;
    for (uint8_t a = 1; a < 127; ++a) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[touch]   device at 0x%02X\n", a);
            ++found;
        }
    }
    if (!found) Serial.println("[touch]   (none — bus/wiring problem)");
    Serial.printf("[touch] INT=%d  chipid(0xA3)=0x%02X vendor(0xA8)=0x%02X "
                  "gmode(0xA4)=0x%02X  calValid=%d\n",
                  digitalRead(PIN_INT), readReg(0xA3), readReg(0xA8),
                  readReg(0xA4), s_calValid);
}

bool isCalibrated() { return s_calValid; }

bool rawSample(int16_t* rx, int16_t* ry) { return ftRaw(rx, ry); }

bool isTouched() {
    int16_t x, y;
    return ftRaw(&x, &y);
}

void busResume() { Wire.begin(PIN_SDA, PIN_SCL, 400000U); }

Point poll() {
    Point e{s_x0, s_y0, false, false};
    int16_t x, y;
    const bool touching = readPoint(&x, &y);
    const uint32_t now = millis();

    if (touching) {
        e.down = true;
        e.x = x;
        e.y = y;
        if (!s_down) {
            s_down = true;
            s_x0 = x;
            s_y0 = y;
            s_maxMove = 0;
            s_downMs = now;
        } else {
            int dist = abs(x - s_x0) + abs(y - s_y0);
            if (dist > s_maxMove) s_maxMove = dist;
        }
    } else if (s_down) {
        s_down = false;
        if (s_maxMove < 24 && now - s_downMs < 700) {
            e.pressed = true;     // "tap": lifted after a short, stationary press
            e.x = s_x0;
            e.y = s_y0;
        }
    }
    return e;
}

// --- blocking 4-corner calibration (port of gridiron display_calibrate) ----

namespace {

void calTarget(LGFX_Sprite& g, int tx, int ty, const char* msg) {
    g.fillScreen(theme::BG);
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::middle_center);
    g.setTextColor(theme::INK);
    g.drawString("Touch calibration", OT_W / 2, OT_H / 2 - 14);
    g.setTextColor(theme::INK_DIM);
    g.drawString(msg, OT_W / 2, OT_H / 2 + 8);
    g.drawLine(tx - 12, ty, tx + 12, ty, theme::ACCENT);
    g.drawLine(tx, ty - 12, tx, ty + 12, theme::ACCENT);
    g.drawCircle(tx, ty, 7, theme::ACCENT);
    g.pushSprite(0, 0);
}

}  // namespace

bool runCalibration(LGFX_Sprite& g) {
    // ~9% insets (the values used for the calibration that passed the Phase 1
    // touch test); matches gridiron's proven portrait proportions.
    constexpr int INSET_X = 28, INSET_Y = 26;
    const int tx[4] = {INSET_X, OT_W - INSET_X, OT_W - INSET_X, INSET_X};
    const int ty[4] = {INSET_Y, INSET_Y, OT_H - INSET_Y, OT_H - INSET_Y};
    int16_t rx[4], ry[4];
    int16_t dx, dy;

    for (int i = 0; i < 4; i++) {
        char m[24];
        snprintf(m, sizeof(m), "tap target %d of 4", i + 1);
        calTarget(g, tx[i], ty[i], m);

        while (ftRaw(&dx, &dy)) delay(20);          // wait for release
        delay(150);
        while (!ftRaw(&dx, &dy)) delay(10);         // wait for the tap

        long sx = 0, sy = 0;
        int nn = 0;
        uint32_t t0 = millis();
        while (ftRaw(&dx, &dy) && millis() - t0 < 2500) {
            sx += dx;
            sy += dy;
            nn++;
            delay(10);
        }
        rx[i] = nn ? (int16_t)(sx / nn) : dx;
        ry[i] = nn ? (int16_t)(sy / nn) : dy;

        g.fillCircle(tx[i], ty[i], 7, theme::INK);
        g.pushSprite(0, 0);
        delay(250);
    }

    // Screen X is driven by raw Y: average raw-Y at the left pair and the right
    // pair of targets, then extrapolate to screen x = 0 and x = OT_W.
    const float yL = (ry[0] + ry[3]) / 2.0f;
    const float yR = (ry[1] + ry[2]) / 2.0f;
    const float yPerPx = (yR - yL) / (float)(OT_W - 2 * INSET_X);

    // Screen Y is driven by raw X: top pair vs bottom pair.
    const float xT = (rx[0] + rx[1]) / 2.0f;
    const float xB = (rx[2] + rx[3]) / 2.0f;
    const float xPerPx = (xB - xT) / (float)(OT_H - 2 * INSET_Y);

    Cal c;
    c.sxAtLeft   = (int16_t)lroundf(yL - yPerPx * INSET_X);
    c.sxAtRight  = (int16_t)lroundf(yL + yPerPx * (OT_W - INSET_X));
    c.syAtTop    = (int16_t)lroundf(xT - xPerPx * INSET_Y);
    c.syAtBottom = (int16_t)lroundf(xT + xPerPx * (OT_H - INSET_Y));

    const bool ok = abs(c.sxAtRight - c.sxAtLeft) > 40 &&
                    abs(c.syAtBottom - c.syAtTop) > 40;

    Serial.printf("[cal] corners raw XY: (%d,%d)(%d,%d)(%d,%d)(%d,%d)\n",
                  rx[0], ry[0], rx[1], ry[1], rx[2], ry[2], rx[3], ry[3]);
    Serial.printf("[cal] fit sx %d..%d  sy %d..%d  ok=%d\n",
                  c.sxAtLeft, c.sxAtRight, c.syAtTop, c.syAtBottom, ok);

    if (ok) {
        s_cal = c;
        s_calValid = true;
        Preferences p;
        if (p.begin(NVS_NS, false)) {
            p.putInt("sxl", c.sxAtLeft);
            p.putInt("sxr", c.sxAtRight);
            p.putInt("syt", c.syAtTop);
            p.putInt("syb", c.syAtBottom);
            p.putInt("rot", OT_ROTATION);
            p.end();
        }
    }

    g.fillScreen(theme::BG);
    g.setFont(&fonts::Font4);
    g.setTextDatum(textdatum_t::middle_center);
    g.setTextColor(ok ? theme::ACCENT : theme::WARN);
    g.drawString(ok ? "calibrated" : "calibration failed", OT_W / 2, OT_H / 2);
    g.pushSprite(0, 0);
    delay(900);
    return ok;
}

}  // namespace touch
