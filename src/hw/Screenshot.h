// Dev-only: dump the current frame sprite over USB-CDC so the host can see the
// panel. Triggered by sending 'S'. Host decoder: tools/screenshot.py / mon.py.
//
// Wire format: "FRAMEBUF <w> <h> RGB565BE b64\n", base64 of the raw RGB565
// buffer (LovyanGFX stores it big-endian), "\nENDFRAME <bytes>\n".
//
// This blocks loop() for the length of the transfer, so it MUST NOT be fired on
// a timer during interactive use — a freeze here reads as dropped taps. Written
// in big USB-sized chunks so the transfer is as short as possible.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

namespace screenshot {

inline void dump(LGFX_Sprite& frame) {
    const int w = frame.width();
    const int h = frame.height();
    const auto* buf = static_cast<const uint8_t*>(frame.getBuffer());
    const size_t len = static_cast<size_t>(w) * h * 2;
    if (buf == nullptr || len == 0) {
        Serial.println("FRAMEBUF-ERR no buffer");
        return;
    }

    Serial.printf("FRAMEBUF %d %d RGB565BE b64\n", w, h);

    static const char B64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // 1020 = multiple of 4, so each flushed chunk is whole base64 quanta; the
    // host rejoins chunks anyway. Newline per chunk keeps host parsing line-based.
    char out[1021];
    size_t o = 0;
    uint32_t acc = 0;
    int bits = 0;

    for (size_t i = 0; i < len; ++i) {
        acc = (acc << 8) | buf[i];
        bits += 8;
        while (bits >= 6) {
            bits -= 6;
            out[o++] = B64[(acc >> bits) & 0x3F];
            if (o == 1020) {
                out[o++] = '\n';
                Serial.write(reinterpret_cast<uint8_t*>(out), o);
                o = 0;
            }
        }
    }
    if (bits > 0) out[o++] = B64[(acc << (6 - bits)) & 0x3F];
    while (o % 4 != 0) out[o++] = '=';
    if (o) Serial.write(reinterpret_cast<uint8_t*>(out), o);

    Serial.printf("\nENDFRAME %u\n", static_cast<unsigned>(len));
    Serial.flush();
}

}  // namespace screenshot
