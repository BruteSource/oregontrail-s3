// ES8311 mono codec + amp on the shared touch I2C bus (addr 0x18), fed by I2S.
// Enable pin GPIO1 = LOW (target ref §5g / §12). Register init and the I2S
// config are the working values from the "Debugging the S3" Axel-F sketch.
//
// Audio is otherwise parked for this project — this is just enough to prove the
// speaker is wired and to hang landmark jingles off later.
#pragma once
#include <stdint.h>

namespace audio {

// Enable the codec/amp, run the ES8311 register init, install the I2S driver.
// Safe to call after touch::begin() (shares Wire). Returns false if the ES8311
// did not ACK on I2C.
bool begin();

bool ready();

// 0x00 (silent) .. 0xFF (max). Persisted value lives in prefs::volume as 0..10.
void setVolume(uint8_t reg);

// Blocking sine tone. freqHz 50..8000, durationMs up to a few seconds.
void playTone(float freqHz, int durationMs);

// A short three-note rising arpeggio — the "is the speaker alive?" check.
void testChime();

}  // namespace audio
