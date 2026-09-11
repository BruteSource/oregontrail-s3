// LiPo gauge on GPIO9 (ADC1_CH8). Port of gridiron-esp32s3 battery.cpp — see
// HOSYOND_ESP32S3_TARGET.md §5f. The resistor divider varies a little per unit
// and per battery; calibrate() trims it against a meter reading and stores the
// factor in NVS. Dev serial: 'V' prints the raw reading, 'V<mV>' calibrates.
#pragma once

namespace battery {

void  begin();
void  update();      // call each loop; self rate-limited to ~0.5 Hz
float volts();       // smoothed terminal voltage
int   percent();     // 0..100 from a LiPo discharge curve
int   charging();    // 1 charging, 0 full/on-charger, -1 discharging

float rawVolts();    // unsmoothed, this instant (for the calibration readout)
float divider();     // current divider factor

// Tell the gauge the true terminal voltage right now (from a multimeter); it
// back-solves the divider, applies it, and saves it to NVS. Returns the new
// divider, or 0 on a nonsense input.
float calibrate(float trueVolts);

}  // namespace battery
