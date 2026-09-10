// Opening card — the MECC wordmark and the pioneer family by their wagon.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "art/Art.h"
#include "art/gen/music.h"
#include "hw/Audio.h"
#include "screens/MainMenuScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class TitleScreen : public Screen {
public:
    // No dedicated title theme in the MECC set — loop one of the landmark tunes
    // under the opening card until the player taps through.
    static constexpr int kTitleSong = 14;   // 14 = Blue Mountains
    void onEnter() override {
        audio::playSong(music::landmark[kTitleSong].notes,
                        music::landmark[kTitleSong].len, true);
    }

    void tick(uint32_t dtMs) override { blinkMs_ += dtMs; }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(0x0000);
        g.drawPng(art::banner_img, art::banner_img_len, 0, 14);
        // family scene ~0.78x, centred
        g.drawPng(art::family_img, art::family_img_len, 30, 82, 0, 0, 0, 0,
                  0.78f, 0.78f);

        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::top_center);

        if ((blinkMs_ / 600) % 2 == 0) {
            g.setTextColor(theme::ACCENT);
            g.drawString("tap to begin", 160, 190);
        }
        g.setTextColor(theme::INK_DIM);
        g.drawString("artwork from the 1990 MECC edition", 160, 210);
        g.drawString("Vibecoded with Claude by BruteSource", 160, 224);
    }

    void onTap(int16_t, int16_t) override {
        app::screens.replace(new MainMenuScreen());
    }

private:
    uint32_t blinkMs_ = 0;
};
