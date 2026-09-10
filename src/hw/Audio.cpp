#include "hw/Audio.h"

#include <Arduino.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <math.h>

namespace audio {

namespace {

constexpr int     AUDIO_EN   = 1;     // LOW = codec + amp enabled
constexpr int     I2S_MCK    = 4;
constexpr int     I2S_BCK    = 5;
constexpr int     I2S_LRC    = 7;
constexpr int     I2S_DOUT   = 8;
constexpr uint8_t ES8311_ADDR = 0x18;
constexpr int     SAMPLE_RATE = 16000;

// reg 0xF0 crackled on this board's speaker; 0xC0 was clean. Cap a notch below
// the bad level, and use a square-ish output amplitude with headroom.
constexpr uint8_t kVolMaxReg = 0xDC;   // just below the level that clips (0xF0)
constexpr uint8_t kVolMinReg = 0x80;   // level 1 is still clearly audible
constexpr float   kAmplitude = 27000.0f;   // sine peak, headroom to 32767

bool    s_ready  = false;
int     s_level  = 7;
bool    s_muted  = false;
uint8_t s_volReg = 0xC0;

// --- song player state (written by callers, read by the task) --------------
portMUX_TYPE      s_mux   = portMUX_INITIALIZER_UNLOCKED;
const Note*      s_song  = nullptr;
size_t           s_len   = 0;
bool             s_loop  = false;
volatile uint32_t s_gen  = 0;          // bumped to abandon the current song
volatile bool    s_click = false;      // a UI tick is pending
TaskHandle_t     s_task  = nullptr;

bool es8311Write(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ES8311_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

// Espressif esp-adf reference sequence for 16 kHz / 16-bit, MCLK = 256*fs, as
// used verbatim in the working "Debugging the S3" sketch.
bool es8311Init() {
    bool ok = true;
    ok &= es8311Write(0x00, 0x1F);
    delay(20);
    ok &= es8311Write(0x00, 0x00);
    ok &= es8311Write(0x01, 0x3F);
    ok &= es8311Write(0x02, 0x00);
    ok &= es8311Write(0x03, 0x10);
    ok &= es8311Write(0x04, 0x10);
    ok &= es8311Write(0x05, 0x00);
    ok &= es8311Write(0x06, 0x07);
    ok &= es8311Write(0x07, 0x00);
    ok &= es8311Write(0x08, 0xFF);
    ok &= es8311Write(0x09, 0x0C);
    ok &= es8311Write(0x0A, 0x0C);
    ok &= es8311Write(0x0D, 0x01);
    ok &= es8311Write(0x0E, 0x02);
    ok &= es8311Write(0x12, 0x00);
    ok &= es8311Write(0x13, 0x10);
    ok &= es8311Write(0x1C, 0x6A);
    ok &= es8311Write(0x37, 0x08);
    ok &= es8311Write(0x32, s_volReg);
    ok &= es8311Write(0x00, 0x80);
    return ok;
}

void i2sSetup() {
    i2s_config_t config = {};
    config.mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    config.sample_rate          = SAMPLE_RATE;
    config.bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT;
    config.channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT;
    config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    config.intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1;
    config.dma_buf_count        = 6;
    config.dma_buf_len          = 256;
    config.use_apll             = true;
    config.tx_desc_auto_clear   = true;
    config.fixed_mclk           = SAMPLE_RATE * 256;

    i2s_driver_install(I2S_NUM_0, &config, 0, nullptr);
    i2s_pin_config_t pins = {};
    pins.mck_io_num   = I2S_MCK;
    pins.bck_io_num   = I2S_BCK;
    pins.ws_io_num    = I2S_LRC;
    pins.data_out_num = I2S_DOUT;
    pins.data_in_num  = I2S_PIN_NO_CHANGE;
    i2s_set_pin(I2S_NUM_0, &pins);
}

void applyVolReg() {
    es8311Write(0x32, s_muted ? 0x00 : s_volReg);
}

void renderClick();   // defined below; renderNote() interleaves it

// Render one note straight to I2S. Bails between DMA chunks if `gen` is stale
// (a new song / stop landed), so a long note is cut promptly.
void renderNote(uint16_t hz, uint16_t ms, uint32_t gen) {
    const int total = (SAMPLE_RATE * ms) / 1000;
    constexpr int CHUNK = 256;
    int16_t buf[CHUNK * 2];
    static float phase = 0.0f;              // continuous across notes -> no click
    const float inc = hz ? 2.0f * (float)M_PI * hz / SAMPLE_RATE : 0.0f;

    int left = total;
    while (left > 0 && s_gen == gen) {
        if (s_click) renderClick();      // UI tick over the top of the music
        const int n = left < CHUNK ? left : CHUNK;
        for (int i = 0; i < n; ++i) {
            int16_t s = 0;
            if (hz) {
                s = (int16_t)(sinf(phase) * kAmplitude);   // sine — softer tone
                phase += inc;
                if (phase > 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
            }
            buf[i * 2] = s;
            buf[i * 2 + 1] = s;
        }
        size_t written;
        i2s_write(I2S_NUM_0, buf, n * 2 * sizeof(int16_t), &written, portMAX_DELAY);
        left -= n;
    }
}

// A short UI tick: a low-ish square with a fast decay so it reads as a "tock",
// not a beep. Only the song task calls this.
void renderClick() {
    s_click = false;
    constexpr int   N    = 420;          // ~26 ms at 16 kHz
    constexpr float FREQ = 720.0f;
    constexpr float AMP  = 15000.0f;
    int16_t buf[N * 2];
    float phase = 0.0f;
    const float inc = 2.0f * (float)M_PI * FREQ / SAMPLE_RATE;
    for (int i = 0; i < N; ++i) {
        float env = 1.0f - (float)i / N;
        env *= env;                       // steeper tail -> click, not tone
        const int16_t s = (int16_t)((sinf(phase) >= 0.0f ? AMP : -AMP) * env);
        phase += inc;
        if (phase > 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
        buf[i * 2] = s;
        buf[i * 2 + 1] = s;
    }
    size_t w;
    i2s_write(I2S_NUM_0, buf, sizeof(buf), &w, portMAX_DELAY);
}

// The task ALWAYS feeds I2S — a song, a click, or a stream of silence. The bus
// never underruns, so its DMA buffers never loop stale audio (that was the
// click stutter), and i2s_write never wedges from being left idle.
void songTask(void*) {
    constexpr int CHUNK = 256;
    static int16_t sil[CHUNK * 2] = {0};   // read-only feed; keep off the stack

    for (;;) {
        if (s_click) renderClick();

        taskENTER_CRITICAL(&s_mux);
        const Note* song = s_song;
        const size_t len = s_len;
        const bool   loop = s_loop;
        const uint32_t gen = s_gen;
        taskEXIT_CRITICAL(&s_mux);

        if (!song || !len) {
            size_t w;
            i2s_write(I2S_NUM_0, sil, sizeof(sil), &w, portMAX_DELAY);
            continue;
        }

        do {
            for (size_t i = 0; i < len && s_gen == gen; ++i)
                renderNote(song[i].hz, song[i].ms, gen);
        } while (loop && s_gen == gen);

        taskENTER_CRITICAL(&s_mux);
        if (s_gen == gen) {              // finished on its own, not interrupted
            s_song = nullptr;
            s_len = 0;
        }
        taskEXIT_CRITICAL(&s_mux);
    }
}

}  // namespace

bool begin() {
    pinMode(AUDIO_EN, OUTPUT);
    digitalWrite(AUDIO_EN, LOW);      // enable codec + amp
    delay(10);
    Wire.begin(16, 15, 400000U);      // no-op re-assert (touch already did this)

    setLevel(s_level);               // compute s_volReg from the default level
    const bool ok = es8311Init();
    i2sSetup();
    delay(50);

    s_ready = ok;
    Serial.printf("[audio] ES8311 init %s\n", ok ? "ok" : "FAILED (no ACK)");

    if (ok && !s_task)
        xTaskCreatePinnedToCore(songTask, "song", 6144, nullptr, 1, &s_task, 0);
    return ok;
}

bool ready() { return s_ready; }

void setLevel(int lv) {
    if (lv < 0) lv = 0;
    if (lv > 10) lv = 10;
    s_level = lv;
    s_volReg = lv == 0 ? 0x00
                       : (uint8_t)(kVolMinReg +
                                   (long)(kVolMaxReg - kVolMinReg) * lv / 10);
    if (s_ready) applyVolReg();
}

int level() { return s_level; }

void setMuted(bool m) {
    s_muted = m;
    if (s_ready) applyVolReg();
}

bool muted() { return s_muted; }

void playSong(const Note* notes, size_t count, bool loop) {
    if (!s_ready || !notes || !count) return;
    taskENTER_CRITICAL(&s_mux);
    s_song = notes;
    s_len = count;
    s_loop = loop;
    s_gen++;
    taskEXIT_CRITICAL(&s_mux);
}

void stopSong() {
    taskENTER_CRITICAL(&s_mux);
    s_song = nullptr;
    s_len = 0;
    s_gen++;
    taskEXIT_CRITICAL(&s_mux);
}

bool songPlaying() { return s_song != nullptr; }

void click() {
    if (s_ready) s_click = true;
}

void testChime() {
    static const Note kTune[] = {
        {392, 130}, {392, 130}, {523, 160}, {659, 220},
        {587, 130}, {523, 130}, {587, 160}, {659, 240}, {0, 60},
        {523, 130}, {392, 130}, {440, 130}, {494, 130}, {523, 380}, {0, 70},
        {659, 150}, {784, 150}, {659, 150}, {523, 150}, {392, 160}, {523, 560},
    };
    playSong(kTune, sizeof(kTune) / sizeof(kTune[0]));
}

}  // namespace audio
