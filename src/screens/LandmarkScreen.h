// "You have reached ___." Shown on arrival at a trail stop: a drawn landmark, a
// line of flavor, and — at forts — a way into the store.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class LandmarkScreen : public Screen {
public:
    void onEnter() override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    bool     hasStore_ = false;
    ui::Rect go_{}, store_{};
};
