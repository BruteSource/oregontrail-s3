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

// 0xF0 clipped/crackled on this speaker; 0xC0 was clean. Cap a notch below the
// bad level and sit the default a little under that.
constexpr uint8_t kVolMaxReg = 0xDC;
bool    s_ready = false;
uint8_t s_vol   = 0xD4;

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
    ok &= es8311Write(0x32, s_vol);
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

}  // namespace

bool begin() {
    pinMode(AUDIO_EN, OUTPUT);
    digitalWrite(AUDIO_EN, LOW);      // enable codec + amp
    delay(10);

    // Wire is already up from touch::begin(); this is a no-op re-assert.
    Wire.begin(16, 15, 400000U);

    const bool ok = es8311Init();
    i2sSetup();
    delay(50);

    s_ready = ok;
    Serial.printf("[audio] ES8311 init %s\n", ok ? "ok" : "FAILED (no ACK)");
    return ok;
}

bool ready() { return s_ready; }

void setVolume(uint8_t reg) {
    s_vol = reg > kVolMaxReg ? kVolMaxReg : reg;
    es8311Write(0x32, s_vol);
}

void playTone(float freqHz, int durationMs) {
    if (!s_ready) return;
    const int total = (SAMPLE_RATE * durationMs) / 1000;
    constexpr int CHUNK = 256;
    int16_t buf[CHUNK * 2];
    float   phase = 0.0f;
    const float inc = 2.0f * (float)M_PI * freqHz / SAMPLE_RATE;

    int left = total;
    while (left > 0) {
        const int n = left < CHUNK ? left : CHUNK;
        for (int i = 0; i < n; ++i) {
            const int16_t s = (int16_t)(sinf(phase) * 24000.0f);  // headroom vs clip
            buf[i * 2] = s;
            buf[i * 2 + 1] = s;
            phase += inc;
            if (phase > 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
        }
        size_t written;
        i2s_write(I2S_NUM_0, buf, n * 2 * sizeof(int16_t), &written, portMAX_DELAY);
        left -= n;
    }
    // Flush a few ms of silence so the amp doesn't hold the last sample.
    int16_t z[CHUNK * 2] = {0};
    size_t w;
    i2s_write(I2S_NUM_0, z, sizeof(z), &w, portMAX_DELAY);
}

void testChime() {
    setVolume(s_vol);         // re-assert in case something clobbered reg 0x32

    // An original C-major flourish — just something with enough notes to judge
    // tone and volume. {freq Hz, ms}; freq 0 = rest.
    struct Note { float f; int ms; };
    static const Note kTune[] = {
        {392.00f, 130}, {392.00f, 130}, {523.25f, 160}, {659.25f, 220},
        {587.33f, 130}, {523.25f, 130}, {587.33f, 160}, {659.25f, 240},
        {0.0f,     60},
        {523.25f, 130}, {392.00f, 130}, {440.00f, 130}, {493.88f, 130},
        {523.25f, 380},
        {0.0f,     70},
        {659.25f, 150}, {783.99f, 150}, {659.25f, 150}, {523.25f, 150},
        {392.00f, 160}, {523.25f, 560},
    };
    for (const Note& n : kTune) {
        if (n.f <= 0.0f) delay(n.ms);
        else             playTone(n.f, n.ms);
    }
}

}  // namespace audio
