// LiPo gauge on GPIO9 (ADC1_CH8). Port of gridiron-esp32s3 battery.cpp — see
// HOSYOND_ESP32S3_TARGET.md §5f. BAT_DIVIDER is calibrated per unit; trim it if
// the reading is off (watch the [bat] serial line).
#pragma once

namespace battery {

void  begin();
void  update();      // call each loop; self rate-limited to ~0.5 Hz
float volts();       // smoothed terminal voltage
int   percent();     // 0..100 from a LiPo discharge curve
int   charging();    // 1 charging, 0 full/on-charger, -1 discharging

}  // namespace battery
