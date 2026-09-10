// ES8311 mono codec + amp on the shared touch I2C bus (addr 0x18), fed by I2S.
// Enable pin GPIO1 = LOW (target ref §5g / §12). Register init and the I2S
// config are the working values from the "Debugging the S3" Axel-F sketch.
//
// Playback runs on its own FreeRTOS task: playSong() just hands over a note
// list, stopSong() / a new playSong() cut the current one off between notes.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace audio {

// One monophonic note. hz == 0 is a rest. Matches the {hz, ms} pairs decoded in
// the OregonTrail repo's music/*.json (see tools/gen_assets.py -> music.{h,cpp}).
struct Note {
    uint16_t hz;
    uint16_t ms;
};

// Enable the codec/amp, run the ES8311 register init, install I2S, start the
// playback task. Safe after touch::begin() (shares Wire). false = no I2C ACK.
bool begin();
bool ready();

// Volume 0..10 (0 = silent). Maps to the ES8311 DAC register, clamped to a
// per-speaker ceiling below the level that clips on this board.
void setLevel(int level);
int  level();

void setMuted(bool m);
bool muted();

// Hand the player a note list. Returns immediately; the task plays it. A second
// call replaces whatever is playing. `notes` must outlive playback (flash data).
void playSong(const Note* notes, size_t count, bool loop = false);
void stopSong();
bool songPlaying();

// A short UI tick, played over the top of any music. Cheap to call on every tap.
void click();

// Short built-in flourish — the "is the speaker alive?" check (dev serial 'B').
void testChime();

}  // namespace audio
