// Opening card — the MECC wordmark and the pioneer family by their wagon.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "art/Art.h"
#include "screens/MainMenuScreen.h"
#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

class TitleScreen : public Screen {
public:
    void tick(uint32_t dtMs) override { blinkMs_ += dtMs; }

    void render(LGFX_Sprite& g) override {
        g.fillScreen(0x0000);
        g.drawPng(art::banner_img, art::banner_img_len, 0, 24);
        g.drawPng(art::family_img, art::family_img_len, 0, 96);

        g.setFont(&fonts::Font2);
        g.setTextDatum(textdatum_t::top_center);
        if ((blinkMs_ / 600) % 2 == 0) {
            g.setTextColor(theme::ACCENT);
            g.drawString("tap to begin", 160, 214);
        }
        g.setTextColor(theme::INK_DIM);
        g.drawString("artwork from the 1990 MECC edition", 160, 230);
    }

    void onTap(int16_t, int16_t) override {
        app::screens.replace(new MainMenuScreen());
    }

private:
    uint32_t blinkMs_ = 0;
};
