// Presents a random trail event. Most just need acknowledging; a broken wagon
// part offers the choice of fitting a spare or making camp to repair.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class EventScreen : public Screen {
public:
    void onEnter() override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    bool     resolved_ = false;
    int      daysLost_ = 0;
    ui::Rect ok_{}, spare_{}, repair_{};
};
