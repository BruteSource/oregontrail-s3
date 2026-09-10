// FT6336 capacitive touch — raw register reads (library drivers misbehave on
// this panel; see HOSYOND_ESP32S3_TARGET.md §5c / §4.7). Port of the proven
// implementation from github.com/BruteSource/gridiron-esp32s3, adapted from
// that project's portrait 240x320 to this project's landscape 320x240
// (rotation 1): screen X is driven by the panel's native Y axis and screen Y
// by native X, with a 4-corner calibration fitting the endpoints.
#pragma once
#include <stdint.h>

#include <LovyanGFX.hpp>

namespace touch {

// Raw FT6336 value at each screen edge, in the native axis that drives that
// screen axis. `sx*` come from raw-Y, `sy*` from raw-X (see file header).
struct Cal {
    int16_t sxAtLeft, sxAtRight;   // raw-Y at screen x = 0 and x = OT_W
    int16_t syAtTop, syAtBottom;   // raw-X at screen y = 0 and y = OT_H
};

// Reset pulse, I2C @ 400 kHz, threshold bump, polling INT mode. Loads a stored
// calibration for the current rotation if one exists.
void begin();

// Print an I2C scan + FT6336 identity/status registers to Serial.
void diag();

bool isCalibrated();

// Blocking 4-corner calibration (like gridiron's display_calibrate): draws
// crosshairs into `frame`, pushes it to the panel itself, samples each corner,
// persists the fit to NVS. Returns false if the fit looks degenerate.
bool runCalibration(LGFX_Sprite& frame);

// Debounced screen-space touch. `pressed` is true for one poll on the down edge.
struct Point { int16_t x, y; bool down; bool pressed; };
Point poll();

// Raw panel-frame sample, no calibration. false if no finger / garbage sample.
bool rawSample(int16_t* rx, int16_t* ry);

// "Finger down right now?" — cheap, for spin-wait loops.
bool isTouched();

// Re-init the I2C bus after a light-standby (target ref §14).
void busResume();

}  // namespace touch
